/*
 * Input_Manager.cpp
 * This file implements the Input_Manager class, which is responsible for handling user input:
 * - Mouse cursor position, mouse buttons, and scroll input, which move the camera about a pivot (orbit, pan, and dolly)
 * - Keyboard key input
 */

#include "Input_Manager.h"

#include "Node_Manager.h"
#include "Scene_Manager.h"
#include "Selection_Manager.h"
#include "../Core.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../renderer/Renderer.h"

// Constructor
// -----------
Input_Manager::Input_Manager() : last_mouse_x{}, last_mouse_y{}, first_mouse{ true }, camera_control_enabled{ false } {}

// Destructor
// ----------
Input_Manager::~Input_Manager() = default;

// Public Methods
// --------------
// Callback Methods
void Input_Manager::cursor_pos_callback(GLdouble xpos, GLdouble ypos)
{
	if (first_mouse) // to prevent the camera from jumping to the mouse position on the first input
	{
		last_mouse_x = xpos;
		last_mouse_y = ypos;
		first_mouse  = false;
	}

	// movement of the mouse in pixels since the last position: to the right and down are positive
	const GLfloat dx = static_cast<GLfloat>(xpos) - last_mouse_x;
	const GLfloat dy = static_cast<GLfloat>(ypos) - last_mouse_y;
	last_mouse_x     = static_cast<GLfloat>(xpos);
	last_mouse_y     = static_cast<GLfloat>(ypos);

	// the camera moves only during a drag that started over the view (it goes on while the mouse is over a window of the GUI)
	if (camera_drag == Camera_Drag::NONE)
		return;
	const auto& camera = Core::get_instance()->get_scene_manager()->get_camera();
	if (!camera)
		return;

	if (camera_drag == Camera_Drag::ORBIT)
		Camera_Controller::orbit(*camera, drag_pivot, dx, dy, camera_control_params);
	else
		Camera_Controller::pan(*camera, drag_pivot, dx, dy, Core::get_instance()->get_scene_viewport().height);
}

void Input_Manager::scroll_callback(GLdouble xoffset, GLdouble yoffset)
{
	if (!camera_control_enabled) // only process scroll input if camera control is enabled (not over a window of the GUI)
		return;
	const auto& camera = Core::get_instance()->get_scene_manager()->get_camera();
	if (camera) // the wheel moves the camera towards or away from the pivot, and does not change the field of view
		Camera_Controller::dolly(*camera, find_pivot(), static_cast<GLfloat>(yoffset), camera_control_params);
}

void Input_Manager::mouse_button_callback(std::string input)
{
	if (input == "RMB_RELEASED" && camera_drag == Camera_Drag::ORBIT)
		camera_drag = Camera_Drag::NONE;
	else if (input == "MIDDLE_RELEASED" && camera_drag == Camera_Drag::PAN)
		camera_drag = Camera_Drag::NONE;
	else if ((input == "RMB_PRESSED" || input == "MIDDLE_PRESSED") && camera_control_enabled)
	{ // a drag starts only over the view; its pivot is the one of the moment the button is pressed
		camera_drag = (input == "RMB_PRESSED") ? Camera_Drag::ORBIT : Camera_Drag::PAN;
		drag_pivot  = find_pivot();
	}
}

void Input_Manager::key_callback(std::string input)
{
	auto core = Core::get_instance(); // get the Core instance
	if (!core)                        // ensure the Core instance is valid before proceeding
	{                                 // if the Core instance is null, print an error message and return
		std::cerr << "[ERROR::INPUT_MANAGER::key_callback] Core instance is null" << std::endl;
		return;
	}
	auto renderer = core->get_renderer(); // get the Renderer instance
	if (!renderer)                        // ensure the Renderer instance is valid before proceeding
	{                                     // if the Renderer instance is null, print an error message and return
		std::cerr << "[ERROR::INPUT_MANAGER::key_callback] Renderer instance is null" << std::endl;
		return;
	}

	// handle key inputs
	if (input == "ESC_PRESSED")
	{ // if the 'Escape' key is pressed, set the window to close
		const_cast<Renderer*>(renderer)->set_window_should_close();
		std::cout << "[INFO::INPUT_MANAGER::key_callback] Escape key pressed, closing window..." << std::endl;
	}
	else if (input == "DEL_PRESSED")
	{ // if the 'Delete' key is pressed, delete the selected node (if any) via the selection manager
		auto& selection_manager = core->get_selection_manager();
		auto& node_manager      = core->get_node_manager();

		// get the currently selected node id
		std::uint32_t selected_node_id = selection_manager->get_selected_node_id();

		if (selected_node_id == 0)
		{ // if no node is selected, print a warning message and return
			std::cerr << "[WARNING::INPUT_MANAGER::key_callback] Delete key pressed, but no node is selected" << std::endl;
			return;
		}

		auto node = node_manager->get_node_by_id(selected_node_id); // get the selected node
		if (!node)
		{ // if the selected node does not exist, print an error message and return
			std::cerr << "[ERROR::INPUT_MANAGER::key_callback] Delete key pressed, but selected node with id " << selected_node_id
					  << " does not exist" << std::endl;
			return;
		}

		// a node that cannot be selected (a camera, for example) is not deleted through the selection, even if an application
		// selected it by its id
		if (!node->get_is_selectable())
		{
			std::cout << "[INFO::INPUT_MANAGER::key_callback] Node '" << node->get_name() << "' (id " << selected_node_id
					  << ") cannot be selected, so it is not deleted" << std::endl;
			return;
		}

		// if the node exists, print a message and let the selection manager delete it (it keeps a node that cannot be deleted)
		std::cout << "[INFO::INPUT_MANAGER::key_callback] Delete key pressed for the selected node " << node->get_name() << " (id "
				  << selected_node_id << ")" << std::endl;

		selection_manager->delete_selected(node_manager.get());
	}
	// any other key has no action (and pressing it is not an error), so it is ignored
}

// Private Methods
// ---------------
glm::vec3 Input_Manager::find_pivot() const
{
	auto core = Core::get_instance();
	auto node = core->get_selection_manager()->get_selected_node(core->get_node_manager().get());
	if (!node)
		return glm::vec3{ 0.0f }; // nothing is selected: the origin of the world

	// a light is found by its gizmo, which has the geometry
	if (node->get_type() == Node_Type::LIGHT)
		if (auto gizmo = static_cast<Light*>(node.get())->get_gizmo())
			node = gizmo;

	// the center of the bounding box of the node and its descendants, or its position if it has no geometry
	const Bounding_Box box{ node->get_world_bounding_box() };
	return box.is_valid() ? box.get_center() : node->get_world_position();
}
