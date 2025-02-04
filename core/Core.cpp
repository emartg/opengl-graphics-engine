/*
* Core.cpp
* This file implements the Core class, which is is responsible for initializing OpenGL, creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
*/

#include <iostream>
#include <memory> // for smart pointers

#include "Core.h"

Core::Core()
	: lastX{ SCR_WIDTH / 2.0f }, lastY{ SCR_HEIGHT / 2.0f }, firstMouse{ true },
	deltaTime{ 0.0f }, lastFrame{ 0.0f },
	m_window{ nullptr }, m_camera{ nullptr }, m_lightPos{ 1.0f }
{}

Core::~Core()
{
	// Terminate GLFW, clearing any resources allocated by GLFW
	glfwTerminate();
}

// Public Methods
// --------------
void Core::InitOGL()
{
	// GLFW: initialize and configure
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Test Window", nullptr, nullptr);
	if (m_window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);

	// GLFW: register callbacks
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback_static);
	glfwSetCursorPosCallback(m_window, mouse_callback_static);
	glfwSetScrollCallback(m_window, scroll_callback_static);

	// GLFW: other configurations
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // cursor is visible but not confined to the window

	// GLAD: load all OpenGL function pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return;
	}

	//  Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);
}

void Core::MainLoop()
{
	// Build and compile shader programs
	Shader cubeShader("shaders/vs.glsl", "shaders/fs_albedo.glsl");
	//Shader cubeShader("shaders/vs.glsl", "shaders/fs_textures.glsl");

	// Shader configuration
	cubeShader.Use();
	// vertex shader uniforms
	cubeShader.SetVec3("lightPos", m_lightPos);
	// fragment shader uniforms
	cubeShader.SetVec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f));	// low influence of ambient light
	cubeShader.SetVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));   // slightly higher influence of diffuse light
	cubeShader.SetVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));  // full influence of specular light
	cubeShader.SetFloat("light.constant", 1.0f);                        // constant attenuation term for a distance of 50
	cubeShader.SetFloat("light.linear", 0.09f);                         // linear attenuation term for a distance of 50
	cubeShader.SetFloat("light.quadratic", 0.032f);                     // quadratic attenuation term for a distance of 50
	//for (const auto& res : m_models)									// set the textures for each model
	//	res.second->BindTextures(cubeShader);							// sets "material.diffuseN" and "material.specularN" for each material
	cubeShader.SetVec3("material.albedo", glm::vec3(0.5f, 0.0f, 0.0f));	// albedo color for the material (red)
	cubeShader.SetFloat("material.shininess", 32.0f);					// shininess factor for the material

	// Render loop
	while (!shouldClose())
	{
		// Per-frame time logic
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput();

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		cubeShader.Use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_camera->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_camera->GetViewMatrix();
		cubeShader.SetMat4("projection", projection);
		cubeShader.SetMat4("view", view);
		// world transformation
		glm::mat4 model{ 1.0f };
		cubeShader.SetMat4("model", model);

		// render the model
		for (const auto& res : m_models)
			res.second->Draw();

		// GLFW: swap buffers and poll IO events
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}

	// De-allocate all resources once they've outlived their purpose
	glDeleteProgram(cubeShader.ID);
}

void Core::SetCamera(std::unique_ptr<Camera> camera) { m_camera = std::move(camera); }

void Core::SetLightPos(glm::vec3 lightPos) { m_lightPos = lightPos; }

void Core::AddResource(std::string name, std::unique_ptr<Model> model) { m_models.emplace_back(name, std::move(model)); }

// Private Methods
// ---------------
void Core::framebuffer_size_callback(GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

void Core::mouse_callback(GLdouble xposIn, GLdouble yposIn)
{
	static GLboolean rightMouseButtonPressed{ false };
	static GLboolean leftMouseButtonPressed{ false };

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		rightMouseButtonPressed = true;
		leftMouseButtonPressed = false;
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		leftMouseButtonPressed = true;
		rightMouseButtonPressed = false;
	}
	else
	{
		rightMouseButtonPressed = false;
		leftMouseButtonPressed = false;
	}

	GLfloat xpos = static_cast<GLfloat>(xposIn);
	GLfloat ypos = static_cast<GLfloat>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	if (m_camera) // only process mouse movement if a camera is present
		if (rightMouseButtonPressed) // the right mouse button is used to rotate the camera
			m_camera->ProcessMouseRotation(xoffset, yoffset);
		else if (leftMouseButtonPressed) // the left mouse button is used to translate the camera in 2D
			m_camera->ProcessMouseTranslation(xoffset, yoffset, 0.025f);
}

void Core::scroll_callback(GLdouble xoffset, GLdouble yoffset)
{
	if (m_camera) // only zoom if a camera is present
		m_camera->ProcessMouseScroll(static_cast<GLfloat>(yoffset), 2.5f);
}

void Core::framebuffer_size_callback_static(GLFWwindow* window, GLint width, GLint height)
{
	reinterpret_cast<Core*>(glfwGetWindowUserPointer(window))->framebuffer_size_callback(width, height);
}

void Core::mouse_callback_static(GLFWwindow* window, GLdouble xpos, GLdouble ypos)
{
	reinterpret_cast<Core*>(glfwGetWindowUserPointer(window))->mouse_callback(xpos, ypos);
}

void Core::scroll_callback_static(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	reinterpret_cast<Core*>(glfwGetWindowUserPointer(window))->scroll_callback(xoffset, yoffset);
}

void Core::processInput()
{
	// Close window on ESC
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	// Reset camera position and rotation on R
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
		if (m_camera)
		{
			auto camera = std::make_unique<Camera>(INITIAL_CAMERA_POSITION, INITIAL_CAMERA_UP, INITIAL_CAMERA_YAW, INITIAL_CAMERA_PITCH);
			SetCamera(std::move(camera));
		}
}

bool Core::shouldClose() const { return glfwWindowShouldClose(m_window); }
