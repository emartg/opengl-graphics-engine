/*
* Core.cpp
* This file implements the Core class, which is is responsible for initializing OpenGL,
* creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
* It is a Singleton class.
*/

#include <iostream>
#include <memory> // for smart pointers
#include <algorithm>

#include "Core.h"

#include "gizmos/Line.h" // for directional light gizmo rendering
#include "gizmos/RECTANGULAR_PLANE.h" // for directional light gizmo rendering

// Static Instance initialization
// ------------------------------
Core* Core::m_instance{ nullptr };

// Constructors
// ------------
Core::Core()
	: m_renderer{ nullptr },
	m_assetManager{ std::make_shared<AssetManager>() },
	m_inputManager{ std::make_shared<InputManager>() },
	m_sceneManager{ std::make_shared<SceneManager>() },
	m_selectionManager{ std::make_shared<SelectionManager>() }
{}

// Destructor
// ----------
Core::~Core()
{
	std::cout << "[INFO::CORE::~Core] Core destructor called" << std::endl;
}

// Public Static Methods
// ---------------------
Core* Core::GetInstance()
{
	if (!m_instance)
	{ // if the Core instance is null, create a new instance
		std::cout << "[INFO::CORE::GetInstance] Creating Core instance..." << std::endl;
		m_instance = new Core();
	}
	return m_instance;
}

// Private Static Methods
// ----------------------
void Core::destroyInstance()
{
	if (m_instance)
	{ // check if the Core instance is not null before destroying it
		std::cout << "[INFO::CORE::destroyInstance] Destroying Core instance..." << std::endl;
		delete m_instance; // destroy the Core instance
		m_instance = nullptr; // nullify the pointer to avoid dangling pointer issues
	}
}

// Public Methods
// --------------
bool Core::Init() const
{
	// use the current time as seed for the random number generator 
	// (to get different positions, colors, etc. each run)
	srand(static_cast<unsigned int>(time(0)));

	// initialize the renderer
	if (!m_renderer->Init())
	{ // if the renderer initialization fails, print an error message and return false
		std::cerr << "[ERROR::CORE::Init] Failed to initialize renderer" << std::endl;
		return false;
	}

	// create a window with the specified width, height, and title; and configure it
	m_renderer->CreateWindow(m_screenWidth, m_screenHeight, "Test Window");
	m_renderer->ConfigureWindow();

	// set callback functions
	m_renderer->SetCallbackFunctions();

	// load all OpenGL function pointers with GLAD
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(m_renderer->GetProcAddress())))
	{ // if GLAD fails to load OpenGL functions, print an error message and return false
		std::cerr << "[ERROR::CORE::Init] Failed to initialize GLAD" << std::endl;
		return false;
	}
	// now the OpenGL context is set up, and we can use OpenGL functions

	// set the viewport to the window size
	m_renderer->SetViewport(m_screenWidth, m_screenHeight);

	// initialize selection manager picking FBO with current window size
	if (m_selectionManager)
		m_selectionManager->Resize(m_screenWidth, m_screenHeight);
	// configure OpenGL global state
	m_renderer->ConfigOpenGL();
	// initialize the user interface
	m_renderer->InitGUI();

	// if all the initializations are successful, print a success message and return true
	std::cout << "[SUCCESS::CORE::Init] Core initialized successfully" << std::endl;
	return true;
}

void Core::Run()
{
	std::cout << "[INFO::CORE::Run] Starting main loop..." << std::endl;
	while (!m_renderer->ShouldClose())
	{
		// always wait for events first: the renderer fully blocks until an event occurs,
		// and when that happens, the renderer processes frames but throttles the frame rate,
		// reducing CPU / GPU usage and improving performance
		m_renderer->WaitForEvents();

		m_renderer->FrameStartConfig(); // start of the frame configuration
		m_renderer->RenderScene(); // composite the scene
		m_renderer->RenderGUI(); // render the GUI
		m_renderer->FrameEndConfig(); // end of the frame configuration
	}
}

void Core::Shutdown()
{
	if (m_renderer)
	{ // check if the renderer is not null before shutting it down
		std::cout << "[INFO::CORE::Shutdown] Destroying renderer..." << std::endl;
		m_renderer->ShutdownGUI(); // the GUI must be shut down before the renderer
		delete m_renderer; // destroy the renderer before the Core instance
		m_renderer = nullptr; // nullify the pointer to avoid dangling pointer issues
	}
	else
	{ // if the renderer is null, print an error message and return
		std::cerr << "[INFO::CORE::Shutdown] Renderer is null during Core destruction!" << std::endl;
		return;
	}

	// destroy the Core instance itself and print a message to the console
	destroyInstance();
	std::cout << "[INFO::CORE::Shutdown] Core shutdown complete" << std::endl;
}

bool Core::CompileShaders(const std::vector<std::string>& shaderNames,
						  const std::vector<std::string>& vertexShaderPaths,
						  const std::vector<std::string>& fragmentShaderPaths)
{
	size_t nShaders = shaderNames.size(); // number of shaders to compile

	// ensure the sizes of the input vectors match
	if (vertexShaderPaths.size() != nShaders
		|| fragmentShaderPaths.size() != nShaders)
	{ // if the sizes do not match, print an error message and return false
		std::cerr << "[ERROR::CORE::CompileShaders] Mismatched shader names and paths sizes!" << std::endl;
		return false;
	}

	for (size_t i{}; i < nShaders; i++)
	{ // iterate through the shader names and paths
		// create a new Shader object with the name and paths, and compile it
		auto shader = std::make_shared<Shader>(shaderNames[i], vertexShaderPaths[i], fragmentShaderPaths[i]);
		if (!shader->Compile()) // compile the shader
		{ // if the shader compilation fails, print an error message and return false
			std::cerr << "[ERROR::CORE::CompileShaders] Failed to compile shader: "
				<< shaderNames[i] << std::endl;
			return false;
		}

		// set the shader in the renderer by name
		if (!m_renderer->SetShaderByName(shader->GetName(), shader))
		{ // if the shader was not set successfully, print an error message and return false
			std::cerr << "[ERROR::CORE::CompileShaders] Failed to set shader: "
				<< shaderNames[i] << std::endl;
			return false;
		}
		else
		{ // if the shader was set successfully, print a success message
			std::cout << "[SUCCESS::CORE::CompileShaders] Shader set successfully: "
				<< shaderNames[i] << std::endl;
		}

		// add the compiled shader to the asset manager
		m_assetManager->AddAsset(std::move(shader));

		// if this is the picking shader, set it in the selection manager
		if (shaderNames[i] == "Picking Shader" && m_selectionManager)
		{
			auto added = std::dynamic_pointer_cast<Shader>(m_assetManager->GetAssets(AssetType::SHADER).back());
			m_selectionManager->SetPickingShader(added);
			std::cout << "[INFO::CORE::CompileShaders] Picking Shader assigned to SelectionManager" << std::endl;
		}
	}

	// if all shaders are compiled successfully, print a success message and return true
	std::cout << "[SUCCESS::CORE::CompileShaders] Shaders compiled successfully" << std::endl;
	return true;
}

bool Core::CompileShaders(const std::vector<std::string>& shaderNames,
						  const std::vector<std::string>& vertexShaderPaths,
						  const std::vector<std::string>& geometryShaderPaths,
						  const std::vector<std::string>& fragmentShaderPaths)
{
	// number of shaders to compile
	size_t nShaders = shaderNames.size();

	// ensure the sizes of the input vectors match
	if (vertexShaderPaths.size() != nShaders
		|| geometryShaderPaths.size() != nShaders
		|| fragmentShaderPaths.size() != nShaders)
	{ // if the sizes do not match, print an error message and return false
		std::cerr << "[ERROR::CORE::CompileShaders] Mismatched shader names and paths sizes!" << std::endl;
		return false;
	}

	for (size_t i{}; i < nShaders; i++)
	{ // iterate through the shader names and paths
		// create a new Shader object with the name and paths, and compile it
		auto shader = std::make_shared<Shader>(shaderNames[i],
											   vertexShaderPaths[i],
											   geometryShaderPaths[i],
											   fragmentShaderPaths[i]);
		if (!shader->Compile()) // compile the shader
		{ // if the shader compilation fails, print an error message and return false
			std::cerr << "[ERROR::CORE::CompileShaders] Failed to compile shader: "
				<< shaderNames[i] << std::endl;
			return false;
		}
		// add the compiled shader to the asset manager
		m_assetManager->AddAsset(std::move(shader));
	}

	// if all shaders are compiled successfully, print a success message and return true
	std::cout << "[SUCCESS::CORE::CompileShaders] Shaders compiled successfully" << std::endl;
	return true;
}

void Core::LoadTextures(const std::vector<std::string>& textureNames,
						const std::vector<std::string>& texturePaths,
						const std::vector<std::string>& textureTypes)
{
	for (GLuint i{}; i < textureNames.size(); i++)
	{
		auto texture = std::make_shared<Texture>(textureNames[i], texturePaths[i], TextureType::DIFFUSE);
		m_assetManager->AddAsset(std::move(texture));
	}
}

void Core::FramebufferSizeCallback(GLint width, GLint height)
{
	// keep Core's notion of the default framebuffer size in sync with the actual window size
	m_screenWidth = static_cast<GLuint>(std::max(0, width));
	m_screenHeight = static_cast<GLuint>(std::max(0, height));

	// update the default framebuffer viewport
	if (m_renderer) m_renderer->SetViewport(m_screenWidth, m_screenHeight);

	// keep picking/outline FBOs in sync with the default framebuffer size (the actual window size)s
	if (m_selectionManager) m_selectionManager->Resize(width, height);
}