/*
* Core.h
* This file defines the Core class, which is is responsible for initializing OpenGL, creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
*/

#pragma once

#include <string>
#include <vector>
#include <memory> // for smart pointers

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
	void InitOGL(); // initialize OpenGL
	void MainLoop(); // the main rendering loop of the engine (includes input processing)

	// Getters
	std::unique_ptr<Camera>& GetCamera() { return m_cameras.front(); }

	// Adds an asset to the engine (e.g., a camera, light, model, etc.)
	void AddAsset(std::unique_ptr<Asset> asset);
	void AddCamera(std::unique_ptr<Camera> camera);
	void AddLight(std::unique_ptr<PointLight> light);
	void AddModel(std::unique_ptr<Shape> shape);
	void AddShader(std::unique_ptr<Shader> shader);
	void AddTexture(std::unique_ptr<Texture> texture);

private:
	// Private Attributes
	// ------------------
	GLfloat m_lastMouseX, m_lastMouseY, m_firstMouse; // mouse settings
	GLfloat m_deltaTime, m_lastFrameTime; // time settings
	GLFWwindow* m_window; // window object
	std::vector<std::unique_ptr<Camera>> m_cameras; // cameras in the engine
	std::vector<std::unique_ptr<PointLight>> m_lights; // lights in the engine
	std::vector<std::unique_ptr<Shape>> m_models; // models in the engine
	std::vector<std::unique_ptr<Shader>> m_shaders; // shaders in the engine
	std::vector<std::unique_ptr<Texture>> m_textures; // textures in the engine
	std::unique_ptr<Camera> m_camera; // camera currently in use

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
