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

#include "core/Core.h"
#include "core/Node.h"
#include "core/camera/Camera.h"
#include "core/light/Light.h"
#include "core/light/Directional_Light.h"
#include "core/light/Point_Light.h"
#include "core/light/Spotlight.h"
#include "core/model/Shape_Model.h"
#include "core/model/Assimp_Model.h"
#include "core/managers/Node_Manager.h"
#include "core/managers/Selection_Manager.h"
#include "core/managers/Scene_Manager.h"
#include "core/managers/Input_Manager.h"
#include "core/renderer/Renderer.h"
#include "core/utils/random/Random.h"
#include "core/utils/string/String_Utils.h"

// Namespaces
// ----------
// Namespace for cubemap face handling (unnamed to limit scope to this file)
namespace
{
	// the expected name tokens (case-insensitive) for each cubemap face are:
	// right, left, top, bottom, front, back; or posx, negx, posy, negy, posz, negz;
	// or short forms: px, nx, py, ny, pz, nz; up/down are also accepted for top/bottom
	enum class cubemap_face_index
	{
		RIGHT  = 0,
		LEFT   = 1,
		TOP    = 2,
		BOTTOM = 3,
		FRONT  = 4,
		BACK   = 5,
		COUNT  = 6
	};

	// assigns the given path to the specified cubemap face index if it has not been assigned yet
	// (returns true if the assignment was successful, false otherwise)
	bool assign_cubemap_face(
		std::array<std::string, static_cast<std::size_t>(cubemap_face_index::COUNT)>& ordered_paths,
		std::array<bool, static_cast<std::size_t>(cubemap_face_index::COUNT)>&        path_assigned,
		cubemap_face_index                                                            idx,
		const std::string&                                                            path)
	{
		const std::size_t i = static_cast<std::size_t>(idx);
		if (!path_assigned[i])
		{
			ordered_paths[i] = path;
			path_assigned[i] = true;
			return true;
		}
		return false;
	}
}

// Static Attributes
// -----------------
bool GUI::proportional_scaling{ true }; // propertional scaling flag is true by default

// Constructors
// ------------
GUI::GUI() :
	randomizer{ std::make_unique<Random>() }, // create a random number generator
	new_albedo{ 0.8f, 0.8f, 0.8f, 1.0f },     // default albedo color for new objects is light gray
	new_position{ 0.0f },                     // default position for new objects is the origin
	new_rotation{ 0.0f },                     // default rotation for new objects is no rotation (identity quaternion)
	new_direction{ 0.0f, 0.0f, -1.0f },       // default direction for new objects is negative z-axis
	new_scale{ 1.0f },                        // default scale for new objects is 1.0
	new_inner_cutoff{ 12.5f },                // default inner cutoff angle for new spotlights
	new_outer_cutoff{ 32.5f },                // default outer cutoff angle for new spotlights
	medium_font{ nullptr },                   // medium font for the GUI (default font) is nullptr initially
	bold_font{ nullptr }                      // bold font for the GUI is nullptr initially
{
	init_gui_layout_attributes(); // initialize the display size-independent layout attributes
	// initialize the positions and sizes of the GUI windows to default values
	// (since they require the ImGui context to be created first to access the ImGui IO object)
	// (each attribute is assigned separately, since chaining them with commas would only assign the last one)
	scene_graph_window_position = properties_window_position = ImVec2{ 0.0f, 0.0f };
	creation_window_position = debug_window_position = ImVec2{ 0.0f, 0.0f };
	scene_graph_window_size = properties_window_size = ImVec2{ 0.0f, 0.0f };
	creation_window_size = debug_window_size = ImVec2{ 0.0f, 0.0f };
	// initialize the flags for the windows to prevent focus on the first frame
	scene_graph_window_just_appeared = true;
	properties_window_just_appeared  = true;
	creation_window_just_appeared    = true;
	debug_window_just_appeared       = true;
}

// Destructor
// ----------
GUI::~GUI() {}

// Public Methods
// --------------
void GUI::configure()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // enable gamepad controls

	// button behavior configuration
	// faster button repeat when held down
	io.KeyRepeatDelay = 0.25f; // delay before repeating starts (in seconds)
	io.KeyRepeatRate  = 0.05f; // rate at which the button is repeated (seconds between repeats)

	configure_gui_style(); // configure the ImGui style (fonts, colors, etc.)
}

void GUI::draw()
{
	configure_gui_layout(); // configure the layout of the GUI windows based on the display size
	draw_gui_windows();     // draw the GUI windows (information, settings, and create Windows)
	handle_gui_input();     // handle ImGui input (mouse and keyboard)
}

// Private methods
// ---------------
void GUI::init_gui_layout_attributes()
{
	// set the private variables for the GUI layout
	scene_graph_window_relative_width  = 0.25f;
	scene_graph_window_relative_height = 1.0f;
	properties_window_relative_width   = 0.25f;
	properties_window_relative_height  = 0.4f;
	creation_window_relative_width     = 0.25f;
	creation_window_relative_height    = 0.2f;
	debug_window_relative_width        = 0.25f;
	debug_window_relative_height       = 0.4f;

	scene_graph_window_x_offset = 0.0f;
	scene_graph_window_y_offset = 0.0f;
	properties_window_x_offset  = 1.0f - properties_window_relative_width;
	properties_window_y_offset  = 0.0f;
	creation_window_x_offset    = 1.0f - creation_window_relative_width;
	creation_window_y_offset    = properties_window_relative_height;
	debug_window_x_offset       = 1.0f - debug_window_relative_width;
	debug_window_y_offset       = 1.0f - debug_window_relative_height;

	window_position_padding = ImVec2{ 10.f, 10.0f };
	window_size_padding     = ImVec2{ 20.f, 20.0f };
}

void GUI::configure_gui_style()
{
	ImGuiIO&    io    = ImGui::GetIO();    // get ImGui IO object for font and style settings
	ImGuiStyle& style = ImGui::GetStyle(); // get the ImGui style object

	// add custom fonts to the ImGui context, setting one as the default font
	io.Fonts->AddFontDefault();
	ImFont* mediumFont =
		io.Fonts->AddFontFromFileTTF(Core::get_instance()->get_resource_path("fonts/RobotoMono-Medium.ttf").c_str(), 16.0f);
	ImFont* boldFont = io.Fonts->AddFontFromFileTTF(Core::get_instance()->get_resource_path("fonts/RobotoMono-Bold.ttf").c_str(), 16.0f);
	io.FontDefault   = mediumFont; // set the medium font as the default font
	// set the member fonts for the GUI class
	medium_font = mediumFont;
	bold_font   = boldFont;

	// soften the edges of the ImGui windows and frames
	style.WindowRounding    = 4.0f;
	style.FrameRounding     = 3.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding      = 3.0f;

	// set the padding for the ImGui style
	style.WindowPadding    = ImVec2(12.0f, 12.0f); // padding between the window border and its content
	style.FramePadding     = ImVec2(6.0f, 4.0f);   // padding between the frame border and its content
	style.WindowBorderSize = 1.0f;                 // thickness of the window border

	// set the ImGui color scheme to dark mode by default
	ImGui::StyleColorsDark();
	// change the color for all ImGui elements with an accent color to a purple hue,
	// leaving the rest of the colors unchanged
	ImVec4* colors                = style.Colors;
	colors[ImGuiCol_Text]         = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.55f, 1.00f);
	colors[ImGuiCol_WindowBg]     = ImVec4(0.88f, 0.89f, 0.90f, 0.98f);
	colors[ImGuiCol_ChildBg]      = ImVec4(0.84f, 0.86f, 0.88f, 1.00f);
	colors[ImGuiCol_PopupBg]      = ImVec4(0.90f, 0.91f, 0.92f, 0.98f);
	colors[ImGuiCol_Border]       = ImVec4(0.65f, 0.68f, 0.72f, 0.65f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg]        = ImVec4(0.80f, 0.82f, 0.85f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.80f, 0.65f, 0.60f);
	colors[ImGuiCol_FrameBgActive]  = ImVec4(0.95f, 0.75f, 0.55f, 0.80f);

	colors[ImGuiCol_TitleBg]          = ImVec4(0.78f, 0.80f, 0.83f, 1.00f);
	colors[ImGuiCol_TitleBgActive]    = ImVec4(0.95f, 0.60f, 0.25f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.84f, 0.87f, 0.75f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.82f, 0.84f, 0.87f, 1.00f);

	colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.82f, 0.84f, 0.87f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.90f, 0.58f, 0.30f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.95f, 0.65f, 0.35f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.98f, 0.55f, 0.20f, 1.00f);

	colors[ImGuiCol_CheckMark]        = ImVec4(0.95f, 0.55f, 0.20f, 1.00f);
	colors[ImGuiCol_SliderGrab]       = ImVec4(0.92f, 0.55f, 0.22f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.50f, 0.15f, 1.00f);

	colors[ImGuiCol_Button]        = ImVec4(0.95f, 0.58f, 0.25f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.68f, 0.38f, 1.00f);
	colors[ImGuiCol_ButtonActive]  = ImVec4(0.90f, 0.48f, 0.18f, 1.00f);

	colors[ImGuiCol_Header]        = ImVec4(0.95f, 0.78f, 0.60f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.95f, 0.70f, 0.48f, 0.80f);
	colors[ImGuiCol_HeaderActive]  = ImVec4(0.95f, 0.62f, 0.38f, 1.00f);

	colors[ImGuiCol_Separator]        = ImVec4(0.70f, 0.72f, 0.75f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.95f, 0.60f, 0.30f, 0.78f);
	colors[ImGuiCol_SeparatorActive]  = ImVec4(0.95f, 0.55f, 0.20f, 1.00f);

	colors[ImGuiCol_ResizeGrip]        = ImVec4(0.92f, 0.58f, 0.28f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.95f, 0.62f, 0.32f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.98f, 0.55f, 0.22f, 0.95f);

	colors[ImGuiCol_Tab]                 = ImVec4(0.78f, 0.80f, 0.83f, 1.00f);
	colors[ImGuiCol_TabHovered]          = ImVec4(0.95f, 0.70f, 0.45f, 1.00f);
	colors[ImGuiCol_TabSelected]         = ImVec4(0.95f, 0.58f, 0.25f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.98f, 0.50f, 0.15f, 1.00f);
	colors[ImGuiCol_TabDimmed]           = ImVec4(0.75f, 0.77f, 0.80f, 1.00f);
	colors[ImGuiCol_TabDimmedSelected]   = ImVec4(0.88f, 0.65f, 0.42f, 1.00f);

	colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.95f, 0.70f, 0.45f, 0.35f);
	colors[ImGuiCol_DragDropTarget]        = ImVec4(0.98f, 0.60f, 0.20f, 0.90f);
	colors[ImGuiCol_NavCursor]             = ImVec4(0.95f, 0.55f, 0.20f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.22f, 0.55f);
}

void GUI::configure_gui_layout()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings

	// position of the Scene Graph Window - top left corner with padding
	scene_graph_window_position = ImVec2{
		io.DisplaySize.x * scene_graph_window_x_offset + window_position_padding.x, // x position
		io.DisplaySize.y * scene_graph_window_y_offset + window_position_padding.y  // y position
	};
	// size (width and height) of the Scene Graph Window
	scene_graph_window_size = ImVec2{
		io.DisplaySize.x * scene_graph_window_relative_width - window_size_padding.x, // width
		io.DisplaySize.y * scene_graph_window_relative_height - window_size_padding.y // height
	};
	// position of the Properties Window - top right corner with padding
	properties_window_position = ImVec2{
		io.DisplaySize.x * properties_window_x_offset + window_position_padding.x, // x position
		io.DisplaySize.y * properties_window_y_offset + window_position_padding.y  // y position
	};
	// size (width and height) of the Properties Window
	properties_window_size = ImVec2{
		io.DisplaySize.x * properties_window_relative_width - window_size_padding.x,        // width
		io.DisplaySize.y * properties_window_relative_height - window_size_padding.y * 0.5f // height
	};
	// position of the Creation Window - right side, below Properties with padding
	creation_window_position = ImVec2{
		io.DisplaySize.x * creation_window_x_offset + window_position_padding.x, // x position
		io.DisplaySize.y * creation_window_y_offset + window_position_padding.y  // y position
	};
	// size (width and height) of the Creation Window
	// (account for the padding between Creation and Debug Windows)
	creation_window_size = ImVec2{
		io.DisplaySize.x * creation_window_relative_width - window_size_padding.x,     // width
		io.DisplaySize.y * creation_window_relative_height - window_position_padding.y // height
	};
	// position of the Debug Window - bottom right corner with padding
	// (no vertical padding at top since Creation window handles the gap)
	debug_window_position = ImVec2{
		io.DisplaySize.x * debug_window_x_offset + window_position_padding.x, // x position
		io.DisplaySize.y * debug_window_y_offset + window_position_padding.y  // y position
	};
	// size (width and height) of the Debug Window
	debug_window_size = ImVec2{
		io.DisplaySize.x * debug_window_relative_width - window_size_padding.x, // width
		io.DisplaySize.y * debug_window_relative_height - window_size_padding.y // height
	};
}

void GUI::draw_gui_windows()
{
	draw_scene_graph_window();
	draw_properties_window();
	draw_creation_window();
	draw_debug_window();
}

void GUI::handle_gui_input() const
{
	// get the input manager from the Core instance
	auto& inputManager = Core::get_instance()->get_input_manager();

	// check if ImGui wants to capture the mouse (when interacting with the GUI)
	if (ImGui::GetIO().WantCaptureMouse) // prevent camera manipulation
		inputManager->set_camera_control_enabled(false);
	else // re-enable camera manipulation
		inputManager->set_camera_control_enabled(true);
}

void GUI::reset_gui_layout()
{
	// recalculate layout for current window size
	configure_gui_layout();

	// reset all window states (positions, sizes, collapsed states)
	ImGui::SetWindowPos("SCENE GRAPH", scene_graph_window_position);
	ImGui::SetWindowSize("SCENE GRAPH", scene_graph_window_size);
	ImGui::SetWindowCollapsed("SCENE GRAPH", false);

	ImGui::SetWindowPos("DEBUG", debug_window_position);
	ImGui::SetWindowSize("DEBUG", debug_window_size);
	ImGui::SetWindowCollapsed("DEBUG", false);

	ImGui::SetWindowPos("PROPERTIES", properties_window_position);
	ImGui::SetWindowSize("PROPERTIES", properties_window_size);
	ImGui::SetWindowCollapsed("PROPERTIES", false);

	ImGui::SetWindowPos("CREATION", creation_window_position);
	ImGui::SetWindowSize("CREATION", creation_window_size);
	ImGui::SetWindowCollapsed("CREATION", false);

	// log info message
	std::cout << "[INFO::GUI::reset_gui_layout] GUI layout reset to default" << std::endl;
}


void GUI::draw_scene_graph_window()
{
	// set initial size and position for the Scene Graph Window
	ImGui::SetNextWindowSize(scene_graph_window_size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(scene_graph_window_position, ImGuiCond_Appearing);
	// set the Scene Graph Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that displays the scene graph as a hierarchical tree
		// begin the Scene Graph window
		ImGui::PushFont(bold_font);
		ImGui::Begin("SCENE GRAPH", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// display instruction text
		ImGui::TextWrapped(
			"Left click: Select node\n"
			"Left click empty space: Clear selection\n"
			"Drag models to group/ungroup\n"
			"Drop on empty space to make a root node\n"
			"Del/Supr: Delete selected node");
		ImGui::Separator();

		// create a child window to hold the tree. this provides a consistent background
		// for the drop target and allows for independent scrolling.
		ImGui::BeginChild("SceneGraphTree", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove);

		// get the node manager from the Core instance
		auto& node_manager = Core::get_instance()->get_node_manager();

		// get all root nodes (nodes without parents)
		std::vector<std::shared_ptr<Node>> root_nodes;
		// collect cameras
		for (const auto& node : node_manager->get_nodes("CAMERA"))
			if (!node->get_parent())
				root_nodes.push_back(node);
		// collect lights (excluding gizmos which have parents)
		for (const auto& node : node_manager->get_nodes("LIGHT"))
			if (!node->get_parent())
				root_nodes.push_back(node);
		// collect models (excluding children which have parents)
		for (const auto& node : node_manager->get_nodes("MODEL"))
			if (!node->get_parent())
				root_nodes.push_back(node);

		// update the open-set every frame so viewport selection drives expansion
		update_scene_graph_auto_open_set();

		// draw the tree recursively starting from root nodes
		for (const auto& root_node : root_nodes)
			if (root_node) // ensure the node is valid
				draw_tree_node_recursive(root_node);

		// create an invisible "drop zone" that fills the remaining space in the child window
		// (this acts as a visual drop target - with a border when hovered - for making nodes root nodes)
		ImVec2 available_space = ImGui::GetContentRegionAvail();
		// ensure minimum height so there's always a droppable area
		float drop_zone_height = std::max(available_space.y, 40.0f);
		// use an InvisibleButton to create an interactable area that fills the remaining space
		ImGui::InvisibleButton("##RootDropZone", ImVec2(available_space.x, drop_zone_height));

		// make this invisible button a drop target for unparenting nodes
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_NODE"))
			{
				std::uint32_t dragged_node_id = *(const std::uint32_t*)payload->Data;
				auto          dragged_node    = node_manager->get_node_by_id(dragged_node_id);

				// only process if the dragged node exists, is draggable, and has a parent
				if (dragged_node && dragged_node->get_is_draggable() && dragged_node->get_parent())
				{
					// preserve the node's world transform before un-parenting
					const glm::mat4 world_transform = dragged_node->get_world_model_matrix();

					// remove from old parent. this sets the node's parent to nullptr
					dragged_node->get_parent()->remove_child(dragged_node);

					// decompose the world matrix and set it as the new local transform
					glm::vec3 scale, translation, skew;
					glm::quat rotation;
					glm::vec4 perspective;
					glm::decompose(world_transform, scale, rotation, translation, skew, perspective);

					dragged_node->set_position(translation);
					dragged_node->set_rotation(rotation);
					dragged_node->set_scale(scale);

					std::cout << "[INFO::GUI] Node '" << dragged_node->get_name() << "' (id " << dragged_node_id << ") is now a root node"
							  << std::endl;
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
			auto& selection_manager = Core::get_instance()->get_selection_manager();

			// get the selected node id to check if a node was selected
			std::uint32_t previous_selected_id = selection_manager->get_selected_node_id();
			if (previous_selected_id != 0)
			{ // only get the name of the node, clear selection, and log if a node was actually selected
				// get the name of the node that is about to be deselected (for logging purposes)
				std::string previous_selected_name;
				auto        previous_selected_node = selection_manager->get_selected_node(node_manager.get());
				if (previous_selected_node)
					previous_selected_name = previous_selected_node->get_name();

				// clear the selection and print info message with the deselected node's name and id
				selection_manager->clear_selection();

				if (!previous_selected_name.empty())
				{
					std::cout << "[INFO::GUI::draw_scene_graph_window] Deselected node '" << previous_selected_name << "' (id "
							  << previous_selected_id << ")" << std::endl;
				}
			}
		}

		ImGui::EndChild(); // end the child window for the tree

		ImGui::End(); // end the Scene Graph window

		if (scene_graph_window_just_appeared)
		{                                             // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr);           // set focus to no window
			scene_graph_window_just_appeared = false; // no longer the first frame
		}
	}
}

void GUI::draw_properties_window()
{
	// get the node manager from the Core instance
	auto& node_manager      = Core::get_instance()->get_node_manager();
	auto& selection_manager = Core::get_instance()->get_selection_manager();

	// set initial size and position for the Properties Window
	ImGui::SetNextWindowSize(properties_window_size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(properties_window_position, ImGuiCond_Appearing);
	// set the Properties Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that allows the user to change the properties of the nodes in the scene
		// begin the Properties window
		ImGui::PushFont(bold_font);
		ImGui::Begin("PROPERTIES", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		auto selected = selection_manager->get_selected_node(node_manager.get());
		if (!selected)
		{ // if no node is selected, display a message
			ImGui::Text("No node selected.\nClick an object to inspect it.");
		}
		else
		{ // if an node is selected, display its name and id, and draw its controls
			// display the name of the selected node in bold font
			ImGui::PushFont(bold_font);
			ImGui::TextUnformatted(selected->get_name().c_str()); // unformatted, so names with '%' are displayed verbatim
			ImGui::PopFont();

			ImGui::Separator();

			// depending on the type of the selected node, draw the corresponding controls
			if (selected->get_type() == Node_Type::LIGHT)
			{                                                      // if the selected node is a light, draw the light controls
				auto light = dynamic_cast<Light*>(selected.get()); // dynamic cast to Light object
				draw_light_controls(light);                        // draw the light controls
			}
			else if (
				selected->get_type() == Node_Type::COMPOSITE_MODEL || selected->get_type() == Node_Type::COMPOSITE_ASSIMP_MODEL ||
				selected->get_type() == Node_Type::ASSIMP_MODEL || selected->get_type() == Node_Type::COMPOSITE_SHAPE_MODEL ||
				selected->get_type() == Node_Type::SHAPE_MODEL)   // any model type
			{                                                     // if the selected node is a model, draw the model controls
				auto model = dynamic_cast<Node*>(selected.get()); // dynamic cast to Model object
				// only draw the model controls if the model is not a gizmo, since that is handled
				// with its corresponding light controls above
				if (model->get_gizmo_type() == Gizmo_Type::NONE)
					draw_model_controls(model); // draw the model controls
			}

			ImGui::Separator();

			if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
			{ // if the Delete button is clicked, delete the selected node via the selection manager
				selection_manager->delete_selected(node_manager.get());
			}
		}

		ImGui::End(); // end the Properties window

		if (properties_window_just_appeared)
		{                                            // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr);          // set focus to no window
			properties_window_just_appeared = false; // no longer the first frame
		}
	}
}

void GUI::draw_creation_window()
{
	// set initial size and position for the create Window
	ImGui::SetNextWindowSize(creation_window_size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(creation_window_position, ImGuiCond_Appearing);
	// set the create Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains buttons to add new objects to the scene
		// begin the Creation window
		ImGui::PushFont(bold_font);
		ImGui::Begin("CREATION", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// button to add a new directional light to the scene
		if (ImGui::Button("Create Directional Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Directional Light");
			// set random initial values
			new_albedo    = randomizer->generate_random_color();
			new_position  = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			new_direction = randomizer->generate_random_direction();
		}
		draw_create_directional_light_popup(); // draw the popup to create a new directional light

		// button to add a new point light to the scene
		if (ImGui::Button("Create Point Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Point Light");
			// set random initial values
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}
		draw_create_point_light_popup(); // draw the popup to create a new point light

		// button to add a new spotlight to the scene
		if (ImGui::Button("Create Spotlight", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Spotlight");
			// set random initial values
			new_albedo    = randomizer->generate_random_color();
			new_position  = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			new_direction = randomizer->generate_random_direction();
		}
		draw_create_spotlight_popup(); // draw the popup to create a new spotlight

		// button to add a new plane shape to the scene
		if (ImGui::Button("Create Plane Shape", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Plane Shape");
			// randomize the albedo and position of the new plane shape
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			// initialize the rotation and scale of the new plane shape with default values
			new_rotation = glm::vec3{ 0.0f };
			new_scale    = glm::vec3{ 1.0f };
		}
		draw_create_plane_shape_popup(); // draw the popup to create a new plane shape

		// buttom to add a new cube shape to the scene
		if (ImGui::Button("Create Cube Shape", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Cube Shape");
			// randomize the albedo and position of the new cube shape
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			// initialize the rotation and scale of the new cube shape with default values
			new_rotation = glm::vec3{ 0.0f };
			new_scale    = glm::vec3{ 1.0f };
		}
		draw_create_cube_shape_popup(); // draw the popup to create a new cube shape

		// button to import a new model from a file
		if (ImGui::Button("Import Model", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = Core::get_instance()->get_resource_path("models"); // initial directory to open the file dialog
			fileDialogConfig.countSelectionMax = 1;                                    // for now, allow only one file to be selected
			fileDialogConfig.flags             = ImGuiFileDialogFlags_Modal;           // no special flags for the file dialog

			// open a file dialog to select a model file
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseFileDlgKey",                                              // unique key for the file dialog
				"Choose 3D Model File",                                          // title of the file dialog
				".obj, .fbx, .dae, .gltf, .glb, .stl, .ply, .3ds, .max, .blend", // supported file extensions
				fileDialogConfig                                                 // file dialog configuration
			);
		}
		draw_import_model_popup(); // draw the popup for importing a new model

		// button to import a new skybox from 6 image files
		if (ImGui::Button("Import Skybox", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = Core::get_instance()->get_resource_path("textures/skyboxes"); // initial directory for the file dialog
			fileDialogConfig.countSelectionMax = 6;                                               // allow up to 6 files to be selected
			fileDialogConfig.flags             = ImGuiFileDialogFlags_Modal;                      // no special flags for the file dialog

			// open a file dialog to select skybox image files
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseSkyboxDlgKey",        // unique key for the file dialog
				"Choose Skybox Image Files", // title of the file dialog
				".jpg,.jpeg,.png,.hdr",      // supported file extensions
				fileDialogConfig             // file dialog configuration
			);
		}
		draw_import_skybox_popup(); // draw the popup for importing a new skybox textures

		ImGui::End(); // end the create Objects window

		if (creation_window_just_appeared)
		{                                          // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr);        // set focus to no window
			creation_window_just_appeared = false; // no longer the first frame
		}
	}
}

void GUI::draw_debug_window()
{
	// set initial size and position for the Debug Window
	ImGui::SetNextWindowSize(debug_window_size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(debug_window_position, ImGuiCond_Appearing);
	// set the Debug Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains debug information and controls
		// begin the Debug window
		ImGui::PushFont(bold_font);
		ImGui::Begin("DEBUG", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// get the renderer screen debug params from the Core instance
		auto  renderer = Core::get_instance()->get_renderer();
		auto& params   = renderer->get_screen_debug_params();
		// vector of strings that represent the different debug modes
		std::vector<std::string> debug_modes    = { "normal", "Inverted Colors", "Picking Colors", "Solid Color", "Grid Overlay" };
		int                      debug_mode_idx = static_cast<int>(params.debug_mode); // current debug mode index
		bool                     params_changed{ false }; // dirty flag to check if any of the params were changed

		// local helper to draw the outline color palette (used for outline-capable modes: 0 and 1)
		auto draw_outline_color_palette = []() {
			ImGui::Text("\tConfiguration");
			// access Selection Manager outline params
			auto& selection_manager = Core::get_instance()->get_selection_manager();
			auto& outline_params    = selection_manager->get_outline_params(); // copy current outline params

			// palette of vibrant outline colors (first item is the default color)
			static const std::vector<std::pair<const char*, glm::vec3>> outline_color_palette = {
				{ "Cyan", glm::vec3{ 0.00f, 0.95f, 1.00f } },    { "Lime", glm::vec3{ 0.30f, 1.00f, 0.30f } },
				{ "Magenta", glm::vec3{ 1.00f, 0.20f, 0.90f } }, { "Yellow", glm::vec3{ 1.00f, 0.95f, 0.20f } },
				{ "Orange", glm::vec3{ 1.00f, 0.60f, 0.20f } },  { "Red", glm::vec3{ 1.00f, 0.20f, 0.20f } },
				{ "Blue", glm::vec3{ 0.20f, 0.50f, 1.00f } },    { "Purple", glm::vec3{ 0.75f, 0.40f, 1.00f } },
				{ "White", glm::vec3{ 1.00f, 1.00f, 1.00f } }
			};

			// find nearest palette entry to current color (keeps UI in sync if color changed elsewhere)
			auto  current_outline_color = outline_params.color;
			int   current_outline_color_idx{};
			float best = std::numeric_limits<float>::max();
			for (int i{}; i < static_cast<int>(outline_color_palette.size()); ++i)
			{
				glm::vec3 d     = current_outline_color - outline_color_palette[i].second;
				float     dist2 = glm::dot(d, d);
				if (dist2 < best)
				{
					best                      = dist2;
					current_outline_color_idx = i;
				}
			}

			ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
			ImGui::SameLine();
			ImGui::Text("Outline Color");
			ImGui::SameLine(); // keep the combo box on the same line as the label
			// make the combo box take the full width of the window
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::BeginCombo(
					"##OutlineColorComboBox",
					outline_color_palette[current_outline_color_idx].first,
					ImGuiComboFlags_HeightSmall))
			{ // if the combo box is opened, iterate through all colors in the palette and display them
				for (int n{}; n < static_cast<int>(outline_color_palette.size()); ++n)
				{
					// check if the current color is selected
					bool is_selected = (current_outline_color_idx == n);
					if (ImGui::Selectable(outline_color_palette[n].first, is_selected))
					{ // if a color is selected, update the outline params in the Selection_Manager
						Outline_Params updated = outline_params;
						updated.color          = outline_color_palette[n].second;
						selection_manager->set_outline_params(updated);
					}
					// set the selected color as the default focus, i.e. highlight it
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo(); // end the combo box
			}
		};

		// scene settings section title
		ImGui::PushFont(bold_font);
		ImGui::Text("SCENE SETTINGS");
		ImGui::PopFont();

		auto& scene_manager = Core::get_instance()->get_scene_manager();

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing before button section

		// calculate button width based on available space and spacing between buttons (3 buttons total)
		float button_width = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0f;

		// button to reset the camera position and orientation
		if (ImGui::Button("Reset\nCamera", ImVec2{ button_width, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, set the camera to its default position and orientation
			scene_manager->reset_camera();
		}
		ImGui::SameLine(); // keep the buttons on the same line
		// button to clear the skybox
		if (ImGui::Button("Clear\nSkybox", ImVec2{ button_width, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, clear the skybox, i.e. remove the current skybox texture
			scene_manager->clear_skybox();
		}
		ImGui::SameLine(); // keep the buttons on the same line
		// button to reset GUI layout
		if (ImGui::Button("Reset GUI\nLayout", ImVec2{ button_width, BUTTON_HEIGHT * 2 }))
		{ // if the button is pressed, reset all GUI windows to default layout
			reset_gui_layout();
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing after button section

		// rendering settings section title
		ImGui::PushFont(bold_font);
		ImGui::Text("RENDERING INFORMATION");
		ImGui::PopFont();

		// display the current FPS and frame time
		ImGui::TextWrapped("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::TextWrapped("Frame Time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);

		// display the screen texture debug mode currently in use and the values of its parameters
		ImGui::TextWrapped("Screen Texture Debug Mode: %s", debug_modes[debug_mode_idx].c_str());
		switch (debug_mode_idx)
		{
			case 0:    // normal mode
				break; // no parameters to display
			case 1:    // Inverted Colors mode
				break; // no parameters to display
			case 2:    // Picking Colors (raw picking buffer visualization)
				// no parameters to display, draw a text to explain what is shown
				ImGui::TextWrapped(" Showing per-object encoded ids as colors");
				break;
			case 3: // Solid Color mode
				ImGui::Text(" Solid Color: (%.3f, %.3f, %.3f)", params.solid_color.r, params.solid_color.g, params.solid_color.b);
				break;
			case 4: // Grid Overlay mode
				ImGui::Text(" Grid Line Count: %d", params.grid_line_count);
				ImGui::Text(" Grid Line Thickness: %.2f", params.grid_line_thickness);
				ImGui::Text(
					" Grid Background Color: (%.3f, %.3f, %.3f)",
					params.grid_bg_color.r,
					params.grid_bg_color.g,
					params.grid_bg_color.b);
				ImGui::Text(
					" Grid Line Color: (%.3f, %.3f, %.3f)",
					params.grid_line_color.r,
					params.grid_line_color.g,
					params.grid_line_color.b);
				break;
			default:
				std::cerr << "[ERROR::GUI::draw_debug_window] Unknown screen texture debug mode index: " << debug_mode_idx << std::endl;
				break;
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // add spacing before next section

		// rendering settings section title
		ImGui::PushFont(bold_font);
		ImGui::Text("RENDERING SETTINGS");
		ImGui::PopFont();

		ImGui::Text("Screen Texture Debug Mode");
		ImGui::SameLine(); // keep the combo box on the same line as the label
		// make the combo box take the full width of the window
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		// draw a combo box to select the screen texture debug mode (with the current mode as preview)
		if (ImGui::BeginCombo("##ScreenTextureDebugMode", debug_modes[debug_mode_idx].c_str(), ImGuiComboFlags_HeightSmall))
		{ // if the combo box is opened
			for (int n{}; n < debug_modes.size(); n++)
			{                                             // iterate through all debug modes
				bool is_selected = (debug_mode_idx == n); // check if the current mode is selected
				if (ImGui::Selectable(debug_modes[n].c_str(), is_selected))
				{ // if a mode is selected, update the current debug mode locally and in the renderer
					debug_mode_idx    = n;
					params.debug_mode = debug_mode_idx;
					params_changed    = true; // mark the params as changed
				}
				if (is_selected) // whatever mode is selected, set it as the default focus
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo(); // end the combo box
		}

		// depending on the selected debug mode, draw additional controls
		switch (debug_mode_idx)
		{
			case 0: // normal mode
			{       // draw the outline color palette in a combo box to allow for outline color selection
				draw_outline_color_palette();
			}
			break;
			case 1: // Inverted Colors mode
			{       // draw the outline color palette in a combo box to allow for outline color selection
				draw_outline_color_palette();
			}
			break;
			case 2:    // Picking Colors mode (raw picking buffer visualization)
				break; // no additional controls needed
			case 3:    // Solid Color mode
			{          // draw a color picker to select the solid color
				ImGui::Text(" Configuration");
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Solid Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// use the custom drawColorControl helper to draw the color picker
				if (draw_color_control("##SolidColor", params.solid_color, false))
					params_changed = true; // mark the params as changed
			}
			break;
			case 4: // Grid Overlay mode
			{       // draw controls for each parameter
				ImGui::Text(" Configuration");
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Count      ");
				ImGui::SameLine();               // keep the input field on the same line as the label
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputInt(
						"##GridLineCount",
						(int*)&params.grid_line_count,
						1,
						10)) // set step values for the input field (normal and fast)
				{            // if the input field is changed, update the number of grid lines
					// clamp the value to a reasonable range [2, 1000]
					if (params.grid_line_count < 2)
						params.grid_line_count = 2;
					else if (params.grid_line_count > 1000)
						params.grid_line_count = 1000;

					// update the number of grid lines in the params
					params.grid_line_count = params.grid_line_count;
					params_changed         = true; // mark the params as changed
				}
				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Thickness  ");
				ImGui::SameLine();               // keep the input field on the same line as the label
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputFloat(
						"##GridLineThickness",
						&params.grid_line_thickness,
						0.05f,
						0.5f, // set step values for the input field (normal and fast)
						"%.2f"))
				{ // if the input field is changed, update the grid line thickness
					// clamp the value to a reasonable range [1.0, 10.0]
					params.grid_line_thickness = std::clamp(params.grid_line_thickness, 1.0f, 10.0f);
					params_changed             = true; // mark the params as changed
				}

				ImGui::Text("  "); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Background Color");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				// for the background color and the line color,
				// disable the alpha channel and the inputs (only show an RGB color picker)
				if (ImGui::ColorEdit3(
						"##GridBackgroundColor",
						(float*)&params.grid_bg_color,
						ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					params_changed = true; // mark the params as changed
				ImGui::Text("  ");         // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::Text("Grid Line Color      ");
				ImGui::SameLine(); // keep the color picker on the same line as the label
				if (ImGui::ColorEdit3(
						"##GridLineColor",
						(float*)&params.grid_line_color,
						ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					params_changed = true; // mark the params as changed
			}
			break;
			default:
				std::cerr << "[ERROR::GUI::draw_debug_window] Unknown screen texture debug mode: " << debug_mode_idx << std::endl;
				break;
		}

		if (params_changed)
		{ // if any of the renderer debug params were changed, apply the changes to the renderer
			renderer->set_screen_debug_params(params);
		}

		ImGui::End(); // end the Debug window

		if (debug_window_just_appeared)
		{                                       // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr);     // set focus to no window
			debug_window_just_appeared = false; // no longer the first frame
		}
	}
}


void GUI::draw_tree_node_recursive(const std::shared_ptr<Node>& node)
{
	if (!node)
		return; // safety check

	// hide gizmo nodes from the tree view to prevent clutter and confusion
	if (node->get_gizmo_type() != Gizmo_Type::NONE)
		return;

	// get selection manager to check if this node is selected
	auto& selection_manager = Core::get_instance()->get_selection_manager();
	bool  is_selected       = (selection_manager->get_selected_node_id() == node->get_id());

	// create flags for the tree node based on its state and type
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	// if the node is selected, add the Selected flag
	if (is_selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	// if the node has no children, make it a leaf node
	if (!node->is_composite())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	// if the node is a light, do no show an arrow as if it was a leaf node (gizmos are hidden)
	if (node->get_type() == Node_Type::LIGHT)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	// determine icon/prefix based on node type
	std::string node_label;
	switch (node->get_type())
	{
		case Node_Type::CAMERA: node_label = "[CAMERA] " + node->get_name(); break;
		case Node_Type::LIGHT: node_label = "[LIGHT] " + node->get_name(); break;
		case Node_Type::COMPOSITE_MODEL:
		case Node_Type::COMPOSITE_ASSIMP_MODEL:
		case Node_Type::COMPOSITE_SHAPE_MODEL: node_label = "[GROUP] " + node->get_name(); break;
		case Node_Type::MODEL:
		case Node_Type::ASSIMP_MODEL:
		case Node_Type::SHAPE_MODEL: node_label = "[MODEL] " + node->get_name(); break;
		default: node_label = "[UNKNOWN] " + node->get_name(); break;
	}

	// push a unique id for this tree node
	ImGui::PushID(static_cast<int>(node->get_id()));

	// auto-expand only nodes on the selection path (root -> ... -> selected),
	// but not the selected node itself, and do so only once per selection change
	if (scene_graph_auto_open_ids.contains(node->get_id()))
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);

	// draw the tree node and get whether it is open
	bool node_open = ImGui::TreeNodeEx(node_label.c_str(), flags);

	// DRAG SOURCE: only make draggable nodes a drag source
	if (node->get_is_draggable() && ImGui::BeginDragDropSource())
	{
		// set payload to carry the node id
		std::uint32_t node_id = node->get_id();
		ImGui::SetDragDropPayload("SCENE_GRAPH_NODE", &node_id, sizeof(node_id));

		// display a preview of the node being dragged
		ImGui::Text("Moving %s", node->get_name().c_str());

		ImGui::EndDragDropSource(); // end the drag source
	}

	// DROP TARGET: only make nodes that can be parents a drop target
	if (node->get_can_be_parent() && ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_NODE"))
		{
			// get the node manager from the Core instance to resolve node ids to node pointers
			auto&         node_manager    = Core::get_instance()->get_node_manager();
			std::uint32_t dragged_node_id = *(const std::uint32_t*)payload->Data;
			auto          dragged_node    = node_manager->get_node_by_id(dragged_node_id);
			auto          target_node     = node; // the current node in the recursion is the target

			// perform validation before re-parenting
			bool is_valid_operation = true;
			if (!dragged_node || !target_node || dragged_node == target_node)
			{
				is_valid_operation = false; // cannot drop on self or if nodes are invalid
			}
			else if (!target_node->get_can_be_parent())
			{
				std::cerr << "[WARNING::GUI] Invalid drop: Target node '" << target_node->get_name() << "' cannot be a parent" << std::endl;
				is_valid_operation = false;
			}
			else
			{
				// cycle check: a node cannot be parented to its own descendant
				for (auto p = target_node->get_parent(); p; p = p->get_parent())
				{
					if (p == dragged_node)
					{ // if any ancestor of the target node is the dragged node, it's an invalid operation
						std::cerr << "[WARNING::GUI] Invalid drop: "
									 " Cannot parent a node to its own descendant"
								  << std::endl;
						is_valid_operation = false;
						break;
					}
				}
			}

			// if the operation is valid, proceed with re-parenting and transform preservation
			if (is_valid_operation)
			{
				// preserve the world transform before re-parenting
				const glm::mat4 world_transform = dragged_node->get_world_model_matrix();

				// re-parent the node
				if (auto old_parent = dragged_node->get_parent())
					old_parent->remove_child(dragged_node);
				target_node->add_child(dragged_node);

				// calculate the new local transform to maintain the original world transform
				const glm::mat4 parent_world_transform         = target_node->get_world_model_matrix();
				const glm::mat4 parent_world_transform_inverse = glm::inverse(parent_world_transform);
				const glm::mat4 new_local_transform            = parent_world_transform_inverse * world_transform;

				// decompose the new local matrix and apply it to the dragged node
				glm::vec3 scale, translation, skew;
				glm::quat rotation;
				glm::vec4 perspective;
				glm::decompose(new_local_transform, scale, rotation, translation, skew, perspective);

				dragged_node->set_position(translation);
				dragged_node->set_rotation(rotation);
				dragged_node->set_scale(scale);

				std::cout << "[INFO::GUI] Reparented node '" << dragged_node->get_name() << "' to '" << target_node->get_name() << "'"
						  << std::endl;
			}
		}

		ImGui::EndDragDropTarget(); // end the drop target
	}

	// consume the pending open request for this node if it exists after drawing the node once
	if (scene_graph_pending_open_ids.contains(node->get_id()))
		scene_graph_pending_open_ids.erase(node->get_id());

	// always scroll the selected row into view (do not gate on auto-open set)
	if (is_selected)
		ImGui::SetScrollHereY();

	// handle selection on click
	if (ImGui::IsItemClicked())
		handle_node_selection(node->get_id());

	// only pop the tree if it was pushed (i.e. node has children)
	const bool tree_pushed = (flags & ImGuiTreeNodeFlags_NoTreePushOnOpen) == 0;

	if (node_open && tree_pushed)
	{ // if the node is open and has childrenn (i.e. tree level was pushed), draw its children recursively
		for (const auto& child : node->get_children())
			if (child)                           // ensure child is valid
				draw_tree_node_recursive(child); // recursive call for child nodes

		ImGui::TreePop(); // end the tree node
	}

	ImGui::PopID(); // pop the unique id
}

void GUI::handle_node_selection(std::uint32_t node_id)
{
	// get the node manager and selection manager from the Core instance
	auto& node_manager      = Core::get_instance()->get_node_manager();
	auto& selection_manager = Core::get_instance()->get_selection_manager();

	// store the original node id for comparison and logging
	std::uint32_t original_node_id = node_id;
	// resolve gizmo to parent light if applicable
	auto node = node_manager->get_node_by_id(node_id);
	if (node && node->get_gizmo_type() != Gizmo_Type::NONE)
	{
		// this is a gizmo, resolve to parent light
		std::uint32_t gizmo_id = node_id; // store the gizmo id for logging
		auto          parent   = node->get_parent();
		if (parent && parent->get_type() == Node_Type::LIGHT)
		{ // if the parent exists and is a light, use its id instead and update the node pointer
			node_id = parent->get_id();
			node    = parent;

			std::cout << "[INFO::GUI::handle_node_selection] Resolved gizmo model '" << node->get_name() << "' (id " << gizmo_id
					  << ") to its parent light '" << node->get_name() << "' (id " << node_id << ")" << std::endl;
		}
		else
		{
			// otherwise, print a warning and return without changing selection
			std::cerr << "[WARNING::GUI::handle_node_selection] Could not resolve gizmo model '" << node->get_name() << "' (id " << gizmo_id
					  << ") to a parent light. Selection will not change" << std::endl;
			return;
		}
	}

	// if the selected node is already selected, do not proceed (no redundant selection and logging)
	if (selection_manager->get_selected_node_id() == original_node_id)
		return;

	// set the selected node in the selection manager and log the selection
	selection_manager->set_selected_node_id(node_id);

	std::cout << "[INFO::GUI::handle_node_selection] Selected node '" << (node ? node->get_name() : "Unknown") << "' (id " << node_id << ")"
			  << std::endl;
}

void GUI::update_scene_graph_auto_open_set()
{
	// get the node manager and selection manager from the Core instance
	auto& node_manager      = Core::get_instance()->get_node_manager();
	auto& selection_manager = Core::get_instance()->get_selection_manager();

	// get the currently selected node id
	const std::uint32_t selected_id = selection_manager->get_selected_node_id();

	// if selection did not change, do not touch open state (allows manual collapsing)
	if (selected_id == last_auto_open_selected_id)
		return;
	last_auto_open_selected_id = selected_id; // update last selected id

	scene_graph_auto_open_ids.clear(); // clear previous auto-open set

	if (selected_id == 0)
		return; // no selection, nothing to auto-open, return

	// get the currently selected node
	auto selected = selection_manager->get_selected_node(node_manager.get());
	if (!selected)
		return; // safety check, return if selected node is invalid

	// even though gizmo nodes are hidden from the tree view,
	// if a gizmo node is selected, do not auto-open anything
	if (selected->get_gizmo_type() != Gizmo_Type::NONE)
		return;

	// build the chain of node ids from the selected node up to the root
	// (only open the ancestor chain, not the selected node itself, which
	// reveals the selected node but does not auto-expand it)
	for (auto parent = selected->get_parent(); parent; parent = parent->get_parent())
	{
		scene_graph_auto_open_ids.insert(parent->get_id());
		// arm open requests for this selection change
		scene_graph_pending_open_ids.insert(parent->get_id());
	}
}


void GUI::draw_light_controls(Light* light)
{
	if (!light)
		return; // safety check

	if (auto gizmo = light->get_gizmo())
	{                                                         // if the light has a gizmo, create a checkbox to toggle its visibility
		bool is_gizmo_visible = gizmo->get_is_visible();      // get current visibility state
		if (ImGui::Checkbox("Show Gizmo", &is_gizmo_visible)) // if the checkbox state is changed
			gizmo->set_is_visible(is_gizmo_visible);          // update the gizmo visibility accordingly

		ImGui::Separator(); // add a separator after the checkbox
	}

	switch (light->get_light_type()) // switch based on the type of the light
	{
		case Light_Type::DIRECTIONAL_LIGHT: // if the Light is a Directional Light
		{
			// dynamically cast the light to a Directional_Light object
			auto directional_light = dynamic_cast<Directional_Light*>(light);
			// draw controls for the directional light
			draw_directional_light_controls(directional_light);
		}
		break;
		case Light_Type::POINT_LIGHT: // if the Light is a Point Light
		{
			// dynamically cast the light to a Point Light object
			auto point_light = dynamic_cast<Point_Light*>(light);
			// draw controls for the point light
			draw_point_light_controls(dynamic_cast<Point_Light*>(light));
		}
		break;
		case Light_Type::SPOTLIGHT: // if the Light is a Spotlight
		{
			// dynamically cast the light to a Spotlight object
			auto spotlight = dynamic_cast<Spotlight*>(light);
			// draw controls for the spotlight
			draw_spotlight_controls(dynamic_cast<Spotlight*>(light));
		}
		break;
		case Light_Type::UNDEFINED: // if the Light is of an undefined type
			std::cerr << "[ERROR::GUI::draw_light_controls] UNDEFINED light type for light '" << light->get_name() << "'" << std::endl;
			return;
		default: // if the Light is of an unknown type
			std::cerr << "[ERROR::GUI::draw_light_controls] Unknown light type for light '" << light->get_name() << "'" << std::endl;
			return;
	}
}

void GUI::draw_directional_light_controls(Directional_Light* directional_light)
{
	// use PushID to create a unique id for each directional light
	ImGui::PushID(directional_light->get_name().c_str());

	// get the color of the directional light
	glm::vec3 color = directional_light->get_diffuse();
	// create a color picker for the directional light's color
	if (draw_color_control("Albedo", color))
	{                                          // if the color control is used
		directional_light->set_diffuse(color); // set the new color of the directional light
	}

	// get the position of the directional light (it only serves visualization purposes)
	glm::vec3 pos = directional_light->get_position();
	// create a control for the x, y, and z components of the directional light's position
	if (draw_vec3_control(
			"position",
			pos,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE))
	{                                         // if the control is used
		directional_light->set_position(pos); // set the new position of the directional light
	}

	// retrieve the gizmo of the directional light and its rotation in Euler angles
	auto gizmo = directional_light->get_gizmo();
	if (gizmo)
	{ // only proceed if the gizmo exists
		glm::vec3 rotation_in_degrees = gizmo->get_rotation_in_euler_angles();
		// create a control for the x, y, and z components of the directional light's rotation
		if (draw_vec3_control(
				"Rotation",
				rotation_in_degrees,
				false, // this is not a scale control
				MIN_ROTATION_VALUE,
				MAX_ROTATION_VALUE,
				INPUT_FIELD_WIDTH,
				ROTATION_SPEED,
				ROTATION_RESET_VALUE))
		{ // if the control is used
			// set the new rotation of the directional light's gizmo
			gizmo->set_rotation_in_euler_angles(rotation_in_degrees);
			// update the light's direction based on the gizmo's new forward vector,
			// without causing the gizmo to be re-oriented by set_forward() again
			directional_light->set_direction_only(gizmo->get_forward());
		}
	}

	ImGui::PopID(); // use PopID to end the unique id scope for the directional light
}

void GUI::draw_point_light_controls(Point_Light* point_light)
{
	// use PushID to create a unique id for each point light
	ImGui::PushID(point_light->get_name().c_str());

	// get the color of the point light
	glm::vec3 color = point_light->get_diffuse();
	// create a color picker for the point light's color
	if (draw_color_control("Albedo", color))
	{                                    // if the color control is used
		point_light->set_diffuse(color); // set the new color of the point light
	}

	// get the position of the point light
	glm::vec3 pos = point_light->get_position();
	// create a control for the x, y, and z components of the point light's position
	if (draw_vec3_control(
			"position",
			pos,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE))
	{                                   // if the control is used
		point_light->set_position(pos); // set the new position of the point light
	}

	ImGui::PopID(); // use PopID to end the unique id scope for the point light
}

void GUI::draw_spotlight_controls(Spotlight* spotlight)
{
	// use PushID to create a unique id for each spotlight
	ImGui::PushID(spotlight->get_name().c_str());

	// get the color of the spotlight
	glm::vec3 color = spotlight->get_diffuse();
	// create a color picker for the spotlight's color
	if (draw_color_control("Albedo", color))
	{                                  // if the color control is used
		spotlight->set_diffuse(color); // set the new color of the spotlight
	}

	// get the position of the spotlight
	glm::vec3 pos = spotlight->get_position();
	// create a control for the x, y, and z components of the spotlight's position
	if (draw_vec3_control(
			"position",
			pos,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE))
	{                                 // if the control is used
		spotlight->set_position(pos); // set the new position of the spotlight
	}

	// retrieve the gizmo of the spotlight and its rotation in Euler angles
	auto gizmo = spotlight->get_gizmo();
	if (gizmo)
	{ // only proceed if the gizmo exists
		glm::vec3 rotation_in_degrees = gizmo->get_rotation_in_euler_angles();
		// create a control for the x, y, and z components of the spotlight's rotation
		if (draw_vec3_control(
				"Rotation",
				rotation_in_degrees,
				false, // this is not a scale control
				MIN_ROTATION_VALUE,
				MAX_ROTATION_VALUE,
				INPUT_FIELD_WIDTH,
				ROTATION_SPEED,
				ROTATION_RESET_VALUE))
		{ // if the control are used
		  // set the new rotation of the spotlight's gizmo
			gizmo->set_rotation_in_euler_angles(rotation_in_degrees);
			// update the light's direction based on the gizmo's new forward vector,
			// without causing the gizmo to be re-oriented by set_forward again
			spotlight->set_direction_only(gizmo->get_forward());
		}
	}

	// get the inner and outer cut-off angles of the spotlight and
	// convert them from radians (cosine) to degrees for the controls
	float inner_cutoff = glm::degrees(glm::acos(spotlight->get_inner_cutoff()));
	float outer_cutoff = glm::degrees(glm::acos(spotlight->get_outer_cutoff()));
	// create controls for the inner and outer cut-off angles of the spotlight
	ImGui::Text("Cut-off Angles (in degrees):");
	if (draw_float_control(
			"Inner",
			inner_cutoff,
			MIN_CUTOFF_VALUE,
			outer_cutoff, // inner_cutoff <= outer_cutoff
			INPUT_FIELD_WIDTH,
			CUTOFF_ANGLES_SPEED,
			INNER_CUTOFF_RESET_VALUE))
	{
		// if the control is used, convert back to radians and cosine
		// and set the new inner cut-off angle for the spotlight
		spotlight->set_inner_cutoff(glm::cos(glm::radians(inner_cutoff)));
	}
	if (draw_float_control(
			"Outer",
			outer_cutoff,
			inner_cutoff,
			MAX_CUTOFF_VALUE, // outer_cutoff >= inner_cutoff
			INPUT_FIELD_WIDTH,
			CUTOFF_ANGLES_SPEED,
			OUTER_CUTOFF_RESET_VALUE))
	{
		// if the control is used, convert back to radians and cosine
		// and set the new outer cut-off angle for the spotlight
		spotlight->set_outer_cutoff(glm::cos(glm::radians(outer_cutoff)));
	}

	ImGui::PopID(); // use PopID to end the unique id scope for the spotlight
}

void GUI::draw_model_controls(Node* model)
{
	// use PushID to create a unique id for each model
	ImGui::PushID(model->get_name().c_str());

	glm::vec4 albedo = model->get_albedo(); // get the albedo color of the model (with alpha channel)

	// for shape models, show the full RGBA color picker, including alpha channel for transparency control
	if (model->get_type() == Node_Type::SHAPE_MODEL)
	{
		// create a color picker for the model's color
		if (draw_color_control("Albedo", albedo))
		{                              // if the color control is used
			model->set_albedo(albedo); // set the new color of the model
		}
	}
	// for Assimp models, show only a slider for the alpha component to allow for
	// transparency/opacity control without affecting the original material colors
	else if (model->get_type() == Node_Type::ASSIMP_MODEL)
	{
		ImGui::Text("Opacity"); // label for the alpha slider
		ImGui::SameLine();      // keep the slider on the same line as the label
		// make the slider take the full width of the window
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		// create a slider for the alpha component of the model's color
		if (ImGui::SliderFloat("##Opacity", &albedo.a, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{                              // if the slider is used, set the new color with original RGB and new alpha
			model->set_albedo(albedo); // set the new color of the model
		}
	}

	// get the position of the model
	glm::vec3 pos = model->get_position();
	// create a control for the x, y, and z components of the model's position
	if (draw_vec3_control(
			"position",
			pos,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE))
	{                             // if the control is used
		model->set_position(pos); // set the new position of the model
	}

	// get the rotation in Euler angles
	glm::vec3 rotation_in_degrees = model->get_rotation_in_euler_angles();
	// create a control for the x, y, and z components of the model's rotation
	if (draw_vec3_control(
			"Rotation",
			rotation_in_degrees,
			false, // this is not a scale control
			MIN_ROTATION_VALUE,
			MAX_ROTATION_VALUE,
			INPUT_FIELD_WIDTH,
			ROTATION_SPEED,
			ROTATION_RESET_VALUE))
	{                                                             // if the control is used
		model->set_rotation_in_euler_angles(rotation_in_degrees); // set the new rotation of the model
	}

	// get the scale of the model
	glm::vec3 scale = model->get_scale();
	// if the model is a shape, use a faster speed for scaling, otherwise use the default speed
	float speed = model->get_type() == Node_Type::SHAPE_MODEL ? SCALE_SPEED * 5.0f : SCALE_SPEED;
	// create a control for the x, y, and z components of the model's scale
	if (draw_vec3_control(
			"Scale",
			scale,
			true, // this is the scale control
			MIN_SCALE_VALUE,
			MAX_SCALE_VALUE,
			INPUT_FIELD_WIDTH,
			speed,
			SCALE_RESET_VALUE))
	{                            // if the control is used
		model->set_scale(scale); // set the new scale of the model
	}

	ImGui::PopID(); // use PopID to end the unique id scope for the model
}


void GUI::draw_create_directional_light_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	if (ImGui::BeginPopupModal("Create Directional Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(bold_font);
		ImGui::Text("Set initial properties for the new Directional Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and direction of the new directional light
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedo_rgb = new_albedo;
		draw_color_control("Albedo", albedo_rgb);
		new_albedo = glm::vec4(albedo_rgb, 1.0f); // set alpha to 1.0f
		draw_vec3_control(
			"position",
			new_position,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE);
		draw_vec3_control(
			"Direction",
			new_direction,
			false, // this is not a scale control
			MIN_DIRECTION_VALUE,
			MAX_DIRECTION_VALUE,
			INPUT_FIELD_WIDTH,
			DIRECTION_SPEED,
			DIRECTION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new directional light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new directional light's properties
			new_albedo    = randomizer->generate_random_color();
			new_position  = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			new_direction = randomizer->generate_random_direction();
		}

		// display a button to add the new directional light
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the create button is clicked
			// create a new directional light with a placeholder name and the specified properties
			auto directional_light = std::make_shared<Directional_Light>(
				"Directional Light",
				glm::vec3{ 0.1f },
				new_albedo,
				glm::vec3{ 1.0f },
				new_position,
				new_direction);

			// convert the light's id to string and set it as part of the light's name
			directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			directional_light->create_gizmo();
			// register the gizmo child for selection/picking before moving the light
			auto directional_light_gizmo = directional_light->get_gizmo();
			if (directional_light_gizmo)
				node_manager->add_node(directional_light_gizmo);
			node_manager->add_node(std::move(directional_light));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{                               // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::draw_create_point_light_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	if (ImGui::BeginPopupModal("Create Point Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(bold_font);
		ImGui::Text("Set initial properties for the new Point Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color and position of the new point light
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedo_rgb = new_albedo;
		draw_color_control("Albedo", albedo_rgb);
		new_albedo = glm::vec4(albedo_rgb, 1.0f); // set alpha to 1.0f
		draw_vec3_control(
			"position",
			new_position,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new point light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new point light's properties
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new point light
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the create button is clicked
			// create a new point light with a placeholder name and the specified properties
			auto point_light = std::make_shared<Point_Light>("Point Light", glm::vec3{ 0.1f }, new_albedo, glm::vec3{ 1.0f }, new_position);

			// convert the light's id to string and set it as part of the light's name
			point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			point_light->create_gizmo();
			// register the gizmo child for selection/picking before moving the light
			auto point_light_gizmo = point_light->get_gizmo();
			if (point_light_gizmo)
				node_manager->add_node(point_light_gizmo);
			node_manager->add_node(std::move(point_light));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{                               // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}
		ImGui::EndPopup();
	}
}

void GUI::draw_create_spotlight_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	if (ImGui::BeginPopupModal("Create Spotlight", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(bold_font);
		ImGui::Text("Set initial properties for the new Spotlight\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, direction, and cut-off angles of the new spotlight
		// (the color picker should only be RGB, no alpha channel)
		glm::vec3 albedo_rgb = new_albedo;
		draw_color_control("Albedo", albedo_rgb);
		new_albedo = glm::vec4(albedo_rgb, 1.0f); // set alpha to 1.0f
		draw_vec3_control(
			"position",
			new_position,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE);
		draw_vec3_control(
			"Direction",
			new_direction,
			false, // this is not a scale control
			MIN_DIRECTION_VALUE,
			MAX_DIRECTION_VALUE,
			INPUT_FIELD_WIDTH,
			DIRECTION_SPEED,
			DIRECTION_RESET_VALUE);
		ImGui::Text("Cut-Off Angles (in degrees):");
		// initialize the inner and outer cut-off angles with default values before drawing the controls
		draw_float_control(
			"Inner",
			new_inner_cutoff,
			MIN_CUTOFF_VALUE,
			MAX_CUTOFF_VALUE, // inner_cutoff <= outer_cutoff
			INPUT_FIELD_WIDTH,
			CUTOFF_ANGLES_SPEED,
			INNER_CUTOFF_RESET_VALUE);
		draw_float_control(
			"Outer",
			new_outer_cutoff,
			new_inner_cutoff,
			MAX_CUTOFF_VALUE, // outer_cutoff >= inner_cutoff
			INPUT_FIELD_WIDTH,
			CUTOFF_ANGLES_SPEED,
			OUTER_CUTOFF_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new spotlight
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new spotlight's properties
			new_albedo       = randomizer->generate_random_color();
			new_position     = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			new_direction    = randomizer->generate_random_direction();
			new_inner_cutoff = randomizer->generate_random_float(MIN_CUTOFF_VALUE, MAX_CUTOFF_VALUE);
			new_outer_cutoff = randomizer->generate_random_float(
				new_inner_cutoff, // ensure outer cut-off is greater than inner cut-off
				MAX_CUTOFF_VALUE);
		}

		// display a button to add the new spotlight
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the create button is clicked
			// create a new spotlight with a placeholder name and the specified properties
			auto spotlight =
				std::make_shared<Spotlight>("Spotlight", glm::vec3{ 0.1f }, new_albedo, glm::vec3{ 1.0f }, new_position, new_direction);

			// convert the light's id to string and set it as part of the light's name
			spotlight->set_name(String_Utils::generate_id_prefixed_name(spotlight));

			// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
			spotlight->create_gizmo();
			// register the gizmo child for selection/picking before moving the light
			auto spotlight_gizmo = spotlight->get_gizmo();
			if (spotlight_gizmo)
				node_manager->add_node(spotlight_gizmo);
			node_manager->add_node(std::move(spotlight));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{                               // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::draw_create_plane_shape_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	if (ImGui::BeginPopupModal("Create Plane Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(bold_font);
		ImGui::Text("Set initial properties for the new Plane Shape\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and size of the new plane shape
		draw_color_control("Color", new_albedo);
		draw_vec3_control(
			"position",
			new_position,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE);
		draw_vec3_control(
			"Rotation",
			new_rotation,
			false, // this is not a scale control
			MIN_ROTATION_VALUE,
			MAX_ROTATION_VALUE,
			INPUT_FIELD_WIDTH,
			ROTATION_SPEED,
			ROTATION_RESET_VALUE);
		draw_vec3_control(
			"Scale",
			new_scale,
			true, // this is the scale control
			MIN_SCALE_VALUE,
			MAX_SCALE_VALUE,
			INPUT_FIELD_WIDTH,
			SCALE_SPEED * 5.0f,
			SCALE_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new plane shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new plane shape's properties
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new plane shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the create button is clicked
			// create a plane shape with a placeholder name and the specified properties
			auto plane_shape = std::make_shared<Shape_Model>(
				"Plane Shape",
				plane_vertices_vector,
				plane_indices_vector,
				new_albedo,
				new_position,
				glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f },
				new_scale);
			// indicate that the plane is a two-sided shape
			plane_shape->set_is_two_sided(true);
			// set the rotation in Euler angles of the new plane shape
			plane_shape->set_rotation_in_euler_angles(new_rotation);

			// convert the shape's id to string and set it as part of the shape's name
			plane_shape->set_name(String_Utils::generate_id_prefixed_name(plane_shape));

			// add the new plane shape to the engine
			node_manager->add_node(std::move(plane_shape));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{                               // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::draw_create_cube_shape_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	if (ImGui::BeginPopupModal("Create Cube Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(bold_font);
		ImGui::Text("Set initial properties for the new Cube Shape\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and size of the new cube shape
		draw_color_control("Color", new_albedo);
		draw_vec3_control(
			"position",
			new_position,
			false, // this is not a scale control
			MIN_POSITION_VALUE,
			MAX_POSITION_VALUE,
			INPUT_FIELD_WIDTH,
			POSITION_SPEED,
			POSITION_RESET_VALUE);
		draw_vec3_control(
			"Rotation",
			new_rotation,
			false, // this is not a scale control
			MIN_ROTATION_VALUE,
			MAX_ROTATION_VALUE,
			INPUT_FIELD_WIDTH,
			ROTATION_SPEED,
			ROTATION_RESET_VALUE);
		draw_vec3_control(
			"Scale",
			new_scale,
			true, // this is the scale control
			MIN_SCALE_VALUE,
			MAX_SCALE_VALUE,
			INPUT_FIELD_WIDTH,
			SCALE_SPEED * 5.0f,
			SCALE_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new cube shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new cube shape's properties
			new_albedo   = randomizer->generate_random_color();
			new_position = randomizer->generate_random_position(glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new cube shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the create button is clicked
			// create a cube shape with a placeholder name and the specified properties
			auto cube_shape = std::make_shared<Shape_Model>(
				"Cube Shape",
				cube_vertices_vector,
				cube_indices_vector,
				new_albedo,
				new_position,
				glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f },
				new_scale);
			// set the rotation of the new cube shape
			cube_shape->set_rotation_in_euler_angles(new_rotation);

			// convert the shape's id to string and set it as part of the shape's name
			cube_shape->set_name(String_Utils::generate_id_prefixed_name(cube_shape));

			// add the new cube shape to the engine
			node_manager->add_node(std::move(cube_shape));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{                               // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}

void GUI::draw_import_model_popup()
{
	// get the node manager from the Core instance
	auto& node_manager = Core::get_instance()->get_node_manager();

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for display size

	// set the size and position of the next window to display the file dialog adequately,
	// centered on the display and with a predefined size
	ImGui::SetNextWindowSize(ImVec2(FILE_DIALOG_POPUP_WIDTH, FILE_DIALOG_POPUP_HEIGHT), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(
		ImVec2(io.DisplaySize.x * 0.5f - FILE_DIALOG_POPUP_WIDTH * 0.5f, io.DisplaySize.y * 0.5f - FILE_DIALOG_POPUP_HEIGHT * 0.5f),
		ImGuiCond_Appearing);

	// check if the file dialog is displayed and if the user selected a file
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{ // if the file dialog is displayed
		if (ImGuiFileDialog::Instance()->IsOk())
		{ // if the user clicked the OK button (or double-clicked a file, i.e. selected a file)
			// get the selected file path and name
			std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string file_name = ImGuiFileDialog::Instance()->GetCurrentFileName();

			// normalize slashes to forward slashes for cross-platform texture loading
			std::replace(file_path.begin(), file_path.end(), '\\', '/');

			// convert the filename to a clean display name:
			// remove the file extension, replace underscores and hyphens with spaces,
			// and capitalize the first letter of each word (title case)
			std::string file_display_name = String_Utils::to_clean_display_name(file_name);

			// create a new model with the clean display name and the file path,
			// which will be loaded by Assimp_Model's constructor
			auto assimp_model = std::make_shared<Assimp_Model>(file_display_name, file_path);

			// convert the model's id to string and set it as part of the model's name
			assimp_model->set_name(String_Utils::generate_id_prefixed_name(assimp_model));

			// add the new model to the engine
			node_manager->add_node(std::move(assimp_model));
		}

		ImGuiFileDialog::Instance()->Close(); // close the file dialog
	}
}

void GUI::draw_import_skybox_popup()
{
	// get the scene manager from the Core instance
	auto& scene_manager = Core::get_instance()->get_scene_manager();

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for display size

	// static state for the error modals and reopening the file dialog, preserved across frames
	static bool        show_count_error{ false };          // true if number of selected images is != 6 or 1
	static bool        show_mapping_error{ false };        // true if the selected images cannot be mapped
	static std::string error_popup_message{};              // message to display in the error popup
	static bool        should_reopen_file_dialog{ false }; // true if the file dialog should be reopened

	// set the size and position of the next window to display the file dialog adequately
	ImGui::SetNextWindowSize(ImVec2(FILE_DIALOG_POPUP_WIDTH, FILE_DIALOG_POPUP_HEIGHT), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(
		ImVec2(io.DisplaySize.x * 0.5f - FILE_DIALOG_POPUP_WIDTH * 0.5f, io.DisplaySize.y * 0.5f - FILE_DIALOG_POPUP_HEIGHT * 0.5f),
		ImGuiCond_Appearing);

	// main loop for the file dialog display and handling
	if (ImGuiFileDialog::Instance()->Display("ChooseSkyboxDlgKey"))
	{ // if the file dialog is displayed
		if (ImGuiFileDialog::Instance()->IsOk())
		{ // if the user clicked the OK button (or double-clicked a file)
			const auto& selection = ImGuiFileDialog::Instance()->GetSelection();
			bool        validation_passed{ true }; // assume validation passed unless an error is found

			// local helper lambda to parse cubemap face names from a set of paths
			// (this avoids code duplication between the validation and import steps)
			auto parse_cubemap_faces = [](const std::vector<std::string>& paths, std::vector<std::string>& ordered_face_paths) -> bool {
				// get the number of cubemap faces
				constexpr std::size_t cubemap_face_count = static_cast<std::size_t>(cubemap_face_index::COUNT);
				// prepare arrays to hold ordered paths and track assigned faces
				std::array<std::string, cubemap_face_count> ordered_paths{};
				std::array<bool, cubemap_face_count>        path_assigned{};

				// iterate over the selected file paths to infer face ordering
				for (const auto& full_path : paths)
				{
					// extract filename from the full path and convert to lowercase
					const std::string file_name = String_Utils::to_lowercase_from_copy(full_path.substr(full_path.find_last_of("/\\") + 1));

					bool matched{ false }; // track if a match was found for this filename

					if (file_name.find("right") != std::string::npos || file_name.find("posx") != std::string::npos ||
						file_name.find("px") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::RIGHT, full_path);
					else if (
						file_name.find("left") != std::string::npos || file_name.find("negx") != std::string::npos ||
						file_name.find("nx") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::LEFT, full_path);
					else if (
						file_name.find("top") != std::string::npos || file_name.find("posy") != std::string::npos ||
						file_name.find("py") != std::string::npos || file_name.find("up") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::TOP, full_path);
					else if (
						file_name.find("bottom") != std::string::npos || file_name.find("negy") != std::string::npos ||
						file_name.find("ny") != std::string::npos || file_name.find("down") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::BOTTOM, full_path);
					else if (
						file_name.find("front") != std::string::npos || file_name.find("posz") != std::string::npos ||
						file_name.find("pz") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::FRONT, full_path);
					else if (
						file_name.find("back") != std::string::npos || file_name.find("negz") != std::string::npos ||
						file_name.find("nz") != std::string::npos)
						matched = assign_cubemap_face(ordered_paths, path_assigned, cubemap_face_index::BACK, full_path);

					if (!matched)
						return false; // if no match was found, parsing fails for this filename
				}

				// if not all faces have been assigned a path, parsing fails
				if (!std::all_of(path_assigned.begin(), path_assigned.end(), [](bool b) { return b; }))
					return false;

				// on success, populate the output vector and return true
				ordered_face_paths.assign(ordered_paths.begin(), ordered_paths.end());
				return true;
			};

			// validate the number of selected files and proceed accordingly
			if (selection.size() == 1)
			{ // if the user selected a single file (assumed to be an HDR equirectangular map)
				const std::string& single_file_name = selection.begin()->first; // get the filename
				std::string        lowercase_name   = single_file_name;
				std::transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
				});
				bool is_hdr = lowercase_name.size() >= 4 && lowercase_name.rfind(".hdr") == lowercase_name.size() - 4;

				if (!is_hdr)
				{ // if the selected file is not an .hdr file, the selection is invalid
					validation_passed   = false;
					show_count_error    = true;
					error_popup_message = "A single file must have the .hdr extension";
					std::cerr << "[ERROR::GUI::draw_import_skybox_popup] Single selected file is not .hdr: " << single_file_name
							  << std::endl;
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

				std::vector<std::string> ordered_face_paths; // will be populated by the parser
				if (!parse_cubemap_faces(paths, ordered_face_paths))
				{ // if parsing fails, the selection is invalid
					validation_passed   = false;
					show_mapping_error  = true;
					error_popup_message = "Could not infer cubemap face ordering from file names.\n"
										  "Filenames must contain tokens like: right, left, top, bottom, front, back.\n"
										  "Alternatives: posx, negx, posy, negy, posz, negz; or px, nx, py, ny, pz, nz.\n"
										  "Up/down are also accepted for top/bottom";
					std::cerr << "[ERROR::GUI::draw_import_skybox_popup] "
								 "Could not map cubemap faces from filenames, valid names must contain "
								 "tokens like:\n\tright, left, top, bottom, front, back; or posx, negx, posy, negy, "
								 "posz, negz; \n\tor px, nx, py, ny, pz, nz. "
								 "Up/down are also accepted for top/bottom"
							  << std::endl;
				}
			}
			else
			{ // if the user selected a number of files other than 1 or 6, it's an invalid selection
				validation_passed   = false;
				show_count_error    = true;
				error_popup_message = "Invalid number of files selected.\n"
									  "Please select either 1 .hdr file or 6 image files (.png, .jpg, .jpeg)";
				std::cerr << "[ERROR::GUI::draw_import_skybox_popup] Invalid number of files selected: " << selection.size() << std::endl;
			}

			// handle the successful validation case by importing the corresponding skybox texture
			if (validation_passed)
			{
				if (selection.size() == 1)
				{ // create the skybox texture from the equirectangular .hdr image
					std::string path = selection.begin()->second;
					std::replace(path.begin(), path.end(), '\\', '/');
					auto skybox_texture = std::make_shared<Texture>("Skybox HDR Equirectangular", path, true);
					scene_manager->set_skybox(std::move(skybox_texture));
					std::cout << "[SUCCESS::GUI::draw_import_skybox_popup] "
								 "Successfully imported HDR equirectangular skybox"
							  << std::endl;
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
					std::vector<std::string> ordered_face_paths;
					parse_cubemap_faces(paths, ordered_face_paths); // will succeed as it was validated
					auto skybox_texture = std::make_shared<Texture>("Skybox Cubemap", ordered_face_paths);
					scene_manager->set_skybox(std::move(skybox_texture));
					std::cout << "[SUCCESS::GUI::draw_import_skybox_popup] "
								 "Successfully imported cubemap skybox"
							  << std::endl;
				}
				should_reopen_file_dialog = false; // ensure no dialog is reopened on success
			}
			else
			{ // on failure, set flag to reopen the file dialog when the error popup is closed
				should_reopen_file_dialog = true;
			}

			// always close the file dialog after processing the selection, valid or invalid
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{ // if the user cancelled the file dialog (i.e., closed without a selection)
			// reset any error state and ensure the dialog is not reopened automatically
			should_reopen_file_dialog = false;
			show_count_error          = false;
			show_mapping_error        = false;
			error_popup_message.clear();
			ImGuiFileDialog::Instance()->Close(); // close the file dialog instance
		}
	}

	// local constant flag indicating if any error popup should be shown this frame
	const bool is_error_modal_open = show_count_error || show_mapping_error;

	// while an error is active, ensure the error popup is kept open as a modal
	if (is_error_modal_open)
		ImGui::OpenPopup("Skybox Import Error");

	// set the size and position of the error popup
	ImGui::SetNextWindowSize(ImVec2(ERROR_POPUP_WIDTH, ERROR_POPUP_HEIGHT), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(
		ImVec2(io.DisplaySize.x * 0.5f - ERROR_POPUP_WIDTH * 0.5f, io.DisplaySize.y * 0.5f - ERROR_POPUP_HEIGHT * 0.5f),
		ImGuiCond_Appearing);

	// error modal popup definition
	if (ImGui::BeginPopupModal("Skybox Import Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the error popup is open, ensure it is focused and displays the error message
		ImGui::SetWindowFocus();
		ImGui::TextWrapped("%s", error_popup_message.c_str());

		// a bit of vertical spacing before the OK button
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		// center the OK button horizontally within the popup
		ImVec2 button_size{ BUTTON_WIDTH, 0.0f };
		float  avail_width = ImGui::GetContentRegionAvail().x;
		float  offset_x    = (avail_width - button_size.x) * 0.5f;
		if (offset_x > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

		if (ImGui::Button("OK", ImVec2(BUTTON_WIDTH, 0.0f)))
		{ // if the OK button is clicked, clear error state and close the popup
			show_count_error   = false;
			show_mapping_error = false;
			error_popup_message.clear();
			ImGui::CloseCurrentPopup();

			// only reopen the file dialog if indicated (i.e., after a failed validation)
			if (should_reopen_file_dialog)
			{
				// file dialog configuration (same as used when first opened)
				IGFD::FileDialogConfig fileDialogConfig;
				fileDialogConfig.path              = Core::get_instance()->get_resource_path("textures/skyboxes");
				fileDialogConfig.countSelectionMax = 6;
				fileDialogConfig.flags             = ImGuiFileDialogFlags_Modal;

				ImGuiFileDialog::Instance()
					->OpenDialog("ChooseSkyboxDlgKey", "Choose Skybox Image Files", ".jpg,.jpeg,.png,.hdr", fileDialogConfig);
				should_reopen_file_dialog = false; // reset the flag
			}
		}
		ImGui::EndPopup(); // end the error popup definition
	}
}


bool GUI::draw_color_control(const std::string& label, glm::vec3& color, bool show_label, float color_picker_width)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io        = ImGui::GetIO();     // get ImGui IO object for font and style settings
	auto     bold_font = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique id for the label to avoid conflicts with other controls

	if (show_label) // if indicated, display the label for the control
		ImGui::Text("%s", label.c_str());

	ImGui::SetNextItemWidth(color_picker_width); // set the width of the color picker
	if (ImGui::ColorEdit3("##color", (float*)&color))
	{                         // if the color picker is used
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique id scope for the label

	return value_changed; // return whether the color has changed
}

bool GUI::draw_color_control(const std::string& label, glm::vec4& color, bool show_label, float color_picker_width)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io        = ImGui::GetIO();     // get ImGui IO object for font and style settings
	auto     bold_font = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique id for the label to avoid conflicts with other controls

	if (show_label) // if indicated, display the label for the control
		ImGui::Text("%s", label.c_str());

	ImGui::SetNextItemWidth(color_picker_width);      // set the width of the color picker
	if (ImGui::ColorEdit4("##color", (float*)&color)) // color picker for vec4 (includes alpha component)
	{                                                 // if the color picker is used
		value_changed = true;                         // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique id scope for the label

	return value_changed; // return whether the color has changed
}

bool GUI::draw_vec3_control(
	const std::string& label,
	glm::vec3&         values,
	bool               scale_controls,
	float              min_input_field_value,
	float              max_input_field_value,
	float              input_field_width,
	float              speed,
	float              reset_value,
	float              reset_button_width,
	float              reset_button_height)
{
	bool value_changed{ false }; // flag to indicate if any value has changed

	ImGui::PushID(label.c_str()); // create a unique id for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	// if the controls are for scaling, display a checkbox to enable proportional scaling
	if (scale_controls)
		ImGui::Checkbox("Proportional", &proportional_scaling);

	// store previous values so that we can check if any value has changed
	glm::vec3 previous_values = values;

	// x component
	ImGui::PushID("x");
	ImGui::Text("  X  "); // display the label for the x component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(input_field_width); // set the width of the input field for the x component
	if (ImGui::InputFloat(
			"##x",
			&values.x,
			0.0f,
			0.0f, // no step buttons
			"%.3f"))
	{                         // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upX", ImGuiDir_Up))
	{                         // if the up arrow button is pressed
		values.x += speed;    // increase the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downX", ImGuiDir_Down))
	{                         // if the down arrow button is pressed
		values.x -= speed;    // decrease the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, reset_button_height)))
	{                                // if the Reset button is pressed
		values.x      = reset_value; // reset the x component to the reset value
		value_changed = true;        // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique id scope for the x component

	// y component
	ImGui::PushID("y");
	ImGui::Text("  Y  "); // display the label for the y component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(input_field_width); // set the width of the input field for the y component
	if (ImGui::InputFloat(
			"##y",
			&values.y,
			0.0f,
			0.0f, // no step buttons
			"%.3f"))
	{                         // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upY", ImGuiDir_Up))
	{                         // if the up arrow button is pressed
		values.y += speed;    // increase the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downY", ImGuiDir_Down))
	{                         // if the down arrow button is pressed
		values.y -= speed;    // decrease the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, reset_button_height)))
	{                                // if the Reset button is pressed
		values.y      = reset_value; // reset the y component to the reset value
		value_changed = true;        // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique id scope for the y component

	// z component
	ImGui::PushID("z");
	ImGui::Text("  Z  "); // display the label for the z component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(input_field_width); // set the width of the input field for the z component
	if (ImGui::InputFloat(
			"##z",
			&values.z,
			0.0f,
			0.0f, // no step buttons
			"%.3f"))
	{                         // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upZ", ImGuiDir_Up))
	{                         // if the up arrow button is pressed
		values.z += speed;    // increase the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	if (ImGui::ArrowButton("##downZ", ImGuiDir_Down))
	{                         // if the down arrow button is pressed
		values.z -= speed;    // decrease the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, reset_button_height)))
	{                                // if the Reset button is pressed
		values.z      = reset_value; // reset the z component to the reset value
		value_changed = true;        // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique id scope for the z component

	ImGui::PopID(); // end the unique id scope for the label

	// clamp values to the specified range
	values.x = std::clamp(values.x, min_input_field_value, max_input_field_value);
	values.y = std::clamp(values.y, min_input_field_value, max_input_field_value);
	values.z = std::clamp(values.z, min_input_field_value, max_input_field_value);

	// if proportional scaling is enabled, adjust the other components accordingly
	if (scale_controls && proportional_scaling)
	{
		if (values.x != previous_values.x)
		{
			values.y = values.z = values.x;
			value_changed       = true;
		}
		else if (values.y != previous_values.y)
		{
			values.x = values.z = values.y;
			value_changed       = true;
		}
		else if (values.z != previous_values.z)
		{
			values.x = values.y = values.z;
			value_changed       = true;
		}
	}

	// update the value_changed flag if any of the components have changed
	if (values.x != previous_values.x || values.y != previous_values.y || values.z != previous_values.z)
		value_changed = true;

	return value_changed;
}

bool GUI::draw_float_control(
	const std::string& label,
	float&             value,
	float              min_input_field_value,
	float              max_input_field_value,
	float              input_field_width,
	float              speed,
	float              reset_value,
	float              reset_button_width,
	float              reset_button_height)
{
	bool value_changed{ false }; // flag to indicate if the value has changed

	ImGui::PushID(label.c_str()); // create a unique id for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	ImGui::SameLine();
	ImGui::SetNextItemWidth(input_field_width); // set the width of the input field
	if (ImGui::InputFloat(
			"##val",
			&value,
			0.0f,
			0.0f, // no step buttons
			"%.2f"))
	{ // if the input field is used
		value_changed = true;
	} // set the value_changed flag to true

	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	ImGui::SameLine();
	if (ImGui::ArrowButton("##up", ImGuiDir_Up))
	{                         // if the up arrow button is pressed
		value += speed;       // increase the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##down", ImGuiDir_Down))
	{                         // if the down arrow button is pressed
		value -= speed;       // decrease the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, reset_button_height)))
	{                                // if the Reset button is pressed
		value         = reset_value; // reset the value to the reset value
		value_changed = true;        // set the value_changed flag to true
	}

	// clamp the value to the specified range
	value = std::clamp(value, min_input_field_value, max_input_field_value);

	ImGui::PopID(); // end the unique id scope for the label

	return value_changed;
}

void GUI::draw_remove_node_button(
	Node*                  node,
	std::vector<uint32_t>& nodes_to_remove_ids,
	const std::string&     label,
	float                  button_width,
	float                  button_height)
{
	std::string button_label = "Remove " + label;
	if (ImGui::Button(button_label.c_str(), ImVec2(button_width, button_height)))
	{                                                  // if the button is clicked
		nodes_to_remove_ids.push_back(node->get_id()); // add the node id to the list of nodes to remove

		std::cout << "[INFO::GUI::draw_remove_node_button] Node '" << node->get_name() << "' (id " << node->get_id()
				  << ") marked for removal" << std::endl;
	}
}
