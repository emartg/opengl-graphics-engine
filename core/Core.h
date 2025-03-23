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
#include <map>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "Asset.h"
#include "camera/Camera.h"
#include "light/PointLight.h"
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
	std::map<std::string, std::vector<std::unique_ptr<Asset>>> m_assets; // map of assets
	std::unique_ptr<Camera> m_camera; // current camera object

	// gui-related attributes
	std::vector<glm::vec3> m_cubePositions; // positions of the cubes in the scene
	std::vector<glm::vec3> m_cubeColors; // colors of the cubes in the scene
	glm::vec3 m_lightPos; // position of the light source
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
	const std::unique_ptr<Camera>& GetCamera() { return m_camera; }
	// get a reference to the vector of cube positions
	const std::vector<glm::vec3>& GetCubePositions() const { return m_cubePositions; }
	// get position of a specific cube in the scene
	const glm::vec3& GetCubePos(GLuint index) const { return m_cubePositions[index]; }
	// get a reference to the vector of cube colors
	const std::vector<glm::vec3>& GetCubeColors() const { return m_cubeColors; }
	// get the color of a specific cube in the scene
	const glm::vec3& GetCubeColor(GLuint index) const { return m_cubeColors[index]; }
	// get the number of cubes in the scene
	const GLuint GetNCubes() const { return m_cubePositions.size(); }
	// get the position of the light source
	const glm::vec3& GetLightPos() const { return m_lightPos; }
	// get the camera control flag
	const GLboolean& GetCameraControlEnabled() const { return m_cameraControlEnabled; }

	// Setters
	void SetRenderer(Renderer* renderer) { m_renderer = renderer; }
	void SetCamera(std::unique_ptr<Camera> camera) { m_camera = std::move(camera); }
	// set the position of a specific cube in the scene
	void SetCubePos(GLuint index, const glm::vec3& position) { m_cubePositions[index] = position; }
	// set the color of a specific cube in the scene
	void SetCubeColor(GLuint index, const glm::vec3& color) { m_cubeColors[index] = color; }
	// set the position of the light source
	void SetLightPos(const glm::vec3& position) { m_lightPos = position; }
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

	// Add a cube position to the vector of cube positions
	void AddCubePos(const glm::vec3& position) { m_cubePositions.push_back(position); }
	// Add a cube color to the vector of cube colors
	void AddCubeColor(const glm::vec3& color) { m_cubeColors.push_back(color); }

	// Engine-specific callback functions
	void FramebufferSizeCallback(GLint width, GLint height);
	void CursorPosCallback(GLdouble xposIn, GLdouble yposIn, std::string input);
	void ScrollCallback(GLdouble xoffset, GLdouble yoffset);

};
