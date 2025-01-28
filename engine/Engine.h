/* Engine.h
This file defines the Engine class, which is is responsible for initializing OpenGL,
creating a window, and running the main loop.
It also manages the camera, the lighting and models that are to be rendered */

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

#include "camera/Camera.h"
#include "shader/Shader.h"
#include "model/Model.h"
#include "model/Shape.h"

class Engine
{
public:
	// Constructors
	// ------------
	Engine();

	// Destructor
	// ----------
	~Engine();

	// Public Methods
	// --------------
	void InitOGL(); // initialize OpenGL
	void MainLoop(); // the main rendering loop of the engine (includes input processing)

	void SetCamera(std::unique_ptr<Camera> camera);
	void SetLightPos(glm::vec3 lightPos);

	void AddResource(std::string name, std::unique_ptr<Model> model);

private:
	// Private Attributes
	// ------------------
	// Screen settings
	const GLuint SCR_WIDTH, SCR_HEIGHT;

	// Mouse settings
	GLfloat lastX, lastY, firstMouse;

	// Time settings
	GLfloat deltaTime, lastFrame; // time between current frame and last frame and time of last frame

	GLFWwindow* m_window; // the window object
	std::unique_ptr<Camera> m_camera; // the camera object
	glm::vec3 m_lightPos; // the light position
	std::vector<std::pair<std::string, std::unique_ptr<Model>>> m_models; // the models to be rendered

	// Private Functions
	// -----------------
	// GLFW callback functions
	static void framebuffer_size_callback_static(GLFWwindow* window, GLint width, GLint height);
	static void mouse_callback_static(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn);
	static void scroll_callback_static(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);

	// GLFW callback Engine-specific functions
	void framebuffer_size_callback(GLint width, GLint height);
	void mouse_callback(GLdouble xposIn, GLdouble yposIn);
	void scroll_callback(GLdouble xoffset, GLdouble yoffset);

	// Input processing
	void processInput();

	// Check if the window should close
	bool shouldClose() const;
};
