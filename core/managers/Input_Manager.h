/*
 * Input_Manager.h
 * This file defines the Input_Manager class, which is responsible for handling user input:
 * - Mouse cursor position, mouse buttons, and scroll input, which move the camera about a pivot (orbit, pan, and dolly)
 * - Keyboard key input
 */

#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "../camera/Camera_Controller.h"

class Input_Manager
{
public:
	// Contructor
	// ----------
	Input_Manager();

	// Destructor
	// ----------
	~Input_Manager();

	// Public Methods
	// --------------
	// Getters
	const GLboolean&             get_camera_control_enabled() const { return camera_control_enabled; }
	const Camera_Control_Params& get_camera_control_params() const { return camera_control_params; }
	// Setters
	// Enables or disables the mouse controls of the camera (the GUI of an application disables them while the mouse is over a
	// window); a drag that has started goes on until its button is released
	void set_camera_control_enabled(GLboolean enabled) { camera_control_enabled = enabled; }
	// Sets the speeds and the limits of the mouse controls of the camera
	void set_camera_control_params(const Camera_Control_Params& params) { camera_control_params = params; }

	// Callback Methods
	// Moves the camera while a drag is active: an orbit about the pivot (right button) or a pan (middle button)
	void cursor_pos_callback(GLdouble xpos, GLdouble ypos);
	// Dollies the camera towards or away from the pivot, while the camera control is enabled
	void scroll_callback(GLdouble xoffset, GLdouble yoffset);
	// Starts or ends a drag of the camera: "RMB_PRESSED" starts an orbit and "MIDDLE_PRESSED" a pan, while the camera control is
	// enabled (the pivot is fixed when the button is pressed); "RMB_RELEASED" and "MIDDLE_RELEASED" end them
	void mouse_button_callback(std::string input);
	void key_callback(std::string input);

private:
	// Private Attributes
	// ------------------
	// kinds of drag of the camera with the mouse
	enum class Camera_Drag
	{
		NONE,
		ORBIT,
		PAN
	};

	GLfloat               last_mouse_x, last_mouse_y;
	GLboolean             first_mouse, camera_control_enabled;
	Camera_Drag           camera_drag{ Camera_Drag::NONE };
	glm::vec3             drag_pivot{ 0.0f };    // the point that the camera moves about during the drag
	Camera_Control_Params camera_control_params; // speeds and limits of the mouse controls

	// Private Methods
	// ---------------
	// Returns the point that the camera moves about: the center of the selected node (of the gizmo of a selected light), or the
	// origin of the world when nothing is selected
	glm::vec3 find_pivot() const;
};
