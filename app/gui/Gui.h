/*
 * GUI.h
 * This file defines the GUI class, which is used to create a graphical user interface
 * using the ImGui library.
 * The engine will use this class to create a window that will display information about the scene
 * and allow the user to interact with it and change certain parameters.
 */

#pragma once

#include <iostream>
#include <memory>
#include <unordered_set>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp> // for glm::decompose
#define GLFW_INCLUDE_NONE               // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Node;
class Light;
class Directional_Light;
class Point_Light;
class Spotlight;
class Random;

// Forward declaration of enum class to avoid cyclic includes
enum class Node_Type;
enum class Light_Type;

class GUI
{
public:
	// Constructors
	// ------------
	GUI();

	// Destructor
	// ----------
	~GUI();

	// Public Methods
	// --------------
	// Initializes the GUI with the given GLFW window and GLSL version
	void init_gui(GLFWwindow* window, const char* glsl_version);
	// Builds the GUI by starting a new ImGui frame and setting up the layout
	void build_gui();
	// Renders the GUI by drawing the ImGui windows and handling input events
	void render_gui();
	// Shuts down the GUI and cleans up resources
	void shutdown_gui() const;

private:
	// Private Attributes
	// ------------------
	std::unique_ptr<Random> randomizer; // random generator to get random colors, positions, etc.

	// attributes for the new objects to be created
	glm::vec4 new_albedo;
	glm::vec3 new_position, new_rotation, new_direction, new_scale;
	float     new_inner_cutoff, new_outer_cutoff;

	// set of node ids that should be auto-opened in the Scene Graph window
	std::unordered_set<std::uint32_t> scene_graph_auto_open_ids;
	// ids to actually force-open this frame (re-armed per selection change)
	std::unordered_set<std::uint32_t> scene_graph_pending_open_ids;
	// last selected id used to refresh the auto-open ids set (allows manual collapsing)
	std::uint32_t last_auto_open_selected_id{ 0 };

	// parameters for the GUI layout and windows
	// relative widths and heights of the windows relative to the display size
	float scene_graph_window_relative_width, scene_graph_window_relative_height;
	float creation_window_relative_width, creation_window_relative_height;
	float properties_window_relative_width, properties_window_relative_height;
	float debug_window_relative_width, debug_window_relative_height;
	// offsets the windows from the edges of the display
	float scene_graph_window_x_offset, scene_graph_window_y_offset;
	float properties_window_x_offset, properties_window_y_offset;
	float creation_window_x_offset, creation_window_y_offset;
	float debug_window_x_offset, debug_window_y_offset;
	// padding of the windows from the edges of the display
	ImVec2 window_position_padding, window_size_padding;
	// positions and sizes of the windows in the display
	ImVec2 scene_graph_window_position, properties_window_position, creation_window_position, debug_window_position;
	ImVec2 scene_graph_window_size, properties_window_size, creation_window_size, debug_window_size;
	// flags for the windows to prevent focus on the first frame (indicating that the window just appeared)
	bool scene_graph_window_just_appeared, properties_window_just_appeared, creation_window_just_appeared, debug_window_just_appeared;

	// style attributes for the GUI
	ImFont* medium_font; // medium font for the GUI (default font)
	ImFont* bold_font;   // bold font for the GUI

	// Private Static Attributes
	// -------------------------
	static bool proportional_scaling; // flag for enabling/disabling proportional scaling

	// default values for ImGui widgets
	static constexpr float INPUT_FIELD_WIDTH{ 70.0f }, INPUT_FIELD_HEIGHT{ 20.0f };
	static constexpr float BUTTON_WIDTH{ 55.0f }, BUTTON_HEIGHT{ 22.5f };
	static constexpr float POPUP_BUTTON_WIDTH{ 120.0f }, POPUP_BUTTON_HEIGHT{ 20.0f };
	static constexpr float POPUP_WIDTH{ 400.0f }, POPUP_HEIGHT{ 300.0f };
	static constexpr float FILE_DIALOG_POPUP_WIDTH{ 1000.0f }, FILE_DIALOG_POPUP_HEIGHT{ 600.0f };
	static constexpr float ERROR_POPUP_WIDTH{ 400.0f }, ERROR_POPUP_HEIGHT{ 150.0f };
	// default values for ImGui controls
	static constexpr float MIN_POSITION_VALUE{ -100.0f }, MAX_POSITION_VALUE{ 100.0f };
	static constexpr float MIN_ROTATION_VALUE{ -360.0f }, MAX_ROTATION_VALUE{ 360.0f };
	static constexpr float MIN_DIRECTION_VALUE{ -1.0f }, MAX_DIRECTION_VALUE{ 1.0f };
	static constexpr float MIN_SCALE_VALUE{ 0.001f }, MAX_SCALE_VALUE{ 100.0f };
	static constexpr float MIN_CUTOFF_VALUE{ 0.0f }, MAX_CUTOFF_VALUE{ 45.0f };
	static constexpr float POSITION_SPEED{ 0.15f }, ROTATION_SPEED{ 1.0f }, DIRECTION_SPEED{ 0.01f }, SCALE_SPEED{ 0.002f },
	    CUTOFF_ANGLES_SPEED{ 0.25f };
	static constexpr float POSITION_RESET_VALUE{ 0.0f }, ROTATION_RESET_VALUE{ 0.0f }, DIRECTION_RESET_VALUE{ 0.0f },
	    SCALE_RESET_VALUE{ 1.0f }, INNER_CUTOFF_RESET_VALUE{ 12.5f }, OUTER_CUTOFF_RESET_VALUE{ 32.5f };
	// default values for distances from the origin
	static constexpr float MIN_DISTANCE_TO_ORIGIN{ 4.0f }, MAX_DISTANCE_TO_ORIGIN{ 15.0f };

	// Private Methods
	// ---------------
	// Initializes the GUI layout attributes that do not depend on the display size
	void init_gui_layout_attributes();

	// Configures the ImGui style (fonts, colors, etc.)
	void configure_gui_style();

	// Starts a new ImGui frame and configures the ImGui style
	void begin_gui_frame() const;
	// Sets the GUI layout attributes based on the current display size
	void configure_gui_layout();
	// Draws the GUI windows
	void draw_gui_windows();
	// Handles input events for ImGui
	void handle_gui_input() const;
	// Resets all GUI windows to their default layout for the current display size
	void reset_gui_layout();

	// Draws the Scene Graph Window with a hierarchical tree view of all nodes
	void draw_scene_graph_window();
	// Draws the Properties Window with controls for the objects in the scene
	void draw_properties_window();
	// Draws the Creation Window with buttons to add new objects to the scene
	void draw_creation_window();
	// Draws the Debug Window with debug information and scene and GUI controls
	void draw_debug_window();

	// Recursively draws a node and its children in the scene graph tree
	void draw_tree_node_recursive(const std::shared_ptr<Node>& node);
	// Handles node selection logic (single selection only)
	void handle_node_selection(std::uint32_t node_id);
	// Updates the set of node ids that should be auto-opened in the scene graph window
	void update_scene_graph_auto_open_set();

	// Draws controls for a light. Depending on the type of light,
	// it will call dynamically cast to the appropriate light type and draw the corresponding controls
	void draw_light_controls(Light* light);
	// Draws controls for a directional light
	void draw_directional_light_controls(Directional_Light* directional_light);
	// Draws controls for a point light
	void draw_point_light_controls(Point_Light* point_light);
	// Draws controls for a spotlight
	void draw_spotlight_controls(Spotlight* spotlight);
	// Draws controls for a model
	void draw_model_controls(Node* model);

	// Draws a pop-up modal window to create a new directional light
	void draw_create_directional_light_popup();
	// Draws a pop-up modal window to create a new point light
	void draw_create_point_light_popup();
	// Draws a pop-up modal window to create a new spotlight
	void draw_create_spotlight_popup();
	// Draws a pop-up modal window to create a new plane shape
	void draw_create_plane_shape_popup();
	// Draws a pop-up modal window to create a new cube shape
	void draw_create_cube_shape_popup();
	// Draws a pop-up modal window to import a model from a file
	void draw_import_model_popup();
	// Draws a pop-up modal window to import a skybox from a folder (6 textures)
	void draw_import_skybox_popup();

	// Creates a color picker with sliders for RGB components
	// and returns true if the color was changed
	bool draw_color_control(
	    const std::string& label,
	    glm::vec3&         color,
	    bool               show_label         = true,
	    float              color_picker_width = ImGui::GetContentRegionAvail().x);
	// Creates a color picker with sliders for RGBA components
	// and returns true if the color was changed
	bool draw_color_control(
	    const std::string& label,
	    glm::vec4&         color,
	    bool               show_label         = true,
	    float              color_picker_width = ImGui::GetContentRegionAvail().x);
	// Creates a 3-component vector control with input fields and buttons
	// and returns true if any of the components were changed
	bool draw_vec3_control(
	    const std::string& label,
	    glm::vec3&         values,
	    bool               scale_controls,
	    float              min_input_field_value,
	    float              max_input_field_value,
	    float              input_field_width   = INPUT_FIELD_WIDTH,
	    float              speed               = 0.1f,
	    float              reset_value         = 0.0f,
	    float              reset_button_width  = BUTTON_WIDTH,
	    float              reset_button_height = BUTTON_HEIGHT);
	// Draws a float control with an input field and arrow buttons
	// and returns true if the value was changed
	bool draw_float_control(
	    const std::string& label,
	    float&             value,
	    float              min_input_field_value,
	    float              max_input_field_value,
	    float              input_field_width   = INPUT_FIELD_WIDTH,
	    float              speed               = 0.1f,
	    float              resetValue          = 0.0f,
	    float              reset_button_width  = BUTTON_WIDTH,
	    float              reset_button_height = BUTTON_HEIGHT);

	// Draws a remove button for an node and adds its id to the vector of nodes marked for removal
	void draw_remove_node_button(
	    Node*                       node,
	    std::vector<std::uint32_t>& nodes_to_remove_ids,
	    const std::string&          label         = "Remove",
	    float                       button_width  = BUTTON_WIDTH,
	    float                       button_height = BUTTON_HEIGHT);
};