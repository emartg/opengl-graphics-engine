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

	// default values for controls and other GUI parameters
	static constexpr float DEFAULT_ITEM_WIDTH{ 225.0f };
	static constexpr float DEFAULT_ITEM_HEIGHT{ 20.0f };
	static constexpr float DEFAULT_INPUT_FIELD_WIDTH{ 60.0f };
	static constexpr float DEFAULT_INPUT_FIELD_HEIGHT{ 20.0f };
	static constexpr float DEFAULT_BUTTON_WIDTH{ 60.0f };
	static constexpr float DEFAULT_BUTTON_HEIGHT{ 20.0f };

	static constexpr float DEFAULT_MIN_POSITION_VALUE{ -15.0f };
	static constexpr float DEFAULT_MAX_POSITION_VALUE{ 15.0f };
	static constexpr float DEFAULT_MIN_ROTATION_VALUE{ -360.0f };
	static constexpr float DEFAULT_MAX_ROTATION_VALUE{ 360.0f };
	static constexpr float DEFAULT_MIN_SCALE_VALUE{ 0.001f };
	static constexpr float DEFAULT_MAX_SCALE_VALUE{ 10.0f };
	static constexpr float DEFAULT_MIN_INNER_CUTOFF_VALUE{ 0.0f };
	static constexpr float DEFAULT_MAX_INNER_CUTOFF_VALUE{ 45.0f };
	static constexpr float DEFAULT_MIN_OUTER_CUTOFF_VALUE{ 0.0f };
	static constexpr float DEFAULT_MAX_OUTER_CUTOFF_VALUE{ 45.0f };

	static constexpr float DEFAULT_MIN_DISTANCE_FROM_ORIGIN{ 4.0f };
	static constexpr float DEFAULT_MAX_DISTANCE_FROM_ORIGIN{ 15.0f };

	// Private Methods
	// ---------------
	// Creates a color picker with sliders for RGB components
	// and returns true if the color was changed
	bool drawColorControl(const std::string& label, glm::vec3& color,
						  float colorControlWidth = DEFAULT_ITEM_WIDTH);
	// Creates a 3-component vector control with input fields and buttons
	// and returns true if any of the components were changed
	bool drawVec3Control(const std::string& label, glm::vec3& values, bool scaleControls,
						 float minInputFieldValue, float maxInputFieldValue,
						 float inputFieldWidth = DEFAULT_INPUT_FIELD_WIDTH,
						 float speed = 0.1f,
						 float resetValue = 0.0f, float resetButtonWidth = DEFAULT_BUTTON_WIDTH);
	// Draws a float control with an input field and arrow buttons
	// and returns true if the value was changed
	bool drawFloatControl(const std::string& label, float& value,
						  float minInputFieldValue, float maxInputFieldValue,
						  float inputFieldWidth = DEFAULT_INPUT_FIELD_WIDTH,
						  float speed = 0.1f,
						  float resetValue = 0.0f, float resetButtonWidth = DEFAULT_BUTTON_WIDTH);

};