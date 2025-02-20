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
	for (auto& assetType : m_assets)
		for (auto& asset : assetType.second)
			asset->DeallocateResources();

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

void Core::CompileShaders()
{
	// Build and compile shader programs
	// ---------------------------------
	std::vector<Shader> shaders{
		{ "shape_shader", "shaders/shape.vert.glsl", "shaders/shape.frag.glsl" }
	};
	for (const auto& shader : shaders)
		AddAsset(std::make_unique<Shader>(shader));
}

void Core::LoadTextures()
{
	// Load textures
	// -------------
	std::vector<Texture> textures{
		{ "diffuse_blue_metal_plate_texture", "textures/blue_metal_plate_diffuse.jpg", TextureType::DIFFUSE },
		{ "specular_blue_metal_plate_texture", "textures/blue_metal_plate_specular.jpg", TextureType::SPECULAR }
	};
	for (const auto& texture : textures)
		AddAsset(std::make_unique<Texture>(texture));
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

	// Get local pointers to all the assets needed for rendering,
	// set the main camera, and output the assets to the console
	// ----------------------------------------------------------
	auto shader = std::make_unique<Shader>(*dynamic_cast<Shader*>(m_assets["SHADER"].front().get()));
	auto light = std::make_unique<PointLight>(*dynamic_cast<PointLight*>(m_assets["LIGHT"].front().get()));
	auto textures = std::vector<Texture>{ *dynamic_cast<Texture*>(m_assets["TEXTURE"].front().get()),
										  *dynamic_cast<Texture*>(m_assets["TEXTURE"].back().get()) };

	std::vector<Shape> shapes;
	for (GLuint i{}; i < m_assets["MODEL"].size(); i++)
	{
		auto shape = std::make_unique<Shape>(*dynamic_cast<Shape*>(m_assets["MODEL"][i].get()));
		shape->AddTextureData(textures.data(), textures.size());
		shapes.emplace_back(*shape);
	}

	// set the main camera
	m_camera = std::make_unique<Camera>(*dynamic_cast<Camera*>(m_assets["CAMERA"].front().get()));

	// output the assets to the console
	std::cout << "Assets loaded:" << std::endl;
	for (const auto& assetType : m_assets)
	{
		std::cout << "Asset type: " << assetType.first << std::endl;
		for (const auto& asset : assetType.second)
			std::cout << "Asset name: " << asset->GetName() << std::endl;
	}

	// Shader configuration
	// --------------------
	shader->Use();

	// vertex shader constant uniforms
	shader->SetVec3("lightPos", light->GetPosition());
	// fragment shader constant uniforms
	shader->SetVec3("light.ambient", light->GetAmbient());
	shader->SetVec3("light.diffuse", light->GetDiffuse());
	shader->SetVec3("light.specular", light->GetSpecular());
	shader->SetFloat("light.constant", light->GetConstant());
	shader->SetFloat("light.linear", light->GetLinear());
	shader->SetFloat("light.quadratic", light->GetQuadratic());

	shader->SetFloat("material.shininess", 32.0f);

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
		shader->Use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_camera->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_camera->GetViewMatrix();
		shader->SetMat4("projection", projection);
		shader->SetMat4("view", view);

		// render shapes at different locations
		for (GLuint i{}; i < shapes.size(); i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			// translate the shape to its corresponding location
			model = glm::translate(model, shapeTranslations[i]);
			shader->SetMat4("model", model);
			// draw the shape with the corresponding texture
			shapes[i].Draw(*shader);
		}

		// GLFW: swap buffers and poll IO events
		// -------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}
}

void Core::AddAsset(std::unique_ptr<Asset> asset)
{
	// get the key for the asset type
	std::string assetType;
	switch (asset->GetType())
	{
		case AssetType::CAMERA:
			assetType = "CAMERA";
			break;
		case AssetType::LIGHT:
			assetType = "LIGHT";
			break;
		case AssetType::MODEL:
			assetType = "MODEL";
			break;
		case AssetType::SHADER:
			assetType = "SHADER";
			break;
		case AssetType::TEXTURE:
			assetType = "TEXTURE";
			break;
		default:
			std::cerr << "AssetType not defined!" << std::endl;
			return;
	}
	// add asset to the corresponding vector in the map
	m_assets[assetType].emplace_back(std::move(asset));
}

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
			m_assets["CAMERA"].clear();
			auto camera = std::make_unique<Camera>("Main Camera");
			AddAsset(std::move(camera));
			m_camera = std::make_unique<Camera>(*dynamic_cast<Camera*>(m_assets["CAMERA"].front().get()));
		}
}

bool Core::shouldClose() const { return glfwWindowShouldClose(m_window); }
