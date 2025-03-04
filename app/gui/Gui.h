/*
* GUI.h
* This file defines the GUI class, which is used to create a graphical user interface
* using the ImGui library. The engine will use this class to create a window that will
* display information about the scene and allow the user to interact with it and change
* certain parameters.
*/

#pragma once

#include <iostream>

#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
	// Initializes the GUI
	void Init(GLFWwindow* window, const char* glslVersion);
	// Sets up the GUI (e.g., creates windows, buttons, etc.)
	void Setup();
	// Renders the GUI
	void Render();
	// Cleans up the GUI
	void Cleanup() const;

private:
	// Private Attributes
	// ------------------
	bool m_showDemoWindow;
	bool m_showAnotherWindow;
	ImVec4 m_clearColor;

};