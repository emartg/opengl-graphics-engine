/*
* Renderer.h
* Defines the interface for all renderers, which are responsible for:
* - Creating and managing windows
* - Handling input
* - Rendering graphics
*/

#pragma once

#include <string>

class Renderer
{
public:
	// Destructor
	// ----------
	virtual ~Renderer() {}

	// Public Methods
	// --------------
	virtual bool Init() const = 0;
	virtual void CreateWindow(int width, int height, const char* title) = 0;
	virtual void ConfigureWindow() const = 0;
	virtual void PollIOEvents() = 0;
	virtual void SwapBuffers() = 0;
	virtual void ClearBuffers() = 0;
	virtual bool ShouldClose() = 0;

	// Getters
	virtual const char* GetProcAddress() const = 0;
	virtual float GetTime() = 0;

	// Setters
	virtual void SetCallbackFunctions() const = 0;
	virtual void SetViewport(int width, int height) = 0;
	virtual void SetClearColor(float r, float g, float b, float a) = 0;
	virtual void SetWindowShouldClose() = 0;

	// GUI
	virtual void InitGUI() {};
	virtual void SetupGUI() {};
	virtual void RenderGUI() {};
	virtual void ShutdownGUI() {};

};