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

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory> 

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Core;
class Node;
class RenderPass;
class Shader;
class Texture;

enum class BufferType
{
	UNDEFINED = 0, COLOR, DEPTH, STENCIL,
	COLOR_DEPTH, COLOR_STENCIL, DEPTH_STENCIL, ALL
};

class Renderer
{
public:
	// Public Types
	// ------------
	// Struct that holds screen debug parameters to test screen-to-texture rendering (with default values)
	struct ScreenDebugParams
	{
		GLuint debugMode{ 0 }; // 0: regular rendering, 1: solid color, 2: grid overlay, 3: inverted colors
		glm::vec3 solidColor{ 0.75f, 0.25f, 0.25f }; // solid color for debug mode 1
		GLuint gridLineCount{ 50 }; // number of lines for the grid overlay for debug mode 2
		GLfloat gridLineThickness{ 1.50f }; // line thickness in pixels for the grid overlay for debug mode 2
		glm::vec3 gridBgColor{ 0.25f }; // background color for the grid overlay for debug mode 2
		glm::vec3 gridLineColor{ 0.75f }; // line color for the grid overlay for debug mode 2
	};

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

	// Blocks the main thread until an event occurs 
	// (mouse movement, key press, node manipulation via GUI, etc.)
	// Keeps held widgets responsive thanks to a reasonable throttle refresh rate
	// when events are being processed
	virtual void WaitForEvents() const = 0;

	// Getters
	virtual const char* GetProcAddress() const = 0;
	virtual float GetTime() const = 0;
	virtual float GetDeltaTime() const { return m_deltaTime; }

	virtual ScreenDebugParams& GetScreenDebugParams() { return m_screenDebugParams; }

	// Setters
	virtual void SetCallbackFunctions() const = 0;
	virtual void SetViewport(int width, int height) const;
	virtual void SetClearColor(float r, float g, float b, float a = 1.0f) const;
	virtual void SetWindowShouldClose() const = 0;

	virtual void SetScreenDebugParams(const ScreenDebugParams& params) { m_screenDebugParams = params; }

	// Assigns the shader program with the specified name to the appropriate member variable
	// for further use in the renderer.
	// Returns true if the shader was set successfully, false if the name is unknown or the shader is null
	virtual bool SetShaderByName(const std::string& name, const std::shared_ptr<Shader>& shader);

	// GUI
	virtual void InitGUI() {};
	virtual void BuildGUI() const {};
	virtual void RenderGUI() const {};
	virtual void ShutdownGUI() {};

	// Render Passes
	virtual void FrameStartConfig();
	virtual void RenderScene();
	virtual void FrameEndConfig() const;

	// Registration/Unregistration for dynamic environment map capture for models by their unique ids
	virtual void RegisterModelForDynamicEnvMapCapture(std::uint32_t modelId, GLuint resolution = 512);
	virtual void UnregisterModelForDynamicEnvMapCapture(std::uint32_t modelId);

protected:
	// Protected Attributes
	// --------------------
	RenderPass* m_mainRenderPass;
	GLfloat m_deltaTime, m_lastFrameTime; // time settings

	// shader programs' smart pointers
	std::shared_ptr<Shader> m_shapeModelShader;
	std::shared_ptr<Shader> m_assimpModelShader;
	std::shared_ptr<Shader> m_singleAlbedoShader;
	std::shared_ptr<Shader> m_screenShader;
	std::shared_ptr<Shader> m_pickingShader;
	std::shared_ptr<Shader> m_skyboxShader;
	std::shared_ptr<Shader> m_equirectangularToCubemapShader;
	std::shared_ptr<Shader> m_reflectiveShader;
	std::shared_ptr<Shader> m_refractiveShader;

	// buffers for the screen quad (for rendering the offscreen texture to the screen)
	GLuint m_screenQuadVAO{}, m_screenQuadVBO{}, m_screenQuadEBO{};
	GLuint m_offscreenWidth{}, m_offscreenHeight{};

	// screen debug parameters for testing screen-to-texture rendering
	ScreenDebugParams m_screenDebugParams;

	// buffers for the skybox cube
	GLuint m_skyboxVAO{}, m_skyboxVBO{}, m_skyboxEBO{};

	// buffers and flags for the HDR to cubemap conversion
	GLuint m_hdrToCubemapFBO{}, m_hdrToCubemapRBO{};
	bool m_hdrToCubemapConverted{ false };
	GLuint m_hdrSourceTexId{ 0 }; // to track if the HDR source texture has changed

	// attributes for dynamic environment maps
	struct DynamicEnvMapEntry
	{
		GLuint fbo{ 0 }, rbo{ 0 };
		GLuint cubemapTexId{ 0 }; // write target for this frame
		GLuint prevCubemapTexId{ 0 }; // stable sampling source for this frame
		GLuint resolution{ 512 }; // default resolution
		bool initialized{ false };
		bool hasPrevCubemap{ false }; // indicates if prevCubemapTexId is valid (rendered at least once)
	};
	// map of dynamic env maps by entity id
	std::unordered_map<std::uint32_t, DynamicEnvMapEntry> m_dynamicEnvMaps;
	bool m_isCapturingDynamicEnvMap{ false }; // flag to prevent recursion during dynamic env map capture

	// Protected Methods
	// -----------------
	// Renders nodes (leaf or composite - composite models will render their children recursively)
	void RenderModel(const std::shared_ptr<Node>& model);

	// Ensures the offscreen render pass is created with the current window size
	void EnsureOffscreenRenderPass();
	// Initializes the screen quad if it has not been initialized yet
	void InitScreenQuad();
	// Composites the offscreen render pass texture to the screen
	void CompositeToScreen();

	// Creates and configures the skybox cube buffers if not already done
	void InitSkyboxCube();
	// Renders the skybox cube with the given texture, view, and projection matrices
	void RenderSkyboxCube(std::shared_ptr<Texture> skyboxTexture,
						  glm::mat4& view, glm::mat4& projection);

	// Converts an equirectangular HDR texture to a cubemap texture if needed
	void ConvertHDRToCubemapIfNeeded();

	// Dynamic environment map helpers
	void UpdateDynamicEnvMaps();
	void CaptureDynamicEnvMapForModel(const std::shared_ptr<Node>& model,
									  DynamicEnvMapEntry& entry);
	void RenderSceneForEnvMapCapture(const glm::mat4& captureView, const glm::mat4& captureProjection,
									 const std::shared_ptr<Node>& excludeModel);

};