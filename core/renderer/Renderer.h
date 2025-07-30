/*
* Renderer.h
* Defines the interface for all renderers, which are responsible for:
* - Initializing OpenGL
* - Creating and managing windows
* - Rendering graphics
* - Managing GUI elements
* - Event polling and buffer swapping
* - Handling input
* - Etc.
* This interface allows for different implementations of renderers, such as GLFWRenderer, SDLRenderer, etc.
*/

#pragma once

#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations

enum class BufferType
{
	UNDEFINED = 0, COLOR, DEPTH, STENCIL,
	COLOR_DEPTH, COLOR_STENCIL, DEPTH_STENCIL, ALL
};

class Renderer
{
public:
	// Destructor
	// ----------
	// pure virtual destructor, must be defined to allow derived classes to implement it
	virtual ~Renderer() = 0;

	// Public Methods
	// --------------
	virtual bool Init() const = 0;
	virtual void ConfigOpenGL() const;
	virtual void CreateWindow(int width, int height, const char* title) = 0;
	virtual void ConfigureWindow() const = 0;
	virtual void PollIOEvents() const = 0;
	virtual void SwapBuffers() const = 0;
	virtual void ClearBuffers(BufferType bufferType = BufferType::ALL) const;
	virtual bool ShouldClose() const = 0;

	// Getters
	virtual const char* GetProcAddress() const = 0;
	virtual float GetTime() const = 0;

	// Setters
	virtual void SetCallbackFunctions() const = 0;
	virtual void SetViewport(int width, int height) const;
	virtual void SetClearColor(float r, float g, float b, float a = 1.0f) const;
	virtual void SetWindowShouldClose() const = 0;

	// GUI
	virtual void InitGUI() {};
	virtual void SetupGUI() const {};
	virtual void RenderGUI() const {};
	virtual void ShutdownGUI() {};

};