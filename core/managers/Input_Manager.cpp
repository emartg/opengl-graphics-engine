/*
 * Input_Manager.h
 * This file implements the Input_Manager class, which is responsible for handling user input:
 * - Mouse cursor position and scroll input
 * - Keyboard key input
 */

#include "Input_Manager.h"

#include "Node_Manager.h"
#include "Scene_Manager.h"
#include "Selection_Manager.h"
#include "../Core.h"
#include "../camera/Camera.h"
#include "../renderer/Renderer.h"

// Constructor
// -----------
Input_Manager::Input_Manager() :
    last_mouse_x{},
    last_mouse_y{},
    mouse_sensitivity{ 1.0f },
    first_mouse{ true },
    camera_control_enabled{ false }
{}

// Destructor
// ----------
Input_Manager::~Input_Manager() = default;

// Public Methods
// --------------
// Callback Methods
void Input_Manager::cursor_pos_callback(GLdouble xpos, GLdouble ypos, std::string input)
{
	if (first_mouse) // to prevent the camera from jumping to the mouse position on the first input
	{
		last_mouse_x = xpos;
		last_mouse_y = ypos;
		first_mouse  = false;
	}

	GLfloat xoffset = xpos - last_mouse_x;
	GLfloat yoffset = last_mouse_y - ypos; // reversed since y-coordinates range from bottom to top
	last_mouse_x    = xpos;
	last_mouse_y    = ypos;

	if (camera_control_enabled) // only process mouse input if camera control is enabled
	{
		if (input == "MIDDLE_HOLD") // middle mouse button hold for translation
			Core::get_instance()->get_scene_manager()->get_camera()->process_mouse_translation(xoffset, yoffset);
		else if (input == "RMB_HOLD") // right mouse button hold for rotation
			Core::get_instance()->get_scene_manager()->get_camera()->process_mouse_rotation(xoffset, yoffset);
	}
}

void Input_Manager::scroll_callback(GLdouble xoffset, GLdouble yoffset)
{
	if (camera_control_enabled) // only process scroll input if camera control is enabled
		Core::get_instance()->get_scene_manager()->get_camera()->process_mouse_scroll(yoffset, 2.5f);
}

void Input_Manager::key_callback(std::string input)
{
	auto core = Core::get_instance(); // get the Core instance
	if (!core)                        // ensure the Core instance is valid before proceeding
	{                                 // if the Core instance is null, print an error message and return
		std::cerr << "[ERROR::INPUTMANAGER::key_callback] Core instance is null" << std::endl;
		return;
	}
	auto renderer = core->get_renderer(); // get the Renderer instance
	if (!renderer)                        // ensure the Renderer instance is valid before proceeding
	{                                     // if the Renderer instance is null, print an error message and return
		std::cerr << "[ERROR::INPUTMANAGER::key_callback] Renderer instance is null" << std::endl;
		return;
	}

	// handle key inputs
	if (input == "ESC_PRESSED")
	{ // if the 'Escape' key is pressed, set the window to close
		const_cast<Renderer*>(renderer)->set_window_should_close();
		std::cout << "[INFO::INPUTMANAGER::key_callback] Escape key pressed, closing window..." << std::endl;
	}
	else if (input == "DEL_PRESSED")
	{ // if the 'Delete' key is pressed, delete the selected node (if any) via the selection manager
		auto& selection_manager = core->get_selection_manager();
		auto& node_manager      = core->get_node_manager();

		// get the currently selected node id
		std::uint32_t selected_node_id = selection_manager->get_selected_node_id();

		if (selected_node_id == 0)
		{ // if no node is selected, print a warning message and return
			std::cerr << "[WARNING::INPUTMANAGER::key_callback] Delete key pressed, but no node is selected" << std::endl;
			return;
		}

		auto node = node_manager->get_node_by_id(selected_node_id); // get the selected node
		if (!node)
		{ // if the selected node does not exist, print an error message and return
			std::cerr << "[ERROR::INPUTMANAGER::key_callback] Delete key pressed, but selected node with id " << selected_node_id
			          << " does not exist" << std::endl;
			return;
		}

		// if the selected node is a camera, print an info message and return,
		// as cameras cannot be deleted for now
		if (node->get_type() == Node_Type::CAMERA)
		{
			std::cout << "[INFO::INPUTMANAGER::key_callback] Cameras cannot be deleted for now "
			             "(id "
			          << selected_node_id << ")" << std::endl;
			return;
		}

		// if the node exists and can be deleted, print a message and delete it from the scene
		std::cout << "[INFO::INPUTMANAGER::key_callback] Delete key pressed, deleting selected node " << node->get_name() << " (id "
		          << selected_node_id << ")" << std::endl;

		selection_manager->delete_selected(node_manager.get());
	}
	else
	{ // if the input is not recognized, print an error message
		std::cerr << "[ERROR::INPUTMANAGER::key_callback] Unknown key input: " << input << std::endl;
	}
}