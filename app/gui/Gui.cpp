/*
* GUI.cpp
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a windows to display information about the scene
* and allow the user to interact with it, e.g. change certain parameters or create new objects.
*/

#include <limits> // for std::numeric_limits<float>::max()

#include "Gui.h"
#include "ImGuiFileDialog.h"
#include "../CUBE.h"

#include "../core/Core.h"
#include "../core/Node.h"
#include "../core/camera/Camera.h"
#include "../core/light/Light.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Model.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"
#include "../core/managers/NodeManager.h"
#include "../core/managers/SelectionManager.h"
#include "../core/managers/SceneManager.h"
#include "../core/managers/InputManager.h"
#include "../core/renderer/Renderer.h"
#include "../core/utils/random/Random.h"

// Static Attributes
// -----------------
bool GUI::s_proportionalScaling{ true }; // propertional scaling flag is true by default

// Constructors
// ------------
GUI::GUI()
	: m_randomizer{ std::make_unique<Random>() }, // create a random number generator
	m_newAlbedo{ 0.8f }, // default albedo color for new objects is light gray
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
	m_informationWindowPosition, m_debugWindowPosition, m_creationWindowPosition, m_propertiesWindowPosition
		= ImVec2{ 0.0f, 0.0f };
	m_informationWindowSize, m_debugWindowSize, m_creationWindowSize, m_propertiesWindowSize
		= ImVec2{ 0.0f, 0.0f };
	// initialize the flags for the windows to prevent focus on the first frame
	m_informationWindowJustAppeared = true;
	m_debugWindowJustAppeared = true;
	m_propertiesWindowJustAppeared = true;
	m_creationWindowJustAppeared = true;
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
	m_informationWindowRelativeWidth = 0.3f;
	m_informationWindowRelativeHeight = 0.6f;
	m_debugWindowRelativeWidth = 0.3f;
	m_debugWindowRelativeHeight = 0.4f;
	m_propertiesWindowRelativeWidth = 0.2f;
	m_propertiesWindowRelativeHeight = 0.8f;
	m_creationWindowRelativeWidth = 0.2f;
	m_creationWindowRelativeHeight = 0.2f;

	m_informationWindowXOffset = 0.0f;
	m_informationWindowYOffset = 0.0f;
	m_debugWindowXOffset = 0.0f;
	m_debugWindowYOffset = 1.0f - m_debugWindowRelativeHeight;
	m_propertiesWindowXOffset = 1.0f - m_propertiesWindowRelativeWidth;
	m_propertiesWindowYOffset = 0.0f;
	m_creationWindowXOffset = 1.0f - m_creationWindowRelativeWidth;
	m_creationWindowYOffset = 1.0f - m_creationWindowRelativeHeight;
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

	// position of the Information Window (top left corner with padding)
	m_informationWindowPosition = ImVec2{
		io.DisplaySize.x * m_informationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_informationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Information Window
	m_informationWindowSize = ImVec2{
		io.DisplaySize.x * m_informationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_informationWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Debug Window (bottom left corner with padding)
	m_debugWindowPosition = ImVec2{
		io.DisplaySize.x * m_debugWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_debugWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Debug Window
	m_debugWindowSize = ImVec2{
		io.DisplaySize.x * m_debugWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_debugWindowRelativeHeight - m_windowSizePadding.y // height
	};
	// position of the Properties Window (top right corner with padding)
	m_propertiesWindowPosition = ImVec2{
		io.DisplaySize.x * m_propertiesWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_propertiesWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Properties Window
	m_propertiesWindowSize = ImVec2{
		io.DisplaySize.x * m_propertiesWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_propertiesWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Create Window (bottom right corner with padding)
	m_creationWindowPosition = ImVec2{
		io.DisplaySize.x * m_creationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_creationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Create Window
	m_creationWindowSize = ImVec2{
		io.DisplaySize.x * m_creationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_creationWindowRelativeHeight - m_windowSizePadding.y // height
	};
}

void GUI::drawGUIWindows()
{
	drawInformationWindow();
	drawDebugWindow();
	drawPropertiesWindow();
	drawCreationWindow();
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


void GUI::drawInformationWindow()
{
	// set initial size and position for the Information Window
	ImGui::SetNextWindowSize(m_informationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_informationWindowPosition, ImGuiCond_Appearing);
	// set the Information Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that displays all information about the nodes in the scene
		// begin the Information window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("SCENE GRAPH INFORMATION", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		drawCamerasInformation();
		ImGui::Text("\n");
		ImGui::Separator();
		drawLightsInformation();
		ImGui::Text("\n");
		ImGui::Separator();
		drawModelsInformation();
		ImGui::Text("\n");

		ImGui::End(); // end the Information window

		if (m_informationWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_informationWindowJustAppeared = false; // no longer the first frame
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
		ImGui::Begin("DEBUG", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
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
			auto outlineParams = selectionManager->GetOutlineParams(); // copy current outline params

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
			ImGui::SetNextItemWidth(120.0f); // set a fixed width for the combo box
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
		// button to reset the camera position and orientation
		if (ImGui::Button("Reset Camera", ImVec2{ BUTTON_WIDTH * 2, BUTTON_HEIGHT }))
		{ // if the button is pressed, set the camera to its default position and orientation
			sceneManager->ResetCamera();
		}
		ImGui::SameLine(); // keep the buttons on the same line
		// button to clear the skybox
		if (ImGui::Button("Clear Skybox", ImVec2{ BUTTON_WIDTH * 2, BUTTON_HEIGHT }))
		{ // if the button is pressed, clear the skybox, i.e. remove the current skybox texture
			sceneManager->ClearSkybox();
		}

		ImGui::Text("\n"); // add some vertical spacing for better visual separation

		// rendering settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("RENDERING INFORMATION");
		ImGui::PopFont();

		// display the current FPS and frame time
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame Time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);

		// display the screen texture debug mode currently in use and the values of its parameters
		ImGui::Text("Screen Texture Debug Mode: %s", debugModes[debugModeIndex].c_str());
		switch (debugModeIndex)
		{
			case 0: // Normal mode
				break; // no parameters to display
			case 1: // Inverted Colors mode
				break; // no parameters to display
			case 2: // Picking Colors (raw picking buffer visualization)
				// no parameters to display, draw a text to explain what is shown
				ImGui::Text("\tShowing per-object encoded IDs as colors");
				break;
			case 3: // Solid Color mode
				ImGui::Text("\tSolid Color: (%.3f, %.3f, %.3f)",
							params.solidColor.r, params.solidColor.g, params.solidColor.b);
				break;
			case 4: // Grid Overlay mode
				ImGui::Text("\tGrid Line Count: %d", params.gridLineCount);
				ImGui::Text("\tGrid Line Thickness: %.2f", params.gridLineThickness);
				ImGui::Text("\tGrid Background Color: (%.3f, %.3f, %.3f)",
							params.gridBgColor.r, params.gridBgColor.g, params.gridBgColor.b);
				ImGui::Text("\tGrid Line Color: (%.3f, %.3f, %.3f)",
							params.gridLineColor.r, params.gridLineColor.g, params.gridLineColor.b);
				break;
			default:
				std::cerr << "[ERROR::GUI::drawDebugWindow] Unknown screen texture debug mode index: "
					<< debugModeIndex << std::endl;
				break;
		}

		ImGui::Text("\n"); // add some vertical spacing for better visual separation

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
				ImGui::Text("\tConfiguration");
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Solid Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// use the custom drawColorControl helper to draw the color picker
				if (drawColorControl("##SolidColor", params.solidColor,
									 false, ImGui::GetContentRegionAvail().x))
					paramsChanged = true; // mark the params as changed
			}
			break;
			case 4: // Grid Overlay mode
			{ // draw controls for each parameter
				ImGui::Text("\tConfiguration");
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
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
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
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

				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Background Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// for the background color and the line color,
				// disable the alpha channel and the inputs (only show an RGB color picker)
				if (ImGui::ColorEdit3("##GridBackgroundColor", (float*)&params.gridBgColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					paramsChanged = true; // mark the params as changed
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
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
		ImGui::Begin("PROPERTIES", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		auto selected = selectionManager->GetSelectedNode(nodeManager.get());
		if (!selected)
		{ // if no node is selected, display a message
			ImGui::Text("No node selected.\nClick an object to inspect it.");
		}
		else
		{ // if an node is selected, display its name and ID, and draw its controls
			// display the name and ID of the selected node in bold font
			ImGui::PushFont(m_boldFont);
			ImGui::TextWrapped("%s\n(ID: %u)", selected->GetName().c_str(), selected->GetId());
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

	{ // show a window that contains buttons to create new objects to the scene
		// begin the Creation window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("CREATION", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// button to create a new directional light to the scene
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

		// button to create a new point light to the scene
		if (ImGui::Button("Create Point Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Point Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}
		drawCreatePointLightPopup(); // draw the popup to create a new point light

		// button to create a new spotlight to the scene
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

		// buttom to create a new cube shape to the scene
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


void GUI::drawCamerasInformation() const
{
	// get the node manager and scene manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();
	auto& sceneManager = Core::GetInstance()->GetSceneManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("CAMERAS");
	ImGui::PopFont();

	// get the number of cameras in the scene
	unsigned int nCameras = nodeManager->GetNCameras();
	// display the number of cameras in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Cameras in the scene: %d", nCameras);
	ImGui::PopFont();

	// display the attributes of each camera in the scene
	ImGui::Text("\nCameras in the scene:");
	std::for_each(nodeManager->GetNodes("CAMERA").begin(),
				  nodeManager->GetNodes("CAMERA").end(),
				  [&](const std::shared_ptr<Node>& node)
	{ // iterate over all cameras in the node manager and display their attributes
		// dynamically cast the node to a Camera object
		auto camera = dynamic_cast<Camera*>(node.get());

		// use PushID to create a unique ID for each camera
		ImGui::PushID(camera->GetName().c_str());

		// display the attributes of the camera
		ImGui::PushFont(m_boldFont);
		ImGui::TextWrapped("\t%s", camera->GetName().c_str());
		ImGui::PopFont();
		ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
					camera->GetPosition().x,
					camera->GetPosition().y,
					camera->GetPosition().z);
		ImGui::Text("\t\tFront: (%.3f, %.3f, %.3f)",
					camera->GetFront().x,
					camera->GetFront().y,
					camera->GetFront().z);
		ImGui::Text("\t\tUp: (%.3f, %.3f, %.3f)",
					camera->GetUp().x,
					camera->GetUp().y,
					camera->GetUp().z);
		ImGui::Text("\t\tRight: (%.3f, %.3f, %.3f)",
					camera->GetRight().x,
					camera->GetRight().y,
					camera->GetRight().z);
		ImGui::Text("\t\tWorld Up: (%.3f, %.3f, %.3f)",
					camera->GetWorldUp().x,
					camera->GetWorldUp().y,
					camera->GetWorldUp().z);
		ImGui::Text("\t\tYaw: %.3f", camera->GetYaw());
		ImGui::Text("\t\tPitch: %.3f", camera->GetPitch());
		ImGui::Text("\t\tMovement Speed: %.3f", camera->GetMovementSpeed());
		ImGui::Text("\t\tMouse Sensitivity: %.3f", camera->GetMouseSensitivity());
		ImGui::Text("\t\tZoom: %.3f", camera->GetZoom());

		ImGui::PopID(); // use PopID to end the unique ID scope
	});
}

void GUI::drawLightsInformation() const
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("LIGHTS");
	ImGui::PopFont();

	// get the number of lights and of each type of light in the scene
	unsigned int nLights = nodeManager->GetNLights();
	unsigned int nDirectionalLights = nodeManager->GetNDirectionalLights();
	unsigned int nPointLights = nodeManager->GetNPointLights();
	unsigned int nSpotlights = nodeManager->GetNSpotlights();
	// display the number of lights and each type of light in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Lights in the scene: %d", nLights);
	ImGui::PopFont();
	ImGui::Text("\tNumber of Directional Lights in the scene: %d", nDirectionalLights);
	ImGui::Text("\tNumber of Point Lights in the scene: %d", nPointLights);
	ImGui::Text("\tNumber of Spotlights in the scene: %d", nSpotlights);

	// display the attributes of each light in the scene
	ImGui::Text("\nLights in the scene:");
	std::for_each(nodeManager->GetNodes("LIGHT").begin(),
				  nodeManager->GetNodes("LIGHT").end(),
				  [&](const std::shared_ptr<Node>& node)
	{ // iterate over all lights in the node manager and display their attributes
		// dynamically cast the node to a Light object
		auto light = dynamic_cast<Light*>(node.get());

		switch (light->GetLightType()) // switch based on the type of the light
		{
			case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
			{
				// get the DirectionalLight object
				auto directionalLight = dynamic_cast<DirectionalLight*>(light);

				// use PushID to create a unique ID for each light 
				ImGui::PushID(directionalLight->GetName().c_str());

				// display the attributes of the directional light (position is just for visualization)
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", directionalLight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							directionalLight->GetDiffuse().x,
							directionalLight->GetDiffuse().y,
							directionalLight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
							directionalLight->GetPosition().x,
							directionalLight->GetPosition().y,
							directionalLight->GetPosition().z);
				ImGui::Text("\t\tDirection: (%.3f, %.3f, %.3f)",
							directionalLight->GetDirection().x,
							directionalLight->GetDirection().y,
							directionalLight->GetDirection().z);

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::POINT_LIGHT: // if the Light is a PointLight
			{
				// dynamically cast the node to a PointLight object
				auto pointLight = dynamic_cast<PointLight*>(light);

				// use PushID to create a unique ID for each light 
				ImGui::PushID(pointLight->GetName().c_str());

				// display the attributes of the point light
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", pointLight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							pointLight->GetDiffuse().x,
							pointLight->GetDiffuse().y,
							pointLight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
							pointLight->GetPosition().x,
							pointLight->GetPosition().y,
							pointLight->GetPosition().z);

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::SPOTLIGHT: // if the Light is a Spotlight
			{
				// dynamically cast the node to a Spotlight object
				auto spotlight = dynamic_cast<Spotlight*>(light);

				// use PushID to create a unique ID for each spotlight 
				ImGui::PushID(spotlight->GetName().c_str());

				// display the attributes of the spotlight
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", spotlight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							spotlight->GetDiffuse().x,
							spotlight->GetDiffuse().y,
							spotlight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
							spotlight->GetPosition().x,
							spotlight->GetPosition().y,
							spotlight->GetPosition().z);
				ImGui::Text("\t\tDirection: (%.3f, %.3f, %.3f)",
							spotlight->GetDirection().x,
							spotlight->GetDirection().y,
							spotlight->GetDirection().z);
				ImGui::Text("\t\tInner cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
				ImGui::Text("\t\tOuter cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetOuterCutOff())));

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::UNDEFINED: // if the Light is of an undefined type
				std::cerr << "[ERROR::GUI::drawLightsInformation] UNDEFINED light type for light: "
					<< light->GetName() << std::endl;
				return;
			default: // if the Light is of an unknown type
				std::cerr << "[ERROR::GUI::drawLightsInformation] Unknown light type for light: "
					<< light->GetName() << std::endl;
				return;
		}
	});
}

void GUI::drawModelsInformation() const
{
	// get the node manager from the Core instance
	auto& nodeManager = Core::GetInstance()->GetNodeManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("MODELS");
	ImGui::PopFont();

	// get the number of models and of each type of model in the scene
	unsigned int nModels = nodeManager->GetNModels();
	unsigned int nAssimpModels = nodeManager->GetNAssimpModels();
	unsigned int nShapes = nodeManager->GetNShapes();
	// display the number of models and each type of model in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Models in the scene: %d", nModels);
	ImGui::PopFont();
	ImGui::Text("\tNumber of Assimp models in the scene: %d", nAssimpModels);
	ImGui::Text("\tNumber of Shapes in the scene: %d", nShapes);

	// display the attributes of each model in the scene
	ImGui::Text("\nModes in the scene:");
	std::for_each(nodeManager->GetNodes("MODEL").begin(), nodeManager->GetNodes("MODEL").end(),
				  [&](const std::shared_ptr<Node>& node)
	{ // iterate over all models in the node manager and display their attributes
		// dynamically cast the node to a Model object
		auto model = dynamic_cast<Node*>(node.get());

		ImGui::PushID(model->GetName().c_str()); // use PushID to create a unique ID for each model

		// display the attributes of the model
		ImGui::PushFont(m_boldFont);
		ImGui::TextWrapped("\t%s", model->GetName().c_str());
		ImGui::PopFont();

		if (model->GetNodeType() == NodeType::SHAPE_MODEL)
		{ // only display the albedo color for shapes
			ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
						model->GetAlbedo().x,
						model->GetAlbedo().y,
						model->GetAlbedo().z);
		}
		ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
					model->GetPosition().x,
					model->GetPosition().y,
					model->GetPosition().z);
		ImGui::Text("\t\tRotation: (%.3f, %.3f, %.3f)",
					model->GetRotationInEulerAngles().x,
					model->GetRotationInEulerAngles().y,
					model->GetRotationInEulerAngles().z);
		ImGui::Text("\t\tScale: (%.3f, %.3f, %.3f)",
					model->GetScale().x,
					model->GetScale().y,
					model->GetScale().z);
		ImGui::Text("\t\tForward: (%.3f, %.3f, %.3f)",
					model->GetForward().x,
					model->GetForward().y,
					model->GetForward().z);

		ImGui::PopID(); // use PopID to end the unique ID scope
	});
}


void GUI::drawLightControls(Light* light)
{
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
			std::cerr << "[ERROR::GUI::drawLightControls] UNDEFINED light type for light: "
				<< light->GetName() << std::endl;
			return;
		default: // if the Light is of an unknown type
			std::cerr << "[ERROR::GUI::drawLightControls] Unknown light type for light: "
				<< light->GetName() << std::endl;
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
	if (drawVec3Control("Position", pos, false, // is not the scale control
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
		if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
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

	ImGui::PopID(); // use PopID to end the unique ID scope
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
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		pointLight->SetPosition(pos); // set the new position of the point light
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
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
	if (drawVec3Control("Position", pos, false, // is not the scale control
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
		if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
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
	{ // if the control is used
					// convert back to radians and cosine and
					// set the new inner cut-off angle for the spotlight
		spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
	}
	if (drawFloatControl("Outer", outerCutOff,
						 innerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, OUTER_CUTOFF_RESET_VALUE))
	{ // if the control is used
					// convert back to radians and cosine and
					// set the new outer cut-off angle for the spotlight
		spotlight->SetOuterCutOff(glm::cos(glm::radians(outerCutOff)));
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}

void GUI::drawModelControls(Node* model)
{
	// use PushID to create a unique ID for each model
	ImGui::PushID(model->GetName().c_str());

	// only if the model is a shape, display the color picker
	if (model->GetNodeType() == NodeType::SHAPE_MODEL)
	{
		// get the color of the model
		glm::vec3 color = model->GetAlbedo();
		// create a color picker for the model's color
		if (drawColorControl("Albedo", color))
		{ // if the color control is used
			model->SetAlbedo(color); // set the new color of the model
		}
	}

	// get the position of the model
	glm::vec3 pos = model->GetPosition();
	// create a control for the x, y, and z components of the model's position
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		model->SetPosition(pos); // set the new position of the model
	}

	// get the rotation in Euler angles
	glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the model's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
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
	if (drawVec3Control("Scale", scale, true, // is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, speed, SCALE_RESET_VALUE))
	{ // if the control is used
		model->SetScale(scale); // set the new scale of the model
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
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
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false,
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
			// get the number of models and directional lights in the scene
			std::string nModels = std::to_string(nodeManager->GetNModels());
			std::string nDirectionalLights =
				std::to_string(nodeManager->GetNDirectionalLights());

			// create a new directional light with the specified properties
			auto newDirectionalLight = std::make_shared<DirectionalLight>(
				"Directional Light " + nDirectionalLights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);
			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			newDirectionalLight->CreateGizmo();
			// add the new directional light to the engine
			nodeManager->AddNode(std::move(newDirectionalLight));

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
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
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

		ImGui::SameLine();
		// display a button to add the new point light
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and point lights in the scene
			std::string nModels = std::to_string(nodeManager->GetNModels());
			std::string nPointLights = std::to_string(nodeManager->GetNPointLights());

			// create a new point light with the specified properties
			auto newPointLight = std::make_shared<PointLight>(
				"Point Light " + nPointLights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition
			);
			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			newPointLight->CreateGizmo();
			// add the new point light to the engine
			nodeManager->AddNode(std::move(newPointLight));

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
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false,
						DIRECTION_RESET_VALUE, DIRECTION_RESET_VALUE,
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
			// get the number of models and spotlights in the scene
			std::string nModels = std::to_string(nodeManager->GetNModels());
			std::string nSpotlights = std::to_string(nodeManager->GetNSpotlights());

			// create a new spotlight with the specified properties
			auto newSpotlight = std::make_shared<Spotlight>(
				"Spotlight " + nSpotlights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);
			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			newSpotlight->CreateGizmo();
			// add the new spotlight to the engine
			nodeManager->AddNode(std::move(newSpotlight));

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
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Rotation", m_newRotation, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE);
		drawVec3Control("Scale", m_newScale, true,
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
			// get the number of models in the scene
			std::string nModels = std::to_string(nodeManager->GetNModels());

			// create a cube shape with the specified properties
			auto newCubeShape = std::make_shared<Shape>(
				"Cube (Model " + nModels + ")",
				cubeVerticesVec, cubeIndicesVec,
				m_newAlbedo, m_newPosition, glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, m_newScale
			);
			// set the rotation of the new cube shape
			newCubeShape->SetRotationInEulerAngles(m_newRotation);

			// add the new cube shape to the engine
			nodeManager->AddNode(std::move(newCubeShape));

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

			// get the current number of models in the scene
			std::string nModels = std::to_string(nodeManager->GetNModels());

			// create a new model from the selected file
			auto newAssimpModel = std::make_shared<AssimpModel>(
				fileName + " (Model " + nModels + ")",
				filePathName
			);

			// add the new model to the engine
			nodeManager->AddNode(std::move(newAssimpModel));
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
				// the expected name tokens (case-insensitive) for each cubemap face are:
				// right, left, top, bottom, front, back; or posx, negx, posy, negy, posz, negz;
				// or short forms: px, nx, py, ny, pz, nz; up/down are also accepted for top/bottom
				enum FaceIndex { RIGHT = 0, LEFT, TOP, BOTTOM, FRONT, BACK, COUNT };

				// array to hold the ordered face paths
				std::array<std::string, COUNT> orderedPaths{};
				// array to track which faces have been assigned paths
				std::array<bool, COUNT> pathAssigned{};

				// lambda to convert a string to lowercase
				auto toLower = [](std::string s)
				{
					std::transform(s.begin(), s.end(), s.begin(),
								   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					return s;
				};

				// lambda to assign a path to a face if not already assigned
				auto assignFace = [&](FaceIndex idx, const std::string& path)
				{
					if (!pathAssigned[idx])
					{
						orderedPaths[idx] = path;
						pathAssigned[idx] = true;
						return true; // face successfully assigned, return true
					}
					return false; // face already assigned, return false
				};

				// iterate over the selected file paths to infer face ordering
				for (const auto& fullPath : paths)
				{
					// extract filename from the full path and convert to lowercase
					std::string fileName = toLower(fullPath.substr(fullPath.find_last_of("/\\") + 1));
					bool matched{ false };
					if (fileName.find("right") != std::string::npos ||
						fileName.find("posx") != std::string::npos ||
						fileName.find("px") != std::string::npos)
						matched |= assignFace(RIGHT, fullPath);
					else if (fileName.find("left") != std::string::npos ||
							 fileName.find("negx") != std::string::npos ||
							 fileName.find("nx") != std::string::npos)
						matched |= assignFace(LEFT, fullPath);
					else if (fileName.find("top") != std::string::npos ||
							 fileName.find("posy") != std::string::npos ||
							 fileName.find("py") != std::string::npos ||
							 fileName.find("up") != std::string::npos)
						matched |= assignFace(TOP, fullPath);
					else if (fileName.find("bottom") != std::string::npos ||
							 fileName.find("negy") != std::string::npos ||
							 fileName.find("ny") != std::string::npos ||
							 fileName.find("down") != std::string::npos)
						matched |= assignFace(BOTTOM, fullPath);
					else if (fileName.find("front") != std::string::npos ||
							 fileName.find("posz") != std::string::npos ||
							 fileName.find("pz") != std::string::npos)
						matched |= assignFace(FRONT, fullPath);
					else if (fileName.find("back") != std::string::npos ||
							 fileName.find("negz") != std::string::npos ||
							 fileName.find("nz") != std::string::npos)
						matched |= assignFace(BACK, fullPath);

					// if no match was found for this filename, parsing fails
					if (!matched) return false;
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
		ImVec2 buttonSize{ ITEM_WIDTH, 0.0f };
		float availWidth = ImGui::GetContentRegionAvail().x;
		float offsetX = (availWidth - buttonSize.x) * 0.5f;
		if (offsetX > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

		if (ImGui::Button("OK", ImVec2(ITEM_WIDTH, 0.0f)))
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


bool GUI::drawColorControl(const std::string& label, glm::vec3& color,
						   bool showLabel, float colorControlWidth)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	auto boldFont = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	if (showLabel) // if indicated, display the label for the control
		ImGui::Text("%s", label.c_str());

	ImGui::SetNextItemWidth(colorControlWidth); // set the width of the color picker's input fields
	if (ImGui::ColorEdit3("##color", (float*)&color))
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
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
		std::cout << "[INFO::GUI::drawRemoveNodeButton] "
			<< node->GetName() << " with ID " << node->GetId() << " marked for removal" << std::endl;
	}
}