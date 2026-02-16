/*
* GUI.cpp
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a windows to display information about the scene
* and allow the user to interact with it, e.g. change certain parameters or create new objects.
*/

#include <algorithm>
#include <array>
#include <cctype> // for std::tolower
#include <cstdint>
#include <limits> // for std::numeric_limits
#include <string>
#include <sstream>

#include "Gui.h"
#include "ImGuiFileDialog.h"
#include "../CUBE.h"
#include "../PLANE.h"

#include "../core/Core.h"
#include "../core/Node.h"
#include "../core/camera/Camera.h"
#include "../core/light/Light.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"
#include "../core/managers/NodeManager.h"
#include "../core/managers/SelectionManager.h"
#include "../core/managers/SceneManager.h"
#include "../core/managers/InputManager.h"
#include "../core/renderer/Renderer.h"
#include "../core/utils/random/Random.h"
#include "../core/utils/string/StringUtils.h"

// Namespaces
// ----------
// Namespace for cubemap face handling (unnamed to limit scope to this file)
namespace
{
	// the expected name tokens (case-insensitive) for each cubemap face are:
	// right, left, top, bottom, front, back; or posx, negx, posy, negy, posz, negz;
	// or short forms: px, nx, py, ny, pz, nz; up/down are also accepted for top/bottom
	enum class CubemapFaceIndex { RIGHT = 0, LEFT, TOP, BOTTOM, FRONT, BACK, COUNT };

	// assigns the given path to the specified cubemap face index if it has not been assigned yet
	// (returns true if the assignment was successful, false otherwise)
	bool AssignCubemapFace(
		std::array<std::string, static_cast<std::size_t>(CubemapFaceIndex::COUNT)>& orderedPaths,
		std::array<bool, static_cast<std::size_t>(CubemapFaceIndex::COUNT)>& pathAssigned,
		CubemapFaceIndex idx, const std::string& path)
	{
		const std::size_t i = static_cast<std::size_t>(idx);
		if (!pathAssigned[i])
		{
			orderedPaths[i] = path;
			pathAssigned[i] = true;
			return true;
		}
		return false;
	}
}

// Static Attributes
// -----------------
bool GUI::s_proportionalScaling{ true }; // propertional scaling flag is true by default

// Constructors
// ------------
GUI::GUI()
	: m_randomizer{ std::make_unique<Random>() }, // create a random number generator
	m_newAlbedo{ 0.8f, 0.8f, 0.8f, 1.0f }, // default albedo color for new objects is light gray
	m_newPosition{ 0.0f }, // default position for new objects is the origin
	m_newRotation{ 0.0f }, // default rotation for new objects is no rotation (identity quaternion)
	m_newDirection{ 0.0f, 0.0f, -1.0f }, // default direction for new objects is negative z-axis
	m_newScale{ 1.0f }, // default scale for new objects is 1.0
	m_newInnerCutOff{ 12.5f }, // default inner cutoff angle for new spotlights
	m_newOuterCutOff{ 32.5f }, // default outer cutoff angle for new spotlights
	m_mediumFont{ nullptr }, // medium font for the GUI (default font) is nullptr initially
	m_boldFont{ nullptr } // bold font for the GUI is nullptr initially
{
	initGUILayoutAttributes(); // initialize the display size-independent layout attributes
	// initialize the positions and sizes of the GUI windows to default values
	// (since they require the ImGui context to be created first to access the ImGui IO object)
	m_nodeInformationWindowPosition, m_sceneGraphWindowPosition, m_propertiesWindowPosition,
		m_creationWindowPosition, m_debugWindowPosition = ImVec2{ 0.0f, 0.0f };
	m_nodeInformationWindowSize, m_sceneGraphWindowSize, m_propertiesWindowSize, m_creationWindowSize,
		m_debugWindowSize = ImVec2{ 0.0f, 0.0f };
	// initialize the flags for the windows to prevent focus on the first frame
	m_nodeInformationWindowJustAppeared = true;
	m_sceneGraphWindowJustAppeared = true;
	m_propertiesWindowJustAppeared = true;
	m_creationWindowJustAppeared = true;
	m_debugWindowJustAppeared = true;
}

// Destructor
// ----------
GUI::~GUI()
{}

// Public Methods
// --------------
void GUI::InitGUI(GLFWwindow* window, const char* glslVersion)
{
	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // enable gamepad controls

	// button behavior configuration
	// faster button repeat when held down
	io.KeyRepeatDelay = 0.25f; // delay before repeating starts (in seconds)
	io.KeyRepeatRate = 0.05f; // rate at which the button is repeated (seconds between repeats)

	configureGUIStyle(); // configure the ImGui style (fonts, colors, etc.)

	// setup platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

void GUI::BuildGUI()
{
	beginGUIFrame(); // start a new ImGui frame
	configureGUILayout(); // configure the layout of the GUI windows based on the display size
	drawGUIWindows(); // draw the GUI windows (information, settings, and Create Windows)
	handleImGuiInput(); // handle ImGui input (mouse and keyboard)
}

void GUI::RenderGUI()
{
	ImGui::Render(); // render the ImGui draw data
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::ShutdownGUI() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(); // destroy the ImGui context
}

// Private methods
// ---------------
void GUI::initGUILayoutAttributes()
{
	// set the private variables for the GUI layout
	m_sceneGraphWindowRelativeWidth = 0.25f;
	m_sceneGraphWindowRelativeHeight = 0.6f;
	m_nodeInformationWindowRelativeWidth = 0.25f;
	m_nodeInformationWindowRelativeHeight = 0.4f;
	m_propertiesWindowRelativeWidth = 0.25f;
	m_propertiesWindowRelativeHeight = 0.4f;
	m_creationWindowRelativeWidth = 0.25f;
	m_creationWindowRelativeHeight = 0.2f;
	m_debugWindowRelativeWidth = 0.25f;
	m_debugWindowRelativeHeight = 0.4f;

	m_sceneGraphWindowXOffset = 0.0f;
	m_sceneGraphWindowYOffset = 0.0f;
	m_nodeInformationWindowXOffset = 0.0f;
	m_nodeInformationWindowYOffset = 1.0f - m_nodeInformationWindowRelativeHeight;
	m_propertiesWindowXOffset = 1.0f - m_propertiesWindowRelativeWidth;
	m_propertiesWindowYOffset = 0.0f;
	m_creationWindowXOffset = 1.0f - m_creationWindowRelativeWidth;
	m_creationWindowYOffset = m_propertiesWindowRelativeHeight;
	m_debugWindowXOffset = 1.0f - m_debugWindowRelativeWidth;
	m_debugWindowYOffset = 1.0f - m_debugWindowRelativeHeight;

	m_windowPositionPadding = ImVec2{ 10.f, 10.0f };
	m_windowSizePadding = ImVec2{ 20.f, 20.0f };
}

void GUI::configureGUIStyle()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	ImGuiStyle& style = ImGui::GetStyle(); // get the ImGui style object

	// add custom fonts to the ImGui context, setting one as the default font
	io.Fonts->AddFontDefault();
	ImFont* mediumFont = io.Fonts->AddFontFromFileTTF("resources/fonts/RobotoMono-Medium.ttf", 16.0f);
	ImFont* boldFont = io.Fonts->AddFontFromFileTTF("resources/fonts/RobotoMono-Bold.ttf", 16.0f);
	io.FontDefault = mediumFont; // set the medium font as the default font
	// set the member fonts for the GUI class
	m_mediumFont = mediumFont;
	m_boldFont = boldFont;

	// soften the edges of the ImGui windows and frames
	style.WindowRounding = 5.0f;
	style.FrameRounding = 5.0f;
	style.ScrollbarRounding = 5.0f;
	style.GrabRounding = 5.0f;

	// set the padding for the ImGui style
	style.WindowPadding = ImVec2(12.0f, 12.0f); // padding inside windows
	style.FramePadding = ImVec2(4.0f, 2.0f); // padding inside frames

	// set the ImGui color scheme to dark mode by default
	ImGui::StyleColorsDark();
	// change the color for all ImGui elements with an accent color to a purple hue,
	// leaving the rest of the colors unchanged
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.08f, 0.15f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.33f, 0.28f, 0.40f, 0.60f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg] = ImVec4(0.26f, 0.19f, 0.38f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.36f, 0.29f, 0.58f, 0.67f);

	colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.04f, 0.18f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.19f, 0.38f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.00f, 0.14f, 0.51f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.10f, 0.22f, 1.00f);

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.10f, 0.22f, 0.60f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.21f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.31f, 0.51f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.41f, 0.61f, 1.00f);

	colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.29f, 0.68f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.32f, 0.78f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.29f, 0.68f, 1.00f);

	colors[ImGuiCol_Button] = ImVec4(0.56f, 0.39f, 0.78f, 0.60f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.66f, 0.49f, 0.88f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.39f, 0.78f, 1.00f);

	colors[ImGuiCol_Header] = ImVec4(0.36f, 0.29f, 0.58f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.29f, 0.58f, 1.00f);

	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.20f, 0.55f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.20f, 0.55f, 1.00f);

	colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.29f, 0.58f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.36f, 0.29f, 0.58f, 0.95f);

	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
	colors[ImGuiCol_TabSelected] = ImLerp(
		colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TabDimmed] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabDimmedSelected] = ImLerp(
		colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);

	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.13f, 0.22f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.21f, 0.21f, 0.25f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

	colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.36f, 0.29f, 0.58f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.36f, 0.29f, 0.58f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}


void GUI::beginGUIFrame() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUI::configureGUILayout()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings

	// position of the Scene Graph Window - top left corner with padding
	m_sceneGraphWindowPosition = ImVec2{
		io.DisplaySize.x * m_sceneGraphWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_sceneGraphWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Scene Graph Window
	m_sceneGraphWindowSize = ImVec2{
		io.DisplaySize.x * m_sceneGraphWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_sceneGraphWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Node Information Window - bottom left corner with padding
	m_nodeInformationWindowPosition = ImVec2{
		io.DisplaySize.x * m_nodeInformationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_nodeInformationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Node Information Window
	m_nodeInformationWindowSize = ImVec2{
		io.DisplaySize.x * m_nodeInformationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_nodeInformationWindowRelativeHeight - m_windowSizePadding.y // height
	};
	// position of the Properties Window - top right corner with padding
	m_propertiesWindowPosition = ImVec2{
		io.DisplaySize.x * m_propertiesWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_propertiesWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Properties Window
	m_propertiesWindowSize = ImVec2{
		io.DisplaySize.x * m_propertiesWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_propertiesWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Creation Window - right side, below Properties with padding
	m_creationWindowPosition = ImVec2{
		io.DisplaySize.x * m_creationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_creationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Creation Window
	// (account for the padding between Creation and Debug Windows)
	m_creationWindowSize = ImVec2{
		io.DisplaySize.x * m_creationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_creationWindowRelativeHeight - m_windowPositionPadding.y // height
	};
	// position of the Debug Window - bottom right corner with padding
	// (no vertical padding at top since Creation window handles the gap)
	m_debugWindowPosition = ImVec2{
		io.DisplaySize.x * m_debugWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_debugWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Debug Window
	m_debugWindowSize = ImVec2{
		io.DisplaySize.x * m_debugWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_debugWindowRelativeHeight - m_windowSizePadding.y // height
	};
}

void GUI::drawGUIWindows()
{
	drawSceneGraphWindow();
	drawNodeInformationWindow();
	drawPropertiesWindow();
	drawCreationWindow();
	drawDebugWindow();
}

void GUI::handleImGuiInput() const
{
	// get the input manager from the Core instance
	auto& inputManager = Core::GetInstance()->GetInputManager();

	// check if ImGui wants to capture the mouse (when interacting with the GUI)
	if (ImGui::GetIO().WantCaptureMouse) // prevent camera manipulation
		inputManager->SetCameraControlEnabled(false);
	else // re-enable camera manipulation
		inputManager->SetCameraControlEnabled(true);
}

void GUI::resetGUILayout()
{
	// recalculate layout for current window size
	configureGUILayout();

	// reset all window states (positions, sizes, collapsed states)
	ImGui::SetWindowPos("NODE INFORMATION", m_nodeInformationWindowPosition);
	ImGui::SetWindowSize("NODE INFORMATION", m_nodeInformationWindowSize);
	ImGui::SetWindowCollapsed("NODE INFORMATION", false);

	ImGui::SetWindowPos("SCENE GRAPH", m_sceneGraphWindowPosition);
	ImGui::SetWindowSize("SCENE GRAPH", m_sceneGraphWindowSize);
	ImGui::SetWindowCollapsed("SCENE GRAPH", false);

	ImGui::SetWindowPos("DEBUG", m_debugWindowPosition);
	ImGui::SetWindowSize("DEBUG", m_debugWindowSize);
	ImGui::SetWindowCollapsed("DEBUG", false);

	ImGui::SetWindowPos("PROPERTIES", m_propertiesWindowPosition);
	ImGui::SetWindowSize("PROPERTIES", m_propertiesWindowSize);
	ImGui::SetWindowCollapsed("PROPERTIES", false);

	ImGui::SetWindowPos("CREATION", m_creationWindowPosition);
	ImGui::SetWindowSize("CREATION", m_creationWindowSize);
	ImGui::SetWindowCollapsed("CREATION", false);

	// log info message
	std::cout << "[INFO::GUI::resetGUILayout] GUI layout reset to default" << std::endl;
}


void GUI::drawSceneGraphWindow()
{
	// set initial size and position for the Scene Graph Window
	ImGui::SetNextWindowSize(m_sceneGraphWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_sceneGraphWindowPosition, ImGuiCond_Appearing);
	// set the Scene Graph Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that displays the scene graph as a hierarchical tree
		// begin the Scene Graph window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("SCENE GRAPH", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// display instruction text
		ImGui::TextWrapped("Left click: Select node\n"
						   "Left click empty space: Clear selection\n"
						   "Drag models to group/ungroup\n"
						   "Drop on empty space to make a root node\n"
						   "Del/Supr: Delete selected node");
		ImGui::Separator();

		// create a child window to hold the tree. this provides a consistent background
		// for the drop target and allows for independent scrolling.
		ImGui::BeginChild("SceneGraphTree", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove);

		// get the node manager from the Core instance
		auto& nodeManager = Core::GetInstance()->GetNodeManager();

		// get all root nodes (nodes without parents)
		std::vector<std::shared_ptr<Node>> rootNodes;
		// collect cameras
		for (const auto& node : nodeManager->GetNodes("CAMERA"))
			if (!node->GetParent())
				rootNodes.push_back(node);
		// collect lights (excluding gizmos which have parents)
		for (const auto& node : nodeManager->GetNodes("LIGHT"))
			if (!node->GetParent())
				rootNodes.push_back(node);
		// collect models (excluding children which have parents)
		for (const auto& node : nodeManager->GetNodes("MODEL"))
			if (!node->GetParent())
				rootNodes.push_back(node);

		// update the open-set every frame so viewport selection drives expansion
		updateSceneGraphAutoOpenSet();

		// draw the tree recursively starting from root nodes
		for (const auto& rootNode : rootNodes)
			if (rootNode) // ensure the node is valid
				drawNodeTreeRecursive(rootNode);

		// create an invisible "drop zone" that fills the remaining space in the child window
		// (this acts as a visual drop target - with a border when hovered - for making nodes root nodes)
		ImVec2 availableSpace = ImGui::GetContentRegionAvail();
		// ensure minimum height so there's always a droppable area
		float dropZoneHeight = std::max(availableSpace.y, 40.0f);
		// use an InvisibleButton to create an interactable area that fills the remaining space
		ImGui::InvisibleButton("##RootDropZone", ImVec2(availableSpace.x, dropZoneHeight));

		// make this invisible button a drop target for unparenting nodes
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_NODE"))
			{
				std::uint32_t draggedNodeId = *(const std::uint32_t*)payload->Data;
				auto draggedNode = nodeManager->GetNodeById(draggedNodeId);

				// only process if the dragged node exists, is draggable, and has a parent
				if (draggedNode && draggedNode->IsDraggable() && draggedNode->GetParent())
				{
					// preserve the node's world transform before un-parenting
					const glm::mat4 worldTransform = draggedNode->GetWorldModelMatrix();

					// remove from old parent. this sets the node's parent to nullptr
					draggedNode->GetParent()->RemoveChild(draggedNode);

					// decompose the world matrix and set it as the new local transform
					glm::vec3 scale, translation, skew;
					glm::quat rotation;
					glm::vec4 perspective;
					glm::decompose(worldTransform, scale, rotation, translation, skew, perspective);

					draggedNode->SetPosition(translation);
					draggedNode->SetRotation(rotation);
					draggedNode->SetScale(scale);

					std::cout << "[INFO::GUI] Node '" << draggedNode->GetName()
						<< "' (ID: " << draggedNodeId << ") is now a root node" << std::endl;
				}
			}

			ImGui::EndDragDropTarget(); // end the drop target for unparenting nodes
		}

		// clear selection when clicking on empty space in the child window
		// (not on items, not on scrollbars/title bar)
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
		{
			// if the window is hovered and left mouse button is clicked on empty space,
			// clear the current selection and print a message to the console
			auto& selectionManager = Core::GetInstance()->GetSelectionManager();

			// get the selected node id to check if a node was selected
			std::uint32_t previousSelectedId = selectionManager->GetSelectedNodeId();
			if (previousSelectedId != 0)
			{ // only get the name of the node, clear selection, and log if a node was actually selected
				// get the name of the node that is about to be deselected (for logging purposes)
				std::string previousSelectedName;
				auto previousSelectedNode = selectionManager->GetSelectedNode(nodeManager.get());
				if (previousSelectedNode)
					previousSelectedName = previousSelectedNode->GetName();

				// clear the selection and print info message with the deselected node's name and id
				selectionManager->ClearSelection();

				if (!previousSelectedName.empty())
				{
					std::cout << "[INFO::GUI::drawSceneGraphWindow] Deselected node '"
						<< previousSelectedName << "' (ID " << previousSelectedId << ")" << std::endl;
				}
			}
		}

		ImGui::EndChild(); // end the child window for the tree

		ImGui::End(); // end the Scene Graph window

		if (m_sceneGraphWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_sceneGraphWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawNodeInformationWindow()
{
	// set initial size and position for the Node Information Window
	ImGui::SetNextWindowSize(m_nodeInformationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_nodeInformationWindowPosition, ImGuiCond_Appearing);
	// set the Node Information Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that displays information about the selected node
		// begin the Node Information window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("NODE INFORMATION", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// get the selection manager from the Core instance
		auto& nodeManager = Core::GetInstance()->GetNodeManager();
		auto& selectionManager = Core::GetInstance()->GetSelectionManager();

		auto selected = selectionManager->GetSelectedNode(nodeManager.get());
		if (!selected)
		{ // if no node is selected, display a message prompting the user to select one
			ImGui::TextWrapped("No node selected."
							   "\nClick a node in the Scene Graph or viewport to inspect it.");
		}
		else
		{ // if a node is selected, display its detailed information
			// display the name and id of the selected node in bold font
			ImGui::PushFont(m_boldFont);
			ImGui::TextWrapped("%s", selected->GetName().c_str());
			ImGui::Text("ID: %u", selected->GetId());
			ImGui::PopFont();

			ImGui::Separator(); // to separate the header from the details

			// display type-specific information
			if (selected->GetNodeType() == NodeType::CAMERA)
			{ // if the selected node is a camera, display camera-specific information
				auto camera = dynamic_cast<Camera*>(selected.get());
				ImGui::Text("Type: Camera");
				ImGui::Text("Position: (%.3f, %.3f, %.3f)",
							camera->GetPosition().x,
							camera->GetPosition().y,
							camera->GetPosition().z);
				ImGui::Text("Front: (%.3f, %.3f, %.3f)",
							camera->GetFront().x,
							camera->GetFront().y,
							camera->GetFront().z);
				ImGui::Text("Yaw: %.3f", camera->GetYaw());
				ImGui::Text("Pitch: %.3f", camera->GetPitch());
				ImGui::Text("Zoom: %.3f", camera->GetZoom());
			}
			else if (selected->GetNodeType() == NodeType::LIGHT)
			{ // if the selected node is a light, display light-specific information
				auto light = dynamic_cast<Light*>(selected.get());
				ImGui::Text("Type: Light");

				switch (light->GetLightType()) // display information based on the light type
				{
					case LightType::DIRECTIONAL_LIGHT:
					{ // if the light is a directional light, display directional light-specific properties
						auto dirLight = dynamic_cast<DirectionalLight*>(light);
						ImGui::Text("Light Type: Directional");
						ImGui::Text("Color: (%.3f, %.3f, %.3f)",
									dirLight->GetDiffuse().x,
									dirLight->GetDiffuse().y,
									dirLight->GetDiffuse().z);
						ImGui::Text("Position: (%.3f, %.3f, %.3f)",
									dirLight->GetPosition().x,
									dirLight->GetPosition().y,
									dirLight->GetPosition().z);
						ImGui::Text("Direction: (%.3f, %.3f, %.3f)",
									dirLight->GetDirection().x,
									dirLight->GetDirection().y,
									dirLight->GetDirection().z);
						break;
					}
					case LightType::POINT_LIGHT:
					{ // if the light is a point light, display point light-specific properties
						auto pointLight = dynamic_cast<PointLight*>(light);
						ImGui::Text("Light Type: Point");
						ImGui::Text("Color: (%.3f, %.3f, %.3f)",
									pointLight->GetDiffuse().x,
									pointLight->GetDiffuse().y,
									pointLight->GetDiffuse().z);
						ImGui::Text("Position: (%.3f, %.3f, %.3f)",
									pointLight->GetPosition().x,
									pointLight->GetPosition().y,
									pointLight->GetPosition().z);
						break;
					}
					case LightType::SPOTLIGHT:
					{ // if the light is a spotlight, display spotlight-specific properties
						auto spotlight = dynamic_cast<Spotlight*>(light);
						ImGui::Text("Light Type: Spotlight");
						ImGui::Text("Color: (%.3f, %.3f, %.3f)",
									spotlight->GetDiffuse().x,
									spotlight->GetDiffuse().y,
									spotlight->GetDiffuse().z);
						ImGui::Text("Position: (%.3f, %.3f, %.3f)",
									spotlight->GetPosition().x,
									spotlight->GetPosition().y,
									spotlight->GetPosition().z);
						ImGui::Text("Direction: (%.3f, %.3f, %.3f)",
									spotlight->GetDirection().x,
									spotlight->GetDirection().y,
									spotlight->GetDirection().z);
						ImGui::Text("Inner Cut-off: %.3f°",
									glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
						ImGui::Text("Outer Cut-off: %.3f°",
									glm::degrees(glm::acos(spotlight->GetOuterCutOff())));
						break;
					}
					default: // unknown light type, display a generic message
						ImGui::Text("Light Type: Unknown");
						break;
				}
			}
			else if (selected->GetNodeType() == NodeType::COMPOSITE_MODEL ||
					 selected->GetNodeType() == NodeType::COMPOSITE_ASSIMP_MODEL ||
					 selected->GetNodeType() == NodeType::COMPOSITE_SHAPE_MODEL)
			{ // if the selected node is a composite model, display composite-specific information
				ImGui::Text("Type: Composite Model (Group)");
				ImGui::Text("Children: %zu", selected->GetChildren().size());
				ImGui::Text("Position: (%.3f, %.3f, %.3f)",
							selected->GetPosition().x,
							selected->GetPosition().y,
							selected->GetPosition().z);
				ImGui::Text("Rotation: (%.3f, %.3f, %.3f)",
							selected->GetRotationInEulerAngles().x,
							selected->GetRotationInEulerAngles().y,
							selected->GetRotationInEulerAngles().z);
				ImGui::Text("Scale: (%.3f, %.3f, %.3f)",
							selected->GetScale().x,
							selected->GetScale().y,
							selected->GetScale().z);
			}
			else if (selected->GetNodeType() == NodeType::ASSIMP_MODEL ||
					 selected->GetNodeType() == NodeType::SHAPE_MODEL)
			{ // if the selected node is a model, display model-specific information
				if (selected->GetGizmoType() != GizmoType::NONE)
				{ // if the model is a gizmo, display its gizmo type
					ImGui::Text("Type: Gizmo");
				}
				else
				{ // if the model is not a gizmo, display its type
					if (selected->GetNodeType() == NodeType::ASSIMP_MODEL)
						ImGui::Text("Type: Assimp Model");
					else if (selected->GetNodeType() == NodeType::SHAPE_MODEL)
						ImGui::Text("Type: Shape");
					else
						ImGui::Text("Type: Unknown");
				}

				if (selected->GetNodeType() == NodeType::SHAPE_MODEL)
				{ // if the model is a shape, display its color
					ImGui::Text("Color: (%.3f, %.3f, %.3f, %.3f)",
								selected->GetAlbedo().x,
								selected->GetAlbedo().y,
								selected->GetAlbedo().z,
								selected->GetAlbedo().w);
				}
				ImGui::Text("Position: (%.3f, %.3f, %.3f)",
							selected->GetPosition().x,
							selected->GetPosition().y,
							selected->GetPosition().z);
				ImGui::Text("Rotation: (%.3f, %.3f, %.3f)",
							selected->GetRotationInEulerAngles().x,
							selected->GetRotationInEulerAngles().y,
							selected->GetRotationInEulerAngles().z);
				ImGui::Text("Scale: (%.3f, %.3f, %.3f)",
							selected->GetScale().x,
							selected->GetScale().y,
							selected->GetScale().z);
			}
		}

		ImGui::End(); // end the Node Information window

		if (m_nodeInformationWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_nodeInformationWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawPropertiesWindow()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();
	auto& selectionManager = Core::GetInstance()->GetSelectionManager();

	// set initial size and position for the Properties Window
	ImGui::SetNextWindowSize(m_propertiesWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_propertiesWindowPosition, ImGuiCond_Appearing);
	// set the Properties Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that allows the user to change the properties of the nodes in the scene
		// begin the Properties window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("PROPERTIES", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		auto selected = selectionManager->GetSelectedNode(nodeManager.get());
		if (!selected)
		{ // if no node is selected, display a message
			ImGui::Text("No node selected.\nClick an object to inspect it.");
		}
		else
		{ // if an node is selected, display its name and ID, and draw its controls
			// display the name of the selected node in bold font
			ImGui::PushFont(m_boldFont);
			ImGui::Text(selected->GetName().c_str());
			ImGui::PopFont();

			ImGui::Separator();

			// depending on the type of the selected node, draw the corresponding controls
			if (selected->GetNodeType() == NodeType::LIGHT)
			{ // if the selected node is a light, draw the light controls
				auto light = dynamic_cast<Light*>(selected.get()); // dynamic cast to Light object
				drawLightControls(light); // draw the light controls
			}
			else if (selected->GetNodeType() == NodeType::COMPOSITE_MODEL ||
					 selected->GetNodeType() == NodeType::COMPOSITE_ASSIMP_MODEL ||
					 selected->GetNodeType() == NodeType::ASSIMP_MODEL ||
					 selected->GetNodeType() == NodeType::COMPOSITE_SHAPE_MODEL ||
					 selected->GetNodeType() == NodeType::SHAPE_MODEL) // any model type
			{ // if the selected node is a model, draw the model controls
				auto model = dynamic_cast<Node*>(selected.get()); // dynamic cast to Model object
				// only draw the model controls if the model is not a gizmo, since that is handled
				// with its corresponding light controls above
				if (model->GetGizmoType() == GizmoType::NONE)
					drawModelControls(model); // draw the model controls
			}

			ImGui::Separator();

			if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
			{ // if the Delete button is clicked, delete the selected node via the selection manager
				selectionManager->DeleteSelected(nodeManager.get());
			}
		}

		ImGui::End(); // end the Properties window

		if (m_propertiesWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_propertiesWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawCreationWindow()
{
	// set initial size and position for the Create Window
	ImGui::SetNextWindowSize(m_creationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_creationWindowPosition, ImGuiCond_Appearing);
	// set the Create Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains buttons to add new objects to the scene
		// begin the Creation window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("CREATION", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// button to add a new directional light to the scene
		if (ImGui::Button("Create Directional Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Directional Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawCreateDirectionalLightPopup(); // draw the popup to create a new directional light

		// button to add a new point light to the scene
		if (ImGui::Button("Create Point Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Point Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}
		drawCreatePointLightPopup(); // draw the popup to create a new point light

		// button to add a new spotlight to the scene
		if (ImGui::Button("Create Spotlight", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Spotlight");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawCreateSpotlightPopup(); // draw the popup to create a new spotlight

		// button to add a new plane shape to the scene
		if (ImGui::Button("Create Plane Shape", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Plane Shape");
			// randomize the albedo and position of the new plane shape
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			// initialize the rotation and scale of the new plane shape with default values
			m_newRotation = glm::vec3{ 0.0f };
			m_newScale = glm::vec3{ 1.0f };
		}
		drawCreatePlaneShapePopup(); // draw the popup to create a new plane shape

		// buttom to add a new cube shape to the scene
		if (ImGui::Button("Create Cube Shape", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Cube Shape");
			// randomize the albedo and position of the new cube shape
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			// initialize the rotation and scale of the new cube shape with default values
			m_newRotation = glm::vec3{ 0.0f };
			m_newScale = glm::vec3{ 1.0f };
		}
		drawCreateCubeShapePopup(); // draw the popup to create a new cube shape

		// button to import a new model from a file
		if (ImGui::Button("Import 3D Model", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = "./resources/models"; // initial directory to open the file dialog
			fileDialogConfig.countSelectionMax = 1; // for now, allow only one file to be selected
			fileDialogConfig.flags = ImGuiFileDialogFlags_Modal; // no special flags for the file dialog

			// open a file dialog to select a model file
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseFileDlgKey", // unique key for the file dialog
				"Choose 3D Model File", // title of the file dialog
				".obj, .fbx, .dae, .gltf, .glb, .stl, .ply, .3ds, .max", // supported file extensions
				fileDialogConfig // file dialog configuration
			);
		}
		drawImportModelPopup(); // draw the popup for importing a new model

		// button to import a new skybox from 6 image files
		if (ImGui::Button("Import Skybox", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = "./resources/textures/skyboxes"; // initial directory for the file dialog
			fileDialogConfig.countSelectionMax = 6; // allow up to 6 files to be selected
			fileDialogConfig.flags = ImGuiFileDialogFlags_Modal; // no special flags for the file dialog

			// open a file dialog to select skybox image files
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseSkyboxDlgKey", // unique key for the file dialog
				"Choose Skybox Image Files", // title of the file dialog
				".jpg,.jpeg,.png,.hdr", // supported file extensions
				fileDialogConfig // file dialog configuration
			);
		}
		drawImportSkyboxPopup(); // draw the popup for importing a new skybox textures

		ImGui::End(); // end the Create Objects window

		if (m_creationWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_creationWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawDebugWindow()
{
	// set initial size and position for the Debug Window
	ImGui::SetNextWindowSize(m_debugWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_debugWindowPosition, ImGuiCond_Appearing);
	// set the Debug Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains debug information and controls
		// begin the Debug window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("DEBUG", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// get the renderer screen debug params from the Core instance
		auto renderer = Core::GetInstance()->GetRenderer();
		auto& params = renderer->GetScreenDebugParams();
		// vector of strings that represent the different debug modes
		std::vector<std::string> debugModes = {
			"Normal",
			"Inverted Colors",
			"Picking Colors",
			"Solid Color",
			"Grid Overlay"
		};
		int debugModeIndex = static_cast<int>(params.debugMode); // current debug mode index
		bool paramsChanged{ false }; // dirty flag to check if any of the params were changed

		// local helper to draw the outline color palette (used for outline-capable modes: 0 and 1)
		auto drawOutlineColorPalette = []()
		{
			ImGui::Text("\tConfiguration");
			// access SelectionManager outline params
			auto& selectionManager = Core::GetInstance()->GetSelectionManager();
			auto& outlineParams = selectionManager->GetOutlineParams(); // copy current outline params

			// palette of vibrant outline colors (first item is the default color)
			static const std::vector<std::pair<const char*, glm::vec3>> outlineColorPalette = {
			{ "Cyan",     glm::vec3{ 0.00f, 0.95f, 1.00f } },
			{ "Lime",     glm::vec3{ 0.30f, 1.00f, 0.30f } },
			{ "Magenta",  glm::vec3{ 1.00f, 0.20f, 0.90f } },
			{ "Yellow",   glm::vec3{ 1.00f, 0.95f, 0.20f } },
			{ "Orange",   glm::vec3{ 1.00f, 0.60f, 0.20f } },
			{ "Red",      glm::vec3{ 1.00f, 0.20f, 0.20f } },
			{ "Blue",     glm::vec3{ 0.20f, 0.50f, 1.00f } },
			{ "Purple",   glm::vec3{ 0.75f, 0.40f, 1.00f } },
			{ "White",    glm::vec3{ 1.00f, 1.00f, 1.00f } }
			};

			// find nearest palette entry to current color (keeps UI in sync if color changed elsewhere)
			auto currentOutlineColor = outlineParams.color;
			int currentIdx{}; // index of the currently selected color in the palette
			float best = std::numeric_limits<float>::max();
			for (int i{}; i < static_cast<int>(outlineColorPalette.size()); ++i)
			{
				glm::vec3 d = currentOutlineColor - outlineColorPalette[i].second;
				float dist2 = glm::dot(d, d);
				if (dist2 < best) { best = dist2; currentIdx = i; }
			}

			ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
			ImGui::SameLine();
			ImGui::Text("Outline Color");
			ImGui::SameLine(); // keep the combo box on the same line as the label
			// make the combo box take the full width of the window
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::BeginCombo("##OutlineColorComboBox", outlineColorPalette[currentIdx].first,
								  ImGuiComboFlags_HeightSmall))
			{ // if the combo box is opened, iterate through all colors in the palette and display them
				for (int n{}; n < static_cast<int>(outlineColorPalette.size()); ++n)
				{
					bool isSelected = (currentIdx == n); // check if the current color is selected
					if (ImGui::Selectable(outlineColorPalette[n].first, isSelected))
					{ // if a color is selected, update the outline params in the SelectionManager
						OutlineParams updated = outlineParams;
						updated.color = outlineColorPalette[n].second;
						selectionManager->SetOutlineParams(updated);
					}
					// set the selected color as the default focus, i.e. highlight it
					if (isSelected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo(); // end the combo box
			}
		};

		// scene settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("SCENE SETTINGS");
		ImGui::PopFont();

		auto& sceneManager = Core::GetInstance()->GetSceneManager();

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing before button section

		// calculate button width based on available space and spacing between buttons (3 buttons total)
		float buttonWidth = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0f;

		// button to reset the camera position and orientation
		if (ImGui::Button("Reset\nCamera", ImVec2{ buttonWidth, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, set the camera to its default position and orientation
			sceneManager->ResetCamera();
		}
		ImGui::SameLine(); // keep the buttons on the same line
		// button to clear the skybox
		if (ImGui::Button("Clear\nSkybox", ImVec2{ buttonWidth, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, clear the skybox, i.e. remove the current skybox texture
			sceneManager->ClearSkybox();
		}
		ImGui::SameLine(); // keep the buttons on the same line
		// button to reset GUI layout
		if (ImGui::Button("Reset GUI\nLayout", ImVec2{ buttonWidth, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, reset all GUI windows to default layout
			resetGUILayout();
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing after button section

		// rendering settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("RENDERING INFORMATION");
		ImGui::PopFont();

		// display the current FPS and frame time
		ImGui::TextWrapped("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::TextWrapped("Frame Time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);

		// display the screen texture debug mode currently in use and the values of its parameters
		ImGui::TextWrapped("Screen Texture Debug Mode: %s", debugModes[debugModeIndex].c_str());
		switch (debugModeIndex)
		{
			case 0: // Normal mode
				break; // no parameters to display
			case 1: // Inverted Colors mode
				break; // no parameters to display
			case 2: // Picking Colors (raw picking buffer visualization)
				// no parameters to display, draw a text to explain what is shown
				ImGui::TextWrapped(" Showing per-object encoded IDs as colors");
				break;
			case 3: // Solid Color mode
				ImGui::Text(" Solid Color: (%.3f, %.3f, %.3f)",
							params.solidColor.r, params.solidColor.g, params.solidColor.b);
				break;
			case 4: // Grid Overlay mode
				ImGui::Text(" Grid Line Count: %d", params.gridLineCount);
				ImGui::Text(" Grid Line Thickness: %.2f", params.gridLineThickness);
				ImGui::Text(" Grid Background Color: (%.3f, %.3f, %.3f)",
							params.gridBgColor.r, params.gridBgColor.g, params.gridBgColor.b);
				ImGui::Text(" Grid Line Color: (%.3f, %.3f, %.3f)",
							params.gridLineColor.r, params.gridLineColor.g, params.gridLineColor.b);
				break;
			default:
				std::cerr << "[ERROR::GUI::drawDebugWindow] Unknown screen texture debug mode index: "
					<< debugModeIndex << std::endl;
				break;
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing before next section

		// rendering settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("RENDERING SETTINGS");
		ImGui::PopFont();

		ImGui::Text("Screen Texture Debug Mode");
		ImGui::SameLine(); // keep the combo box on the same line as the label
		// make the combo box take the full width of the window
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		// draw a combo box to select the screen texture debug mode (with the current mode as preview)
		if (ImGui::BeginCombo("##ScreenTextureDebugMode", debugModes[debugModeIndex].c_str(),
							  ImGuiComboFlags_HeightSmall))
		{ // if the combo box is opened
			for (int n{}; n < debugModes.size(); n++)
			{ // iterate through all debug modes
				bool isSelected = (debugModeIndex == n); // check if the current mode is selected
				if (ImGui::Selectable(debugModes[n].c_str(), isSelected))
				{ // if a mode is selected, update the current debug mode locally and in the renderer
					debugModeIndex = n;
					params.debugMode = debugModeIndex;
					paramsChanged = true; // mark the params as changed
				}
				if (isSelected) // whatever mode is selected, set it as the default focus
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo(); // end the combo box
		}

		// depending on the selected debug mode, draw additional controls
		switch (debugModeIndex)
		{
			case 0: // Normal mode
			{ // draw the outline color palette in a combo box to allow for outline color selection
				drawOutlineColorPalette();
			}
			break;
			case 1: // Inverted Colors mode
			{ // draw the outline color palette in a combo box to allow for outline color selection
				drawOutlineColorPalette();
			}
			break;
			case 2: // Picking Colors mode (raw picking buffer visualization)
				break; // no additional controls needed
			case 3: // Solid Color mode
			{ // draw a color picker to select the solid color
				ImGui::Text(" Configuration");
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Solid Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// use the custom drawColorControl helper to draw the color picker
				if (drawColorControl("##SolidColor", params.solidColor, false))
					paramsChanged = true; // mark the params as changed
			}
			break;
			case 4: // Grid Overlay mode
			{ // draw controls for each parameter
				ImGui::Text(" Configuration");
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Count      ");
				ImGui::SameLine(); // keep the input field on the same line as the label
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputInt("##GridLineCount", (int*)&params.gridLineCount,
									1, 10)) // set step values for the input field (normal and fast)
				{ // if the input field is changed, update the number of grid lines
					// clamp the value to a reasonable range [2, 1000]
					if (params.gridLineCount < 2)
						params.gridLineCount = 2;
					else if (params.gridLineCount > 1000)
						params.gridLineCount = 1000;

					// update the number of grid lines in the params
					params.gridLineCount = params.gridLineCount;
					paramsChanged = true; // mark the params as changed
				}
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Thickness  ");
				ImGui::SameLine(); // keep the input field on the same line as the label
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputFloat("##GridLineThickness", &params.gridLineThickness,
									  0.05f, 0.5f, // set step values for the input field (normal and fast)
									  "%.2f"))
				{ // if the input field is changed, update the grid line thickness
					// clamp the value to a reasonable range [1.0, 10.0]
					params.gridLineThickness = std::clamp(params.gridLineThickness, 1.0f, 10.0f);
					paramsChanged = true; // mark the params as changed
				}

				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Background Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// for the background color and the line color,
				// disable the alpha channel and the inputs (only show an RGB color picker)
				if (ImGui::ColorEdit3("##GridBackgroundColor", (float*)&params.gridBgColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					paramsChanged = true; // mark the params as changed
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Color      ");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				if (ImGui::ColorEdit3("##GridLineColor", (float*)&params.gridLineColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					paramsChanged = true; // mark the params as changed
			}
			break;
			default:
				std::cerr << "[ERROR::GUI::drawDebugWindow] Unknown screen texture debug mode: "
					<< debugModeIndex << std::endl;
				break;
		}

		if (paramsChanged)
		{ // if any of the renderer debug params were changed, apply the changes to the renderer
			renderer->SetScreenDebugParams(params);
		}

		ImGui::End(); // end the Debug window

		if (m_debugWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_debugWindowJustAppeared = false; // no longer the first frame
		}
	}
}


void GUI::drawNodeTreeRecursive(const std::shared_ptr<Node>& node)
{
	if (!node) return; // safety check

	// hide gizmo nodes from the tree view to prevent clutter and confusion
	if (node->GetGizmoType() != GizmoType::NONE) return;

	// get selection manager to check if this node is selected
	auto& selectionManager = Core::GetInstance()->GetSelectionManager();
	bool isSelected = (selectionManager->GetSelectedNodeId() == node->GetId());

	// create flags for the tree node based on its state and type
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	// if the node is selected, add the Selected flag
	if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

	// if the node has no children, make it a leaf node
	if (!node->IsComposite()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	// if the node is a light, do no show an arrow as if it was a leaf node (gizmos are hidden)
	if (node->GetNodeType() == NodeType::LIGHT)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	// determine icon/prefix based on node type
	std::string nodeLabel;
	switch (node->GetNodeType())
	{
		case NodeType::CAMERA:
			nodeLabel = "[CAMERA] " + node->GetName();
			break;
		case NodeType::LIGHT:
			nodeLabel = "[LIGHT] " + node->GetName();
			break;
		case NodeType::COMPOSITE_MODEL:
		case NodeType::COMPOSITE_ASSIMP_MODEL:
		case NodeType::COMPOSITE_SHAPE_MODEL:
			nodeLabel = "[GROUP] " + node->GetName();
			break;
		case NodeType::MODEL:
		case NodeType::ASSIMP_MODEL:
		case NodeType::SHAPE_MODEL:
			nodeLabel = "[MODEL] " + node->GetName();
			break;
		default:
			nodeLabel = "[UNKNOWN] " + node->GetName();
			break;
	}

	// push a unique ID for this tree node
	ImGui::PushID(static_cast<int>(node->GetId()));

	// auto-expand only nodes on the selection path (root -> ... -> selected),
	// but not the selected node itself, and do so only once per selection change
	if (m_sceneGraphAutoOpenIds.contains(node->GetId())) ImGui::SetNextItemOpen(true, ImGuiCond_Always);

	// draw the tree node and get whether it is open
	bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), flags);

	// DRAG SOURCE: only make draggable nodes a drag source
	if (node->IsDraggable() && ImGui::BeginDragDropSource())
	{
		// set payload to carry the node id
		std::uint32_t nodeId = node->GetId();
		ImGui::SetDragDropPayload("SCENE_GRAPH_NODE", &nodeId, sizeof(nodeId));

		// display a preview of the node being dragged
		ImGui::Text("Moving %s", node->GetName().c_str());

		ImGui::EndDragDropSource(); // end the drag source
	}

	// DROP TARGET: only make nodes that can be parents a drop target
	if (node->CanBeParent() && ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_NODE"))
		{
			// get the node manager from the Core instance to resolve node ids to node pointers
			auto& nodeManager = Core::GetInstance()->GetNodeManager();
			std::uint32_t draggedNodeId = *(const std::uint32_t*)payload->Data;
			auto draggedNode = nodeManager->GetNodeById(draggedNodeId);
			auto targetNode = node; // the current node in the recursion is the target

			// perform validation before re-parenting
			bool isValidOperation = true;
			if (!draggedNode || !targetNode || draggedNode == targetNode)
			{
				isValidOperation = false; // cannot drop on self or if nodes are invalid
			}
			else if (!targetNode->CanBeParent())
			{
				std::cerr << "[WARNING::GUI] Invalid drop: Target node '" << targetNode->GetName()
					<< "' cannot be a parent" << std::endl;
				isValidOperation = false;
			}
			else
			{
				// cycle check: a node cannot be parented to its own descendant
				for (auto p = targetNode->GetParent(); p; p = p->GetParent())
				{
					if (p == draggedNode)
					{ // if any ancestor of the target node is the dragged node, it's an invalid operation
						std::cerr << "[WARNING::GUI] Invalid drop: "
							" Cannot parent a node to its own descendant" << std::endl;
						isValidOperation = false;
						break;
					}
				}
			}

			// if the operation is valid, proceed with re-parenting and transform preservation
			if (isValidOperation)
			{
				// preserve the world transform before re-parenting
				const glm::mat4 worldTransform = draggedNode->GetWorldModelMatrix();

				// re-parent the node
				if (auto oldParent = draggedNode->GetParent()) oldParent->RemoveChild(draggedNode);
				targetNode->AddChild(draggedNode);

				// calculate the new local transform to maintain the original world transform
				const glm::mat4 parentWorld = targetNode->GetWorldModelMatrix();
				const glm::mat4 parentWorldInverse = glm::inverse(parentWorld);
				const glm::mat4 newLocalTransform = parentWorldInverse * worldTransform;

				// decompose the new local matrix and apply it to the dragged node
				glm::vec3 scale, translation, skew;
				glm::quat rotation;
				glm::vec4 perspective;
				glm::decompose(newLocalTransform, scale, rotation, translation, skew, perspective);

				draggedNode->SetPosition(translation);
				draggedNode->SetRotation(rotation);
				draggedNode->SetScale(scale);

				std::cout << "[INFO::GUI] Reparented node '" << draggedNode->GetName()
					<< "' to '" << targetNode->GetName() << "'" << std::endl;
			}
		}

		ImGui::EndDragDropTarget(); // end the drop target
	}

	// consume the pending open request for this node if it exists after drawing the node once
	if (m_sceneGraphPendingOpenIds.contains(node->GetId()))
		m_sceneGraphPendingOpenIds.erase(node->GetId());

	// always scroll the selected row into view (do not gate on auto-open set)
	if (isSelected) ImGui::SetScrollHereY();

	// handle selection on click
	if (ImGui::IsItemClicked()) handleNodeSelection(node->GetId());

	// only pop the tree if it was pushed (i.e. node has children)
	const bool treePushed = (flags & ImGuiTreeNodeFlags_NoTreePushOnOpen) == 0;

	if (nodeOpen && treePushed)
	{ // if the node is open and has childrenn (i.e. tree level was pushed), draw its children recursively
		for (const auto& child : node->GetChildren())
			if (child) // ensure child is valid
				drawNodeTreeRecursive(child); // recursive call for child nodes

		ImGui::TreePop(); // end the tree node
	}

	ImGui::PopID(); // pop the unique ID
}

void GUI::handleNodeSelection(std::uint32_t nodeId)
{
	// get the node manager and selection manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();
	auto& selectionManager = Core::GetInstance()->GetSelectionManager();

	// store the original node id for comparison and logging
	std::uint32_t originalNodeId = nodeId;
	// resolve gizmo to parent light if applicable
	auto node = nodeManager->GetNodeById(nodeId);
	if (node && node->GetGizmoType() != GizmoType::NONE)
	{
		// this is a gizmo, resolve to parent light
		std::uint32_t gizmoId = nodeId; // store the gizmo id for logging
		auto parent = node->GetParent();
		if (parent && parent->GetNodeType() == NodeType::LIGHT)
		{ // if the parent exists and is a light, use its id instead and update the node pointer
			nodeId = parent->GetId();
			node = parent;

			std::cout << "[INFO::GUI::handleNodeSelection] Resolved gizmo model '"
				<< node->GetName() << "' (ID " << gizmoId << ") to its parent light '"
				<< node->GetName() << "' (ID " << nodeId << ")" << std::endl;
		}
		else
		{
			// otherwise, print a warning and return without changing selection
			std::cerr << "[WARNING::GUI::handleNodeSelection] Could not resolve gizmo model '"
				<< node->GetName() << "' (ID " << gizmoId << ") to a parent light. Selection will not change"
				<< std::endl;
			return;
		}
	}

	// if the selected node is already selected, do not proceed (no redundant selection and logging)
	if (selectionManager->GetSelectedNodeId() == originalNodeId) return;

	// set the selected node in the selection manager and log the selection
	selectionManager->SetSelectedNodeId(nodeId);

	std::cout << "[INFO::GUI::handleNodeSelection] Selected node '"
		<< (node ? node->GetName() : "Unknown") << "' (ID " << nodeId << ")" << std::endl;
}

void GUI::updateSceneGraphAutoOpenSet()
{
	// get the node manager and selection manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();
	auto& selectionManager = Core::GetInstance()->GetSelectionManager();

	// get the currently selected node id
	const std::uint32_t selectedId = selectionManager->GetSelectedNodeId();

	// if selection did not change, do not touch open state (allows manual collapsing)
	if (selectedId == m_lastAutoOpenSelectedId) return;
	m_lastAutoOpenSelectedId = selectedId; // update last selected id

	m_sceneGraphAutoOpenIds.clear(); // clear previous auto-open set

	if (selectedId == 0) return; // no selection, nothing to auto-open, return

	// get the currently selected node
	auto selected = selectionManager->GetSelectedNode(nodeManager.get());
	if (!selected) return; // safety check, return if selected node is invalid

	// even though gizmo nodes are hidden from the tree view,
	// if a gizmo node is selected, do not auto-open anything
	if (selected->GetGizmoType() != GizmoType::NONE) return;

	// build the chain of node ids from the selected node up to the root
	// (only open the ancestor chain, not the selected node itself, which
	// reveals the selected node but does not auto-expand it)
	for (auto parent = selected->GetParent(); parent; parent = parent->GetParent())
	{
		m_sceneGraphAutoOpenIds.insert(parent->GetId());
		// arm open requests for this selection change
		m_sceneGraphPendingOpenIds.insert(parent->GetId());
	}
}


void GUI::drawLightControls(Light* light)
{
	if (!light) return; // safety check

	if (auto gizmo = light->GetGizmo())
	{ // if the light has a gizmo, create a checkbox to toggle its visibility
		bool gizmoVisible = gizmo->IsVisible(); // get current visibility state
		if (ImGui::Checkbox("Show Gizmo", &gizmoVisible)) // if the checkbox state is changed
			gizmo->SetVisible(gizmoVisible); // update the gizmo visibility accordingly

		ImGui::Separator(); // add a separator after the checkbox
	}

	switch (light->GetLightType()) // switch based on the type of the light
	{
		case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
		{
			// dynamically cast the light to a DirectionalLight object
			auto directionalLight = dynamic_cast<DirectionalLight*>(light);
			// draw controls for the directional light
			drawDirectionalLightControls(directionalLight);
		}
		break;
		case LightType::POINT_LIGHT: // if the Light is a PointLight
		{
			// dynamically cast the light to a PointLight object
			auto pointLight = dynamic_cast<PointLight*>(light);
			// draw controls for the point light
			drawPointLightControls(dynamic_cast<PointLight*>(light));
		}
		break;
		case LightType::SPOTLIGHT: // if the Light is a Spotlight
		{
			// dynamically cast the light to a Spotlight object
			auto spotlight = dynamic_cast<Spotlight*>(light);
			// draw controls for the spotlight
			drawSpotlightControls(dynamic_cast<Spotlight*>(light));
		}
		break;
		case LightType::UNDEFINED: // if the Light is of an undefined type
			std::cerr << "[ERROR::GUI::drawLightControls] UNDEFINED light type for light '"
				<< light->GetName() << "'" << std::endl;
			return;
		default: // if the Light is of an unknown type
			std::cerr << "[ERROR::GUI::drawLightControls] Unknown light type for light '"
				<< light->GetName() << "'" << std::endl;
			return;
	}
}

void GUI::drawDirectionalLightControls(DirectionalLight* directionalLight)
{
	// use PushID to create a unique ID for each directional light 
	ImGui::PushID(directionalLight->GetName().c_str());

	// get the color of the directional light
	glm::vec3 color = directionalLight->GetDiffuse();
	// create a color picker for the directional light's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		directionalLight->SetDiffuse(color); // set the new color of the directional light
	}

	// get the position of the directional light (it only serves visualization purposes)
	glm::vec3 pos = directionalLight->GetPosition();
	// create a control for the x, y, and z components of the directional light's position
	if (drawVec3Control("Position", pos, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		directionalLight->SetPosition(pos); // set the new position of the directional light
	}

	// retrieve the gizmo of the directional light and its rotation in Euler angles
	auto gizmo = directionalLight->GetGizmo();
	if (gizmo)
	{ // only proceed if the gizmo exists
		glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
		// create a control for the x, y, and z components of the directional light's rotation
		if (drawVec3Control("Rotation", rotDegrees, false, // this is not a scale control
							MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
							INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
		{ // if the control is used
			// set the new rotation of the directional light's gizmo
			gizmo->SetRotationInEulerAngles(rotDegrees);
			// update the light's direction based on the gizmo's new forward vector,
			// without causing the gizmo to be re-oriented by SetForward() again
			directionalLight->SetDirectionOnly(gizmo->GetForward());
		}
	}

	ImGui::PopID(); // use PopID to end the unique ID scope for the directional light
}

void GUI::drawPointLightControls(PointLight* pointLight)
{
	// use PushID to create a unique ID for each point light 
	ImGui::PushID(pointLight->GetName().c_str());

	// get the color of the point light
	glm::vec3 color = pointLight->GetDiffuse();
	// create a color picker for the point light's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		pointLight->SetDiffuse(color); // set the new color of the point light
	}

	// get the position of the point light
	glm::vec3 pos = pointLight->GetPosition();
	// create a control for the x, y, and z components of the point light's position
	if (drawVec3Control("Position", pos, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		pointLight->SetPosition(pos); // set the new position of the point light
	}

	ImGui::PopID(); // use PopID to end the unique ID scope for the point light
}

void GUI::drawSpotlightControls(Spotlight* spotlight)
{
	// use PushID to create a unique ID for each spotlight
	ImGui::PushID(spotlight->GetName().c_str());

	// get the color of the spotlight
	glm::vec3 color = spotlight->GetDiffuse();
	// create a color picker for the spotlight's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		spotlight->SetDiffuse(color); // set the new color of the spotlight
	}

	// get the position of the spotlight
	glm::vec3 pos = spotlight->GetPosition();
	// create a control for the x, y, and z components of the spotlight's position
	if (drawVec3Control("Position", pos, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		spotlight->SetPosition(pos); // set the new position of the spotlight
	}

	// retrieve the gizmo of the spotlight and its rotation in Euler angles
	auto gizmo = spotlight->GetGizmo();
	if (gizmo)
	{ // only proceed if the gizmo exists
		glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
		// create a control for the x, y, and z components of the spotlight's rotation
		if (drawVec3Control("Rotation", rotDegrees, false, // this is not a scale control
							MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
							INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
		{ // if the control are used
						// set the new rotation of the spotlight's gizmo
			gizmo->SetRotationInEulerAngles(rotDegrees);
			// update the light's direction based on the gizmo's new forward vector,
			// without causing the gizmo to be re-oriented by SetForward() again
			spotlight->SetDirectionOnly(gizmo->GetForward());
		}
	}

	// get the inner and outer cut-off angles of the spotlight and 
	// convert them from radians (cosine) to degrees for the controls
	float innerCutOff = glm::degrees(glm::acos(spotlight->GetInnerCutOff()));
	float outerCutOff = glm::degrees(glm::acos(spotlight->GetOuterCutOff()));
	// create controls for the inner and outer cut-off angles of the spotlight
	ImGui::Text("Cut-off Angles (in degrees):");
	if (drawFloatControl("Inner", innerCutOff,
						 MIN_CUTOFF_VALUE, outerCutOff, // innerCutOff <= outerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, INNER_CUTOFF_RESET_VALUE))
	{
		// if the control is used, convert back to radians and cosine 
		// and set the new inner cut-off angle for the spotlight
		spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
	}
	if (drawFloatControl("Outer", outerCutOff,
						 innerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, OUTER_CUTOFF_RESET_VALUE))
	{
		// if the control is used, convert back to radians and cosine 
		// and set the new outer cut-off angle for the spotlight
		spotlight->SetOuterCutOff(glm::cos(glm::radians(outerCutOff)));
	}

	ImGui::PopID(); // use PopID to end the unique ID scope for the spotlight
}

void GUI::drawModelControls(Node* model)
{
	// use PushID to create a unique ID for each model
	ImGui::PushID(model->GetName().c_str());

	// only if the model is a shape, display the color picker
	if (model->GetNodeType() == NodeType::SHAPE_MODEL)
	{
		// get the color of the model (with alpha channel)
		glm::vec4 color = model->GetAlbedo();
		// create a color picker for the model's color
		if (drawColorControl("Albedo", color))
		{ // if the color control is used
			model->SetAlbedo(color); // set the new color of the model
		}
	}

	// get the position of the model
	glm::vec3 pos = model->GetPosition();
	// create a control for the x, y, and z components of the model's position
	if (drawVec3Control("Position", pos, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		model->SetPosition(pos); // set the new position of the model
	}

	// get the rotation in Euler angles
	glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the model's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // this is not a scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
	{ // if the control is used
		model->SetRotationInEulerAngles(rotDegrees); // set the new rotation of the model
	}

	// get the scale of the model
	glm::vec3 scale = model->GetScale();
	// if the model is a shape, use a faster speed for scaling, otherwise use the default speed
	float speed = model->GetNodeType() == NodeType::SHAPE_MODEL ? SCALE_SPEED * 5.0f : SCALE_SPEED;
	// create a control for the x, y, and z components of the model's scale
	if (drawVec3Control("Scale", scale, true, // this is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, speed, SCALE_RESET_VALUE))
	{ // if the control is used
		model->SetScale(scale); // set the new scale of the model
	}

	ImGui::PopID(); // use PopID to end the unique ID scope for the model
}


void GUI::drawCreateDirectionalLightPopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	if (ImGui::BeginPopupModal("Create Directional Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Directional Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and direction of the new directional light
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedoRGB = m_newAlbedo;
		drawColorControl("Albedo", albedoRGB);
		m_newAlbedo = glm::vec4(albedoRGB, 1.0f); // set alpha to 1.0f
		drawVec3Control("Position", m_newPosition, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false, // this is not a scale control
						MIN_DIRECTION_VALUE, MAX_DIRECTION_VALUE,
						INPUT_FIELD_WIDTH, DIRECTION_SPEED, DIRECTION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new directional light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new directional light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}

		// display a button to add the new directional light
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// create a new directional light with a placeholder name and the specified properties
			auto directionalLight = std::make_shared<DirectionalLight>(
				"Directional Light", glm::vec3{ 0.1f }, m_newAlbedo,
				glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);

			// convert the light's ID to string and set it as part of the light's name
			directionalLight->SetName(StringUtils::GenerateIdPrefixedName(directionalLight));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			directionalLight->CreateGizmo();
			// register the gizmo child for selection/picking before moving the light
			auto directionalLightGizmo = directionalLight->GetGizmo();
			if (directionalLightGizmo) nodeManager->AddNode(directionalLightGizmo);
			nodeManager->AddNode(std::move(directionalLight));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::drawCreatePointLightPopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	if (ImGui::BeginPopupModal("Create Point Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Point Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color and position of the new point light
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedoRGB = m_newAlbedo;
		drawColorControl("Albedo", albedoRGB);
		m_newAlbedo = glm::vec4(albedoRGB, 1.0f); // set alpha to 1.0f
		drawVec3Control("Position", m_newPosition, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new point light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new point light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new point light
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// create a new point light with a placeholder name and the specified properties
			auto pointLight = std::make_shared<PointLight>(
				"Point Light", glm::vec3{ 0.1f }, m_newAlbedo,
				glm::vec3{ 1.0f }, m_newPosition
			);

			// convert the light's ID to string and set it as part of the light's name
			pointLight->SetName(StringUtils::GenerateIdPrefixedName(pointLight));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			pointLight->CreateGizmo();
			// register the gizmo child for selection/picking before moving the light
			auto pointLightGizmo = pointLight->GetGizmo();
			if (pointLightGizmo) nodeManager->AddNode(pointLightGizmo);
			nodeManager->AddNode(std::move(pointLight));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}
		ImGui::EndPopup();
	}
}

void GUI::drawCreateSpotlightPopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	if (ImGui::BeginPopupModal("Create Spotlight", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Spotlight\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, direction, and cut-off angles of the new spotlight
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedoRGB = m_newAlbedo;
		drawColorControl("Albedo", albedoRGB);
		m_newAlbedo = glm::vec4(albedoRGB, 1.0f); // set alpha to 1.0f
		drawVec3Control("Position", m_newPosition, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false, // this is not a scale control
						MIN_DIRECTION_VALUE, MAX_DIRECTION_VALUE,
						INPUT_FIELD_WIDTH, DIRECTION_SPEED, DIRECTION_RESET_VALUE);
		ImGui::Text("Cut-Off Angles (in degrees):");
		// initialize the inner and outer cut-off angles with default values before drawing the controls
		drawFloatControl("Inner", m_newInnerCutOff,
						 MIN_CUTOFF_VALUE, MAX_CUTOFF_VALUE, // innerCutOff <= outerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, INNER_CUTOFF_RESET_VALUE);
		drawFloatControl("Outer", m_newOuterCutOff,
						 m_newInnerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, OUTER_CUTOFF_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new spotlight
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new spotlight's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
			m_newInnerCutOff = m_randomizer->GenerateRandomFloat(
				MIN_CUTOFF_VALUE,
				MAX_CUTOFF_VALUE);
			m_newOuterCutOff = m_randomizer->GenerateRandomFloat(
				m_newInnerCutOff, // ensure outer cut-off is greater than inner cut-off
				MAX_CUTOFF_VALUE);
		}

		// display a button to add the new spotlight
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// create a new spotlight with a placeholder name and the specified properties
			auto spotlight = std::make_shared<Spotlight>(
				"Spotlight", glm::vec3{ 0.1f }, m_newAlbedo,
				glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);

			// convert the light's ID to string and set it as part of the light's name
			spotlight->SetName(StringUtils::GenerateIdPrefixedName(spotlight));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			spotlight->CreateGizmo();
			// register the gizmo child for selection/picking before moving the light
			auto spotlightGizmo = spotlight->GetGizmo();
			if (spotlightGizmo) nodeManager->AddNode(spotlightGizmo);
			nodeManager->AddNode(std::move(spotlight));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::drawCreatePlaneShapePopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	if (ImGui::BeginPopupModal("Create Plane Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Plane Shape\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and size of the new plane shape
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Rotation", m_newRotation, false, // this is not a scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE);
		drawVec3Control("Scale", m_newScale, true, // this is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, SCALE_SPEED * 5.0f, SCALE_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new plane shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new plane shape's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new plane shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// create a plane shape with a placeholder name and the specified properties
			auto planeShape = std::make_shared<Shape>(
				"Plane Shape", planeVerticesVec, planeIndicesVec, m_newAlbedo, m_newPosition,
				glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, m_newScale
			);
			// indicate that the plane is a two-sided shape
			planeShape->SetTwoSided(true);
			// set the rotation in Euler angles of the new plane shape
			planeShape->SetRotationInEulerAngles(m_newRotation);

			// convert the shape's ID to string and set it as part of the shape's name
			planeShape->SetName(StringUtils::GenerateIdPrefixedName(planeShape));

			// add the new plane shape to the engine
			nodeManager->AddNode(std::move(planeShape));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::drawCreateCubeShapePopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	if (ImGui::BeginPopupModal("Create Cube Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Cube Shape\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and size of the new cube shape
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false, // this is not a scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Rotation", m_newRotation, false, // this is not a scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE);
		drawVec3Control("Scale", m_newScale, true, // this is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, SCALE_SPEED * 5.0f, SCALE_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new cube shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new cube shape's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new cube shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// create a cube shape with a placeholder name and the specified properties
			auto cubeShape = std::make_shared<Shape>(
				"Cube Shape", cubeVerticesVec, cubeIndicesVec, m_newAlbedo, m_newPosition,
				glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, m_newScale
			);
			// set the rotation of the new cube shape
			cubeShape->SetRotationInEulerAngles(m_newRotation);

			// convert the shape's ID to string and set it as part of the shape's name
			cubeShape->SetName(StringUtils::GenerateIdPrefixedName(cubeShape));

			// add the new cube shape to the engine
			nodeManager->AddNode(std::move(cubeShape));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::drawImportModelPopup()
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for display size

	// set the size and position of the next window to display the file dialog adequately,
	// centered on the display and with a predefined size
	ImGui::SetNextWindowSize(ImVec2(FILE_DIALOG_POPUP_WIDTH, FILE_DIALOG_POPUP_HEIGHT),
							 ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - FILE_DIALOG_POPUP_WIDTH * 0.5f,
								   io.DisplaySize.y * 0.5f - FILE_DIALOG_POPUP_HEIGHT * 0.5f),
							ImGuiCond_Appearing);

	// check if the file dialog is displayed and if the user selected a file
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{ // if the file dialog is displayed
		if (ImGuiFileDialog::Instance()->IsOk())
		{ // if the user clicked the OK button (or double-clicked a file, i.e. selected a file)
			// get the selected file path and name
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			// normalize slashes to forward slashes for cross-platform texture loading
			std::replace(filePathName.begin(), filePathName.end(), '\\', '/');

			// create a new model with the file name as a placeholder name and the selected file path
			auto assimpModel = std::make_shared<AssimpModel>(fileName, filePathName);

			// remove the extension from the file name for the model's name
			std::string assimpModelName = fileName.substr(0, fileName.find_last_of('.'));
			// uppercase the first letter of the model's name
			assimpModelName[0] = std::toupper(assimpModelName[0]);
			// convert the model's ID to string and set it as part of the model's name
			assimpModel->SetName(StringUtils::GenerateIdPrefixedName(assimpModel));

			// add the new model to the engine
			nodeManager->AddNode(std::move(assimpModel));
		}

		ImGuiFileDialog::Instance()->Close(); // close the file dialog
	}
}

void GUI::drawImportSkyboxPopup()
{
	// get the scene manager from the Core instance
	auto& sceneManager = Core::GetInstance()->GetSceneManager();

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for display size

	// static state for the error modals and reopening the file dialog, preserved across frames
	static bool showCountError{ false };			// true if number of selected images is != 6 or 1
	static bool showMappingError{ false };			// true if the selected images cannot be mapped
	static std::string errorPopupMessage{};			// message to display in the error popup
	static bool shouldReopenFileDialog{ false };	// true if the file dialog should be reopened

	// set the size and position of the next window to display the file dialog adequately
	ImGui::SetNextWindowSize(ImVec2(FILE_DIALOG_POPUP_WIDTH, FILE_DIALOG_POPUP_HEIGHT),
							 ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - FILE_DIALOG_POPUP_WIDTH * 0.5f,
								   io.DisplaySize.y * 0.5f - FILE_DIALOG_POPUP_HEIGHT * 0.5f),
							ImGuiCond_Appearing);

	// main loop for the file dialog display and handling
	if (ImGuiFileDialog::Instance()->Display("ChooseSkyboxDlgKey"))
	{ // if the file dialog is displayed
		if (ImGuiFileDialog::Instance()->IsOk())
		{ // if the user clicked the OK button (or double-clicked a file)
			const auto& selection = ImGuiFileDialog::Instance()->GetSelection();
			bool validationPassed{ true }; // assume validation passed unless an error is found

			// local helper lambda to parse cubemap face names from a set of paths
			// (this avoids code duplication between the validation and import steps)
			auto parseCubemapFaces =
				[](const std::vector<std::string>& paths,
				   std::vector<std::string>& orderedFacePaths) -> bool
			{
				// get the number of cubemap faces
				constexpr std::size_t cubemapFaceCount = static_cast<std::size_t>(CubemapFaceIndex::COUNT);
				// prepare arrays to hold ordered paths and track assigned faces
				std::array<std::string, cubemapFaceCount> orderedPaths{};
				std::array<bool, cubemapFaceCount> pathAssigned{};

				// iterate over the selected file paths to infer face ordering
				for (const auto& fullPath : paths)
				{
					// extract filename from the full path and convert to lowercase
					const std::string fileName =
						StringUtils::ToLowercaseFromCopy(fullPath.substr(fullPath.find_last_of("/\\") + 1));

					bool matched{ false }; // track if a match was found for this filename

					if (fileName.find("right") != std::string::npos ||
						fileName.find("posx") != std::string::npos ||
						fileName.find("px") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::RIGHT, fullPath);
					else if (fileName.find("left") != std::string::npos ||
							 fileName.find("negx") != std::string::npos ||
							 fileName.find("nx") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::LEFT, fullPath);
					else if (fileName.find("top") != std::string::npos ||
							 fileName.find("posy") != std::string::npos ||
							 fileName.find("py") != std::string::npos ||
							 fileName.find("up") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::TOP, fullPath);
					else if (fileName.find("bottom") != std::string::npos ||
							 fileName.find("negy") != std::string::npos ||
							 fileName.find("ny") != std::string::npos ||
							 fileName.find("down") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::BOTTOM, fullPath);
					else if (fileName.find("front") != std::string::npos ||
							 fileName.find("posz") != std::string::npos ||
							 fileName.find("pz") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::FRONT, fullPath);
					else if (fileName.find("back") != std::string::npos ||
							 fileName.find("negz") != std::string::npos ||
							 fileName.find("nz") != std::string::npos)
						matched = AssignCubemapFace(orderedPaths, pathAssigned,
													CubemapFaceIndex::BACK, fullPath);

					if (!matched) return false; // if no match was found, parsing fails for this filename
				}

				// if not all faces have been assigned a path, parsing fails
				if (!std::all_of(pathAssigned.begin(), pathAssigned.end(),
								 [](bool b) { return b; })) return false;

				// on success, populate the output vector and return true
				orderedFacePaths.assign(orderedPaths.begin(), orderedPaths.end());
				return true;
			};

			// validate the number of selected files and proceed accordingly
			if (selection.size() == 1)
			{ // if the user selected a single file (assumed to be an HDR equirectangular map)
				const std::string& singleFileName = selection.begin()->first; // get the filename
				std::string lowerName = singleFileName;
				std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
							   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				bool isHDR = lowerName.size() >= 4 && lowerName.rfind(".hdr") == lowerName.size() - 4;

				if (!isHDR)
				{ // if the selected file is not an .hdr file, the selection is invalid
					validationPassed = false;
					showCountError = true;
					errorPopupMessage = "A single file must have the .hdr extension";
					std::cerr << "[ERROR::GUI::drawImportSkyboxPopup] Single selected file is not .hdr: "
						<< singleFileName << std::endl;
				}
			}
			else if (selection.size() == 6)
			{ // if the user selected six files (assumed to form a cubemap), attempt to infer order
				std::vector<std::string> paths;
				for (const auto& pair : selection)
				{
					std::string path = pair.second;
					std::replace(path.begin(), path.end(), '\\', '/');
					paths.push_back(path);
				}

				std::vector<std::string> orderedFacePaths; // will be populated by the parser
				if (!parseCubemapFaces(paths, orderedFacePaths))
				{ // if parsing fails, the selection is invalid
					validationPassed = false;
					showMappingError = true;
					errorPopupMessage =
						"Could not infer cubemap face ordering from file names.\n"
						"Filenames must contain tokens like: right, left, top, bottom, front, back.\n"
						"Alternatives: posx, negx, posy, negy, posz, negz; or px, nx, py, ny, pz, nz.\n"
						"Up/down are also accepted for top/bottom";
					std::cerr << "[ERROR::GUI::drawImportSkyboxPopup] "
						"Could not map cubemap faces from filenames, valid names must contain "
						"tokens like:\n\tright, left, top, bottom, front, back; or posx, negx, posy, negy, "
						"posz, negz; \n\tor px, nx, py, ny, pz, nz. "
						"Up/down are also accepted for top/bottom" << std::endl;
				}
			}
			else
			{ // if the user selected a number of files other than 1 or 6, it's an invalid selection
				validationPassed = false;
				showCountError = true;
				errorPopupMessage = "Invalid number of files selected.\n"
					"Please select either 1 .hdr file or 6 image files (.png, .jpg, .jpeg)";
				std::cerr << "[ERROR::GUI::drawImportSkyboxPopup] Invalid number of files selected: "
					<< selection.size() << std::endl;
			}

			// handle the successful validation case by importing the corresponding skybox texture
			if (validationPassed)
			{
				if (selection.size() == 1)
				{ // create the skybox texture from the equirectangular .hdr image
					std::string path = selection.begin()->second;
					std::replace(path.begin(), path.end(), '\\', '/');
					auto skyboxTexture = std::make_shared<Texture>("Skybox HDR Equirectangular",
																   path, true);
					sceneManager->SetSkybox(std::move(skyboxTexture));
					std::cout << "[SUCCESS::GUI::drawImportSkyboxPopup] "
						"Successfully imported HDR equirectangular skybox" << std::endl;
				}
				else if (selection.size() == 6)
				{ // create the cubemap texture from the ordered face paths
					std::vector<std::string> paths;
					for (const auto& pair : selection)
					{
						std::string path = pair.second;
						std::replace(path.begin(), path.end(), '\\', '/');
						paths.push_back(path);
					}
					std::vector<std::string> orderedFacePaths;
					parseCubemapFaces(paths, orderedFacePaths); // will succeed as it was validated
					auto skyboxTexture = std::make_shared<Texture>("Skybox Cubemap", orderedFacePaths);
					sceneManager->SetSkybox(std::move(skyboxTexture));
					std::cout << "[SUCCESS::GUI::drawImportSkyboxPopup] Successfully imported cubemap skybox"
						<< std::endl;
				}
				shouldReopenFileDialog = false; // ensure no dialog is reopened on success
			}
			else
			{ // on failure, set flag to reopen the file dialog when the error popup is closed
				shouldReopenFileDialog = true;
			}

			// always close the file dialog after processing the selection, valid or invalid
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{ // if the user cancelled the file dialog (i.e., closed without a selection)
			// reset any error state and ensure the dialog is not reopened automatically
			shouldReopenFileDialog = false;
			showCountError = false;
			showMappingError = false;
			errorPopupMessage.clear();
			ImGuiFileDialog::Instance()->Close(); // close the file dialog instance
		}
	}

	// local constant flag indicating if any error popup should be shown this frame
	const bool isErrorModalOpen = showCountError || showMappingError;

	// while an error is active, ensure the error popup is kept open as a modal
	if (isErrorModalOpen) ImGui::OpenPopup("Skybox Import Error");

	// set the size and position of the error popup
	ImGui::SetNextWindowSize(ImVec2(ERROR_POPUP_WIDTH, ERROR_POPUP_HEIGHT), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - ERROR_POPUP_WIDTH * 0.5f,
								   io.DisplaySize.y * 0.5f - ERROR_POPUP_HEIGHT * 0.5f),
							ImGuiCond_Appearing);

	// error modal popup definition
	if (ImGui::BeginPopupModal("Skybox Import Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the error popup is open, ensure it is focused and displays the error message
		ImGui::SetWindowFocus();
		ImGui::TextWrapped("%s", errorPopupMessage.c_str());

		// a bit of vertical spacing before the OK button
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		// center the OK button horizontally within the popup
		ImVec2 buttonSize{ BUTTON_WIDTH, 0.0f };
		float availWidth = ImGui::GetContentRegionAvail().x;
		float offsetX = (availWidth - buttonSize.x) * 0.5f;
		if (offsetX > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

		if (ImGui::Button("OK", ImVec2(BUTTON_WIDTH, 0.0f)))
		{ // if the OK button is clicked, clear error state and close the popup
			showCountError = false;
			showMappingError = false;
			errorPopupMessage.clear();
			ImGui::CloseCurrentPopup();

			// only reopen the file dialog if indicated (i.e., after a failed validation)
			if (shouldReopenFileDialog)
			{
				// file dialog configuration (same as used when first opened)
				IGFD::FileDialogConfig fileDialogConfig;
				fileDialogConfig.path = "./resources/textures/skyboxes";
				fileDialogConfig.countSelectionMax = 6;
				fileDialogConfig.flags = ImGuiFileDialogFlags_Modal;

				ImGuiFileDialog::Instance()->OpenDialog(
					"ChooseSkyboxDlgKey", "Choose Skybox Image Files", ".jpg,.jpeg,.png,.hdr",
					fileDialogConfig
				);
				shouldReopenFileDialog = false; // reset the flag
			}
		}
		ImGui::EndPopup(); // end the error popup definition
	}
}


bool GUI::drawColorControl(const std::string& label, glm::vec3& color, bool showLabel,
						   float colorPickerWidth)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	auto boldFont = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	if (showLabel) // if indicated, display the label for the control
		ImGui::Text("%s", label.c_str());

	ImGui::SetNextItemWidth(colorPickerWidth); // set the width of the color picker
	if (ImGui::ColorEdit3("##color", (float*)&color))
	{ // if the color picker is used
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed; // return whether the color has changed
}

bool GUI::drawColorControl(const std::string& label, glm::vec4& color, bool showLabel,
						   float colorPickerWidth)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	auto boldFont = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	if (showLabel) // if indicated, display the label for the control
		ImGui::Text("%s", label.c_str());

	ImGui::SetNextItemWidth(colorPickerWidth); // set the width of the color picker
	if (ImGui::ColorEdit4("##color", (float*)&color)) // color picker for vec4 (includes alpha component)
	{ // if the color picker is used
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed; // return whether the color has changed
}

bool GUI::drawVec3Control(const std::string& label, glm::vec3& values, bool scaleControls,
						  float minInputFieldValue, float maxInputFieldValue, float inputFieldWidth,
						  float speed, float resetValue, float resetButtonWidth, float resetButtonHeight)
{
	bool value_changed{ false }; // flag to indicate if any value has changed

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	// if the controls are for scaling, display a checkbox to enable proportional scaling
	if (scaleControls)
		ImGui::Checkbox("Proportional", &s_proportionalScaling);

	// store previous values so that we can check if any value has changed
	glm::vec3 prevValues = values;

	// x component
	ImGui::PushID("x");
	ImGui::Text("  X  "); // display the label for the x component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the x component
	if (ImGui::InputFloat("##x", &values.x,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upX", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.x += speed; // increase the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downX", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.x -= speed; // decrease the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, resetButtonHeight)))
	{ // if the Reset button is pressed
		values.x = resetValue; // reset the x component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the x component

	// y component
	ImGui::PushID("y");
	ImGui::Text("  Y  "); // display the label for the y component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the y component
	if (ImGui::InputFloat("##y", &values.y,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upY", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.y += speed; // increase the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downY", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.y -= speed; // decrease the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, resetButtonHeight)))
	{ // if the Reset button is pressed
		values.y = resetValue; // reset the y component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the y component

	// z component
	ImGui::PushID("z");
	ImGui::Text("  Z  "); // display the label for the z component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the z component
	if (ImGui::InputFloat("##z", &values.z,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upZ", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.z += speed; // increase the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	if (ImGui::ArrowButton("##downZ", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.z -= speed; // decrease the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, resetButtonHeight)))
	{ // if the Reset button is pressed
		values.z = resetValue; // reset the z component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the z component

	ImGui::PopID(); // end the unique ID scope for the label

	// clamp values to the specified range
	values.x = std::clamp(values.x, minInputFieldValue, maxInputFieldValue);
	values.y = std::clamp(values.y, minInputFieldValue, maxInputFieldValue);
	values.z = std::clamp(values.z, minInputFieldValue, maxInputFieldValue);

	// if proportional scaling is enabled, adjust the other components accordingly
	if (scaleControls && s_proportionalScaling)
	{
		if (values.x != prevValues.x)
		{
			values.y = values.z = values.x;
			value_changed = true;
		}
		else if (values.y != prevValues.y)
		{
			values.x = values.z = values.y;
			value_changed = true;
		}
		else if (values.z != prevValues.z)
		{
			values.x = values.y = values.z;
			value_changed = true;
		}
	}

	// update the value_changed flag if any of the components have changed
	if (values.x != prevValues.x || values.y != prevValues.y || values.z != prevValues.z)
		value_changed = true;

	return value_changed;
}

bool GUI::drawFloatControl(const std::string& label, float& value,
						   float minInputFieldValue, float maxInputFieldValue, float inputFieldWidth,
						   float speed, float resetValue, float resetButtonWidth, float resetButtonHeight)
{
	bool value_changed{ false }; // flag to indicate if the value has changed

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field
	if (ImGui::InputFloat("##val", &value,
						  0.0f, 0.0f, // no step buttons
						  "%.2f"))
	{ // if the input field is used
		value_changed = true;
	} // set the value_changed flag to true

	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	ImGui::SameLine();
	if (ImGui::ArrowButton("##up", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		value += speed; // increase the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##down", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		value -= speed; // decrease the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, resetButtonHeight)))
	{ // if the Reset button is pressed
		value = resetValue; // reset the value to the reset value
		value_changed = true; // set the value_changed flag to true
	}

	// clamp the value to the specified range
	value = std::clamp(value, minInputFieldValue, maxInputFieldValue);

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed;
}

void GUI::drawRemoveNodeButton(Node* node, std::vector<uint32_t>& nodesToRemoveIds,
							   const std::string& label, float buttonWidth, float buttonHeight)
{
	std::string buttonLabel = "Remove " + label;
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonWidth, buttonHeight)))
	{ // if the button is clicked
		nodesToRemoveIds.push_back(node->GetId()); // add the node id to the list of nodes to remove
		std::cout << "[INFO::GUI::drawRemoveNodeButton] Node '"
			<< node->GetName() << "' (ID " << node->GetId() << ") marked for removal" << std::endl;
	}
}