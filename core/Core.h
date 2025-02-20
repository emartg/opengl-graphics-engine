/*
* Core.h
* This file defines the Core class, which is is responsible for initializing OpenGL, creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
*/

#pragma once

#include <string>
#include <vector>
#include <memory> // for smart pointers
#include <map>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <GLFW/glfw3.h>
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

class Core
{
public:
	// Constructors
	// ------------
	Core();

	// Destructor
	// ----------
	~Core();

	// Public Methods
	// --------------
	// Pipeline methods
	void InitOGL(); // initializes OpenGL
	void CompileShaders(); // builds and compiles the shader programs and adds them to the engine
	void LoadTextures(); // loads the textures and adds them to the engine
	void MainLoop(); // the main rendering loop of the engine (includes input processing)

	// Setters
	void SetCamera(std::unique_ptr<Camera> camera) { m_camera = std::move(camera); }

	// Adds an asset to the engine (e.g., a camera, light, model, etc.)
	void AddAsset(std::unique_ptr<Asset> asset);

private:
	// Private Attributes
	// ------------------
	GLfloat m_lastMouseX, m_lastMouseY, m_firstMouse; // mouse settings
	GLfloat m_deltaTime, m_lastFrameTime; // time settings
	GLFWwindow* m_window; // window object
	std::map<std::string, std::vector<std::unique_ptr<Asset>>> m_assets; // map of assets
	std::unique_ptr<Camera> m_camera; // current camera object

	// Static Constants
	// ----------------
	static constexpr GLuint SCR_WIDTH{ 800 }, SCR_HEIGHT{ 600 }; // screen settings

	// Private Functions
	// -----------------
	// GLFW callback functions
	static void framebuffer_size_callback_static(GLFWwindow* window, GLint width, GLint height);
	static void mouse_callback_static(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn);
	static void scroll_callback_static(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);

	// GLFW callback engine-specific functions
	void framebuffer_size_callback(GLint width, GLint height);
	void mouse_callback(GLdouble xposIn, GLdouble yposIn);
	void scroll_callback(GLdouble xoffset, GLdouble yoffset);

	// Input processing
	void processInput();

	// Check if the window should close
	bool shouldClose() const;

};
