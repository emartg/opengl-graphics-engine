/*
* InputManager.h
* This file defines the InputManager class, which is responsible for handling user input:
* - Mouse cursor position and scroll input
*/


#pragma once

#include <string>
#include <glad/glad.h> // holds all OpenGL type declarations

class InputManager
{
public:
	// Contructor
	// ----------
	InputManager();

	// Destructor
	// ----------
	~InputManager();

	// Public Methods
	// --------------
	// Getters
	const GLboolean& GetCameraControlEnabled() const { return m_cameraControlEnabled; }
	// Setters
	void SetCameraControlEnabled(GLboolean enabled) { m_cameraControlEnabled = enabled; }

	// Callback Methods
	void CursorPosCallback(GLdouble xpos, GLdouble ypos, std::string input);
	void ScrollCallback(GLdouble xoffset, GLdouble yoffset);
	void KeyCallback(std::string input);
	void MouseButtonCallback(GLint x, GLint y, std::string input);

private:
	GLfloat m_lastMouseX, m_lastMouseY, m_mouseSensitivity;
	GLboolean m_firstMouse;
	GLboolean m_cameraControlEnabled;

};