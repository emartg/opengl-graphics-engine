/*
 * Gui_Layer.h
 * This file defines the Gui_Layer interface, which represents the application-specific part of
 * the graphical user interface (the ImGui windows, fonts, and style).
 * The platform renderer (e.g., GLFW_Renderer) owns the ImGui context and its backends, and calls
 * the GUI layer at the appropriate moments, so that each application (the sample App, a simulator, etc.)
 * only has to implement its own windows.
 */

#pragma once

class Gui_Layer
{
public:
	// Destructor
	// ----------
	virtual ~Gui_Layer() = default;

	// Public Methods
	// --------------
	// Configures the ImGui context (e.g., input flags, fonts, and style). Called once, right after
	// the ImGui context is created and before the ImGui backends are initialized
	virtual void configure() {}

	// Draws the ImGui windows of the application. Called every frame, between the start of a new
	// ImGui frame and its rendering
	virtual void draw() = 0;
};
