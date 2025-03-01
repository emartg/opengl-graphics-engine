/*
* GLFWRenderer.h
* Defines an inferited class of the Renderer interface using GLFW,
* a library for creating windows and handling input.
*/

#pragma once

#include "../core/Core.h"
#include "../core/renderer/Renderer.h"

#include <iostream>
#include <GLFW/glfw3.h>

class GLFWRenderer : public Renderer
{
public:
	GLFWRenderer();

	~GLFWRenderer();

	bool Init() const override;
	void CreateWindow(int width, int height, const char* title);
	void ConfigureWindow() const;
	void PollIOEvents() override;
	void SwapBuffers() override;
	void ClearBuffers() override;
	bool ShouldClose() override;
	const std::string ProcessKeyboardInput() override;

	const char* GetProcAddress() const override;
	float GetTime() override;

	void SetCallbackFunctions() const;
	void SetViewport(int width, int height) override;
	void SetClearColor(float r, float g, float b, float a = 1.0f) override;
	void SetWindowShouldClose() override;

private:
	GLFWwindow* window;

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

};