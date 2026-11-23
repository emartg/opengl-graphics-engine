/*
 * GUI.h
 * This file defines the GUI class, which is used to create a graphical user interface
 * using the ImGui library.
 * The engine will use this class to create a window that will display information about the scene
 * and allow the user to interact with it and change certain parameters.
 * It implements the Gui_Layer interface: the platform renderer owns the ImGui context and backends,
 * and calls this class to configure the context and to draw the windows every frame.
 */

#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include "Assets_Window.h"
#include "platform/gui/Gui_Layer.h"

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Node;
class Light;
class Directional_Light;
class Point_Light;
class Spotlight;
class Random;
class Material;

// Forward declaration of enum class to avoid cyclic includes
enum class Node_Type;
enum class Light_Type;

class GUI : public Gui_Layer
{
public:
	// Constructors
	// ------------
	GUI();

	// Destructor
	// ----------
	~GUI() override;

	// Public Methods
	// --------------
	// Configures the ImGui context (input flags, fonts, and style)
	void configure() override;
	// Sets up the layout, draws the GUI windows, and handles the GUI input for the current frame
	void draw() override;

private:
	// Private Types
	// -------------
	// A window of a side bar, as configure_gui_layout sees it: whether it is shown, its share of the height of the bar, and
	// where its position and size are stored
	struct Bar_Window
	{
		bool    is_shown;
		float*  share;
		ImVec2* position;
		ImVec2* size;
	};
	// A splitter of the side bars: the inner edge of a bar (dragged sideways, it changes the width of the bar) or the gap
	// between two windows of a bar (dragged up or down, it moves height from one window to the other)
	struct Layout_Splitter
	{
		std::string label;                  // label of the invisible window that holds the splitter
		ImVec2      position, size;         // rectangle of the splitter in the display
		bool        is_bar_edge{ false };   // the inner edge of a bar (otherwise the gap between two windows)
		bool        is_left_bar{ false };   // whether it belongs to the left bar
		float*      upper_share{ nullptr }; // shares of the windows above and below a gap
		float*      lower_share{ nullptr };
		float       upper_top{ 0.0f };   // top of the window above a gap
		float       pair_height{ 0.0f }; // height of the two windows of a gap and of the gap itself
	};

	// Private Attributes
	// ------------------
	std::unique_ptr<Random> randomizer; // random generator to get random colors, positions, etc.

	// attributes for the new objects to be created
	glm::vec4 new_albedo;
	glm::vec3 new_position, new_rotation, new_direction, new_scale;
	float     new_inner_cutoff, new_outer_cutoff;

	// the ASSETS window (the folders and assets of the assets directory), and the node that was selected in the scene
	// in the last frame, to know when the user selects something of the scene instead of an asset
	Assets_Window assets_window;

	// whether each window is shown (the View menu and the close button of each window change them)
	bool          show_scene_graph_window{ true };
	bool          show_assets_window{ true };
	bool          show_properties_window{ true };
	bool          show_creation_window{ true };
	bool          show_debug_window{ true };
	std::uint32_t last_scene_selection_id{ 0 };

	// set of node ids that should be auto-opened in the Scene Graph window
	std::unordered_set<std::uint32_t> scene_graph_auto_open_ids;
	// ids to actually force-open this frame (re-armed per selection change)
	std::unordered_set<std::uint32_t> scene_graph_pending_open_ids;
	// last selected id used to refresh the auto-open ids set (allows manual collapsing)
	std::uint32_t last_auto_open_selected_id{ 0 };

	// the side bars of the layout: the left bar holds the Scene Graph and the Assets windows, and the right bar the Properties,
	// the Creation, and the Debug windows. Each bar spans the height below the menu bar, its width is a fraction of the width of
	// the display, and each window of a bar takes a share of its height (a hidden window leaves its share to the others, and a
	// bar without a window shown is not drawn). The splitters of the bars change these values (set by init_gui_layout_attributes)
	float                left_bar_relative_width{ 0.0f }, right_bar_relative_width{ 0.0f };
	std::array<float, 2> left_bar_shares{};  // Scene Graph, Assets
	std::array<float, 3> right_bar_shares{}; // Properties, Creation, Debug
	// the splitters of the current frame (set by configure_gui_layout, drawn by draw_layout_splitters)
	std::vector<Layout_Splitter> layout_splitters;
	// positions and sizes of the windows in the display (set by configure_gui_layout every frame)
	ImVec2 scene_graph_window_position, properties_window_position, creation_window_position, debug_window_position;
	ImVec2 scene_graph_window_size, properties_window_size, creation_window_size, debug_window_size;
	ImVec2 assets_window_position, assets_window_size;
	// state of the Material Editor window
	bool                                material_editor_open{ false };            // whether the window is shown
	std::shared_ptr<Material>           edited_material;                          // material shown in the window
	std::unordered_set<const Material*> unsaved_materials;                        // materials changed since they were loaded or saved
	std::array<char, 128>               material_name_buffer{};                   // text of the name field of the edited material
	std::string                         material_editor_status;                   // result of the last action (e.g., a save)
	bool                                material_editor_status_is_error{ false }; // whether the last action failed
	std::shared_ptr<Material>           texture_dialog_material;                  // material whose texture the file dialog loads
	std::string                         texture_dialog_slot;                      // texture slot whose texture the file dialog loads
	std::array<char, 128>               new_material_name_buffer{};               // name of a new material (New Material popup)
	std::string                         new_material_shader_name;                 // shader of a new material (New Material popup)
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
	static constexpr float MATERIAL_EDITOR_WIDTH{ 460.0f }, MATERIAL_EDITOR_HEIGHT{ 560.0f };
	// default values for ImGui controls
	static constexpr float MIN_POSITION_VALUE{ -100.0f }, MAX_POSITION_VALUE{ 100.0f };
	static constexpr float MIN_ROTATION_VALUE{ -360.0f }, MAX_ROTATION_VALUE{ 360.0f };
	static constexpr float MIN_DIRECTION_VALUE{ -1.0f }, MAX_DIRECTION_VALUE{ 1.0f };
	static constexpr float MIN_SCALE_VALUE{ 0.001f }, MAX_SCALE_VALUE{ 100.0f };
	static constexpr float MIN_CUTOFF_VALUE{ 0.0f }, MAX_CUTOFF_VALUE{ 45.0f };
	// speeds of the drag fields: the change for each pixel that the mouse moves (Shift makes a drag 10 times faster and Alt 100 times
	// slower)
	static constexpr float POSITION_SPEED{ 0.01f }, ROTATION_SPEED{ 0.5f }, DIRECTION_SPEED{ 0.005f }, SCALE_SPEED{ 0.002f },
		CUTOFF_ANGLES_SPEED{ 0.1f };
	static constexpr float POSITION_RESET_VALUE{ 0.0f }, ROTATION_RESET_VALUE{ 0.0f }, DIRECTION_RESET_VALUE{ 0.0f },
		SCALE_RESET_VALUE{ 1.0f }, INNER_CUTOFF_RESET_VALUE{ 12.5f }, OUTER_CUTOFF_RESET_VALUE{ 32.5f };
	// default values for distances from the origin
	static constexpr float MIN_DISTANCE_TO_ORIGIN{ 4.0f }, MAX_DISTANCE_TO_ORIGIN{ 15.0f };

	// the size of the gaps around the windows of the side bars, where the splitters are, and the limits of the splitters
	static constexpr float LAYOUT_GAP{ 10.0f };
	static constexpr float MIN_BAR_WIDTH{ 220.0f }, MAX_BAR_RELATIVE_WIDTH{ 0.4f };
	static constexpr float MIN_BAR_WINDOW_HEIGHT{ 80.0f };
	// flags of the windows of the side bars: their position and size come from the layout, so they cannot be moved, resized, or
	// collapsed by themselves
	static constexpr ImGuiWindowFlags BAR_WINDOW_FLAGS{ ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove |
														ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse };

	// Private Methods
	// ---------------
	// Sets the default layout: the widths of the side bars and the shares of their windows
	void init_gui_layout_attributes();

	// Configures the ImGui style (fonts, colors, etc.)
	void configure_gui_style();

	// Sets the positions and the sizes of the windows of the side bars, and their splitters, for the current display size, and
	// gives the Core the area between the bars as the scene viewport (where the scene is drawn and picked)
	void configure_gui_layout();
	// Lays out the windows of one side bar (from top to bottom) in the given horizontal span, and adds its splitters
	void lay_out_side_bar(std::span<const Bar_Window> windows, float left, float width, bool is_left_bar);
	// Draws the splitters of the side bars, and applies the drag of the one that the user holds
	void draw_layout_splitters();
	// Draws the GUI windows
	void draw_gui_windows();

	// Draws the menu bar at the top of the window: the View menu shows or hides each window and resets the layout
	void draw_main_menu_bar();
	// Handles input events for ImGui
	void handle_gui_input() const;
	// Shows every window again and resets the side bars to their default layout
	void reset_gui_layout();

	// Draws the Scene Graph Window with a hierarchical tree view of all nodes
	void draw_scene_graph_window();
	// Draws the Properties Window with controls for the objects in the scene
	void draw_properties_window();
	// Draws the Creation Window with buttons to add new objects to the scene
	void draw_creation_window();
	// Draws the Debug Window with debug information and scene and GUI controls
	void draw_debug_window();
	// Draws the Material Editor Window (if it is open), which creates, edits, saves, and reloads the materials of the
	// material library while the scene is rendered with them
	void draw_material_editor_window();
	// Opens the Material Editor Window with a material
	void open_material_editor(const std::shared_ptr<Material>& material);

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
	// Draws a pop-up modal window to create a new material with a shader of the shader library
	void draw_create_material_popup();
	// Draws the file dialog that loads an image file as the texture of a texture slot of a material
	void draw_load_material_texture_dialog();

	// Draws the controls of the parameters of the edited material (and of the shader parameters that it does not set)
	void draw_material_parameter_controls(Material& material);
	// Draws the controls of the textures of the edited material, by texture slot of its shader
	void draw_material_texture_controls(Material& material);
	// Draws the controls of the environment map of the edited material
	void draw_material_environment_controls(Material& material);
	// Sets the result of the last action of the Material Editor, shown at the bottom of its window
	void set_material_editor_status(const std::string& status, bool is_error);

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
	// Creates a 3-component vector control with a drag field and a reset button for each component (a drag changes the value by
	// the speed for each pixel, and a double click or Ctrl+click types a value, kept within the limits) and returns true if any of
	// the components were changed
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
	// Draws a float control with a drag field and a reset button (as draw_vec3_control) and returns true if the value was changed
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
