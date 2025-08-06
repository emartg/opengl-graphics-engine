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
#include <vector>
#include <memory> // for smart pointers
#include <algorithm>

#include <glad/glad.h> // holds all OpenGL type declarations

#include "RenderPass.h"

#include "../model/Model.h"
#include "../light/Light.h"
#include "../light/DirectionalLight.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"

enum class BufferType
{
	UNDEFINED = 0, COLOR, DEPTH, STENCIL,
	COLOR_DEPTH, COLOR_STENCIL, DEPTH_STENCIL, ALL
};

// forward declaration of the Core class to avoid circular dependency
class Core;

class Renderer
{
public:
	// Constructor
	// -----------
	Renderer();

	// Destructor
	// ----------
	// pure virtual destructor, must be defined to allow derived classes to implement it
	virtual ~Renderer();

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
	virtual float GetDeltaTime() const { return m_deltaTime; }

	// Setters
	virtual void SetCallbackFunctions() const = 0;
	virtual void SetViewport(int width, int height) const;
	virtual void SetClearColor(float r, float g, float b, float a = 1.0f) const;
	virtual void SetWindowShouldClose() const = 0;

	// Assigns the shader program with the specified name to the appropriate member variable
	// for further use in the renderer
	// Returns true if the shader was set successfully, false if the name is unknown or the shader is null
	virtual bool SetShaderByName(const std::string& name, const std::shared_ptr<Shader>& shader);

	// GUI
	virtual void InitGUI() {};
	virtual void SetupGUI() const {};
	virtual void RenderGUI() const {};
	virtual void ShutdownGUI() {};

	// Render Passes
	virtual void FrameStartConfig();
	virtual void RenderScene();
	virtual void FrameEndConfig() const;

protected:
	// Protected Attributes
	// --------------------
	RenderPass* m_mainRenderPass;

	GLfloat m_deltaTime, m_lastFrameTime; // time settings

	// shader programs' smart pointers
	std::shared_ptr<Shader> m_untexturedMattShapeShader;
	std::shared_ptr<Shader> m_assimpModelShader;
	std::shared_ptr<Shader> m_singleAlbedoShader;

};