/*
* GLFWRenderer.h
* Defines an inferited class of the Renderer interface using GLFW,
* a library for creating windows and handling input.
*/

#pragma once

#include <iostream>

#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include "../gui/Gui.h"

#include "../core/renderer/Renderer.h"
#include "../core/managers/InputManager.h"

class GLFWRenderer : public Renderer
{
public:
	// Constructors
	// ------------
	GLFWRenderer();

	// Destructor
	// ----------
	~GLFWRenderer();

	// Public Methods
	// --------------
	bool Init() const override;
	void CreateWindow(int width, int height, const char* title) override;
	void ConfigureWindow() const override;
	void PollIOEvents() const override;
	void SwapBuffers() const override;
	bool ShouldClose() const override;

	void WaitForEvents() const override;

	// Getters
	const char* GetProcAddress() const override;
	float GetTime() const override;

	// Setters
	void SetCallbackFunctions() const override;
	void SetWindowShouldClose() const override;

	// GUI
	void InitGUI() override;
	void BuildGUI() const override;
	void RenderGUI() const override;
	void ShutdownGUI() override;

private:
	// Private Attributes
	// ------------------
	GLFWwindow* window;
	GUI* gui;

	// Static Callback Functions
	// -------------------------
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

};