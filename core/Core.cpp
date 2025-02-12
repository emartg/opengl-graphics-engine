/*
* Core.cpp
* This file implements the Core class, which is is responsible for initializing OpenGL, creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
*/

#include <iostream>
#include <memory> // for smart pointers

#include "Core.h"

Core::Core()
	: m_lastMouseX{ SCR_WIDTH / 2.0f }, m_lastMouseY{ SCR_HEIGHT / 2.0f }, m_firstMouse{ true },
	m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0f },
	m_window{ nullptr }, m_camera{ nullptr }
{}

Core::~Core()
{
	// Deallocate all of the engine's resources
	// ----------------------------------------
	for (auto& camera : m_cameras) { camera->DeallocateResources(); }
	for (auto& light : m_lights) { light->DeallocateResources(); }
	for (auto& shape : m_models) { shape->DeallocateResources(); }
	for (auto& shader : m_shaders) { shader->DeallocateResources(); }
	for (auto& texture : m_textures) { texture->DeallocateResources(); }

	// Terminate GLFW, clearing any resources allocated by GLFW
	// --------------------------------------------------------
	glfwTerminate();
}

// Public Methods
// --------------
void Core::InitOGL()
{
	// GLFW: initialization and configuration
	// --------------------------------------
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
	// ------------------------
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback_static);
	glfwSetCursorPosCallback(m_window, mouse_callback_static);
	glfwSetScrollCallback(m_window, scroll_callback_static);

	// GLFW: other configurations
	// ---------------------------
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // cursor is visible but not confined to the window

	// GLAD: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// OpenGL global state configuration
	// ---------------------------------
	glEnable(GL_DEPTH_TEST);
}

void Core::MainLoop()
{
	// Shape hardcoded transformation and color data
	// ---------------------------------------------
	const std::vector<glm::vec3> shapeTranslations
	{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
	};
	const std::vector<glm::vec3> shapeColors
	{
		glm::vec3(0.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.5f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.0f),
	};

	// Build and compile shader programs
	// ---------------------------------
	Shader shapeShader("shapeShader", "shaders/shape.vert.glsl", "shaders/shape.frag.glsl");
	AddShader(std::make_unique<Shader>(shapeShader));

	// Shader configuration
	// --------------------
	shapeShader.Use();
	// vertex shader constant uniforms
	shapeShader.SetVec3("lightPos", m_lights.front()->GetPosition());

	// fragment shader constant uniforms
	shapeShader.SetVec3("light.ambient", m_lights.front()->GetAmbient());
	shapeShader.SetVec3("light.diffuse", m_lights.front()->GetDiffuse());
	shapeShader.SetVec3("light.specular", m_lights.front()->GetSpecular());
	shapeShader.SetFloat("light.constant", m_lights.front()->GetConstant());
	shapeShader.SetFloat("light.linear", m_lights.front()->GetLinear());
	shapeShader.SetFloat("light.quadratic", m_lights.front()->GetQuadratic());

	shapeShader.SetFloat("material.shininess", 32.0f);

	// Render loop
	// -----------
	while (!shouldClose())
	{
		// Per-frame time logic
		// --------------------
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		m_deltaTime = currentFrame - m_lastFrameTime;
		m_lastFrameTime = currentFrame;

		// Input
		// -----
		processInput();

		// Render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		shapeShader.Use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_cameras.front()->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_cameras.front()->GetViewMatrix();
		shapeShader.SetMat4("projection", projection);
		shapeShader.SetMat4("view", view);

		// render shapes at different locations and with different colors
		for (size_t i{}; i < m_models.size(); ++i)
		{
			// set model matrix
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, shapeTranslations[i]);
			shapeShader.SetMat4("model", model);
			// set shape color
			shapeShader.SetVec3("material.albedo", shapeColors[i]);
			// render the model
			m_models[i]->Draw(shapeShader);
		}

		// GLFW: swap buffers and poll IO events
		// -------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}
}

void Core::AddCamera(std::unique_ptr<Camera> camera) { m_cameras.push_back(std::move(camera)); }

void Core::AddLight(std::unique_ptr<PointLight> pointLight) { m_lights.push_back(std::move(pointLight)); }

void Core::AddModel(std::unique_ptr<Shape> shape) { m_models.push_back(std::move(shape)); }

void Core::AddShader(std::unique_ptr<Shader> shader) { m_shaders.push_back(std::move(shader)); }

void Core::AddTexture(std::unique_ptr<Texture> texture) { m_textures.push_back(std::move(texture)); }

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

	if (m_firstMouse)
	{
		m_lastMouseX = xpos;
		m_lastMouseY = ypos;
		m_firstMouse = false;
	}

	GLfloat xoffset = xpos - m_lastMouseX;
	GLfloat yoffset = m_lastMouseY - ypos; // reversed since y-coordinates range from bottom to top
	m_lastMouseX = xpos;
	m_lastMouseY = ypos;

	if (m_cameras.front()) // only process mouse movement if a camera is present
		if (rightMouseButtonPressed) // the right mouse button is used to rotate the camera
			m_cameras.front()->ProcessMouseRotation(xoffset, yoffset);
		else if (leftMouseButtonPressed) // the left mouse button is used to translate the camera in 2D
			m_cameras.front()->ProcessMouseTranslation(xoffset, yoffset, 0.025f);
}

void Core::scroll_callback(GLdouble xoffset, GLdouble yoffset)
{
	if (m_cameras.front()) // only zoom if a camera is present
		m_cameras.front()->ProcessMouseScroll(static_cast<GLfloat>(yoffset), 2.5f);
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
		if (m_cameras.front())
		{
			auto camera = std::make_unique<Camera>("Main Camera");
			m_cameras.front() = std::move(camera);
		}
}

bool Core::shouldClose() const { return glfwWindowShouldClose(m_window); }
