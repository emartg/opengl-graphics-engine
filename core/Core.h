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
#include <unordered_map>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "Asset.h"
#include "camera/Camera.h"
#include "light/Light.h"
#include "light/PointLight.h"
#include "model/Model.h"
#include "model/Shape.h"
#include "shader/Shader.h"
#include "texture/Texture.h"
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

	// Static Instance
	// ---------------
	static Core* m_instance; // instance of the Core class (Singleton)

	// Static Constants
	// ----------------
	static constexpr GLuint SCR_WIDTH{ 1000 }, SCR_HEIGHT{ 750 }; // screen settings

	// Private Attributes
	// ------------------
	Renderer* m_renderer; // current renderer object

	// engine-specific attributes
	GLfloat m_lastMouseX, m_lastMouseY, m_firstMouse; // mouse settings
	GLfloat m_deltaTime, m_lastFrameTime; // time settings
	std::unordered_map<std::string, std::vector<std::unique_ptr<Asset>>> m_assets; // map of assets
	std::unique_ptr<Camera> m_camera; // current camera object

	// gui-related attributes
	GLboolean m_cameraControlEnabled; // flag to disable camera control

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

	// Static Methods
	// --------------
	// Returns the instance of the Core class (Singleton)
	static Core* GetInstance();

	// Public Methods
	// --------------
	// Getters
	// get the renderer object
	const Renderer* GetRenderer() const { return m_renderer; }
	// get the camera object
	const std::unique_ptr<Camera>& GetCamera() { return m_camera; }
	// get number of lights in the scene
	const GLuint GetNLights() const { return Light::GetNLights(); }
	// get number of point lights in the scene
	const GLuint GetNPointLights() const { return PointLight::GetNPointLights(); }
	// get number of models in the scene
	const GLuint GetNModels() const { return Model::GetNModels(); }
	// get number of shapes in the scene
	const GLuint GetNShapes() const { return Shape::GetNShapes(); }
	// get all the assets of a specific type
	const std::vector<std::unique_ptr<Asset>>& GetAssets(const std::string& assetType) const;
	// get the camera control flag
	const GLboolean& GetCameraControlEnabled() const { return m_cameraControlEnabled; }

	// Setters
	// set the renderer object
	void SetRenderer(Renderer* renderer) { m_renderer = renderer; }
	// set the camera object
	void SetCamera(std::unique_ptr<Camera> camera) { m_camera = std::move(camera); }
	// set the camera control flag
	void SetCameraControlEnabled(GLboolean enabled) { m_cameraControlEnabled = enabled; }

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

	// Adds an asset to the engine (e.g., a camera, light, model, etc.)
	void AddAsset(std::unique_ptr<Asset> asset);

	// Engine-specific callback functions
	void FramebufferSizeCallback(GLint width, GLint height);
	void CursorPosCallback(GLdouble xposIn, GLdouble yposIn, std::string input);
	void ScrollCallback(GLdouble xoffset, GLdouble yoffset);

};
