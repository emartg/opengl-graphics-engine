/*
 * Input_Manager.h
 * This file defines the Input_Manager class, which is responsible for handling user input:
 * - Mouse cursor position and scroll input
 * - Keyboard key input
 */

#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations

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
	const GLboolean& get_camera_control_enabled() const { return camera_control_enabled; }
	// Setters
	void set_camera_control_enabled(GLboolean enabled) { camera_control_enabled = enabled; }

	// Callback Methods
	void cursor_pos_callback(GLdouble xpos, GLdouble ypos, std::string input);
	void scroll_callback(GLdouble xoffset, GLdouble yoffset);
	void key_callback(std::string input);

private:
	GLfloat   last_mouse_x, last_mouse_y, mouse_sensitivity;
	GLboolean first_mouse, camera_control_enabled;
};
