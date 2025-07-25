/*
* Core.h
* This file defines the Core class, which is is responsible for initializing OpenGL,
* creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
* It is a Singleton class.
*/

#pragma once

#include <string>
#include <vector>
#include <memory> // for smart pointers

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "Asset.h"
#include "camera/Camera.h"
#include "managers/AssetManager.h"
#include "managers/InputManager.h"
#include "managers/SceneManager.h"
#include "renderer/Renderer.h"

class Core
{
private:
	// Constructors
	// ------------
	Core();

	// Destructor
	// ----------
	~Core();

	// Private Static Instance
	// -----------------------
	static Core* m_instance; // instance of the Core class (Singleton)

	// Private Attributes
	// ------------------
	Renderer* m_renderer;

	// manager instances
	std::shared_ptr<AssetManager> m_assetManager;
	std::shared_ptr<InputManager> m_inputManager;
	std::shared_ptr<SceneManager> m_sceneManager;

	// screen settings
	GLuint m_screenWidth{ 1400 }, m_screenHeight{ 1000 };

	// engine-specific attributes
	GLfloat m_deltaTime, m_lastFrameTime; // time settings


	// Private Functions
	// -----------------
	// Input processing
	void processInput(std::string input);

public:
	// Constructors
	// ------------
	Core(Core const&) = delete; // copy constructor (Singleton is not cloneable)

	// Operator overloading
	// --------------------
	void operator=(Core const&) = delete; // assignment operator (Singleton is not assignable)

	// Public Static Methods
	// ---------------------
	// Returns the instance of the Core class (Singleton)
	static Core* GetInstance();

	// Destroys the instance of the Core class (Singleton)
	static void DestroyInstance();

	// Public Methods
	// --------------
	// Getters
	const Renderer* GetRenderer() const { return m_renderer; }

	const std::shared_ptr<AssetManager>& GetAssetManager() const { return m_assetManager; }
	const std::shared_ptr<InputManager>& GetInputManager() const { return m_inputManager; }
	const std::shared_ptr<SceneManager>& GetSceneManager() const { return m_sceneManager; }

	const GLuint& GetScreenWidth() const { return m_screenWidth; }
	const GLuint& GetScreenHeight() const { return m_screenHeight; }

	// Setters
	void SetRenderer(Renderer* renderer) { m_renderer = renderer; }

	void SetScreenWidth(GLuint width) { m_screenWidth = width; }
	void SetScreenHeight(GLuint height) { m_screenHeight = height; }

	// Initializes OpenGL
	void InitOGL() const;
	// Builds and compiles the shaders and adds them to the engine
	void CompileShaders(const std::vector<std::string>& shaderNames,
						const std::vector<std::string>& vertexShaderPaths,
						const std::vector<std::string>& fragmentShaderPaths);
	void CompileShaders(const std::vector<std::string>& shaderNames,
						const std::vector<std::string>& vertexShaderPaths,
						const std::vector<std::string>& geometryShaderPaths,
						const std::vector<std::string>& fragmentShaderPaths);
	// Loads the textures and adds them to the engine
	void LoadTextures(const std::vector<std::string>& textureNames,
					  const std::vector<std::string>& texturePaths,
					  const std::vector<std::string>& textureTypes);
	// Main rendering loop of the engine (includes input processing)
	void MainLoop();

	// Engine-specific callback functions
	void FramebufferSizeCallback(GLint width, GLint height);

};
