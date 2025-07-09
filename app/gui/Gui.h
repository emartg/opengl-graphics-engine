/*
* GUI.h
* This file defines the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a window that will display information about the scene
* and allow the user to interact with it and change certain parameters.
*/

#pragma once

#include <iostream>
#include <memory> // for smart pointers

#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "../core/Core.h"
#include "../core/utils/random/Random.h"

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
	void Shutdown() const;

private:
	// Private Attributes
	// ------------------
	ImVec4 m_clearColor; // clear color for the background
	std::unique_ptr<Random> m_randomizer; // random generator to get random colors, positions, etc.

	// Private Static Attributes
	// -------------------------
	static bool s_proportionalScaling; // flag for enabling/disabling proportional scaling

	// default values for sliders and other GUI parameters
	static constexpr float MIN_POSITION_SLIDER_VALUE{ -15.0f };
	static constexpr float MAX_POSITION_SLIDER_VALUE{ 15.0f };
	static constexpr float MIN_ROTATION_SLIDER_VALUE{ -360.0f };
	static constexpr float MAX_ROTATION_SLIDER_VALUE{ 360.0f };
	static constexpr float MIN_SCALE_SLIDER_VALUE{ 0.001f };
	static constexpr float MAX_SCALE_SLIDER_VALUE{ 10.0f };
	static constexpr float MIN_INNER_CUTOFF_SLIDER_VALUE{ 0.0f };
	static constexpr float MAX_INNER_CUTOFF_SLIDER_VALUE{ 45.0f };
	static constexpr float MIN_OUTER_CUTOFF_SLIDER_VALUE{ 0.0f };
	static constexpr float MAX_OUTER_CUTOFF_SLIDER_VALUE{ 45.0f };
	static constexpr float MIN_DISTANCE_FROM_ORIGIN{ 4.0f };
	static constexpr float MAX_DISTANCE_FROM_ORIGIN{ 15.0f };

};