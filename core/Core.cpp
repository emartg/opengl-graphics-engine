/*
* Core.cpp
* This file implements the Core class, which is is responsible for initializing OpenGL,
* creating a window, and running the main loop.
* It also manages the camera, the lighting and models that are to be rendered.
* It is a Singleton class.
*/

#include <iostream>
#include <memory> // for smart pointers
#include <algorithm> // for std::for_each

#include "Core.h"

// Constructors
// ------------
Core::Core()
	: m_renderer{ nullptr },
	m_lastMouseX{ SCR_WIDTH / 2.0f }, m_lastMouseY{ SCR_HEIGHT / 2.0f }, m_firstMouse{ true },
	m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0f }, m_camera{ nullptr },
	m_cameraControlEnabled{ false }
{}

// Destructor
// ----------
Core::~Core()
{
	// Cleanup the GUI
	// ---------------
	m_renderer->CleanupGUI();

	// Deallocate all of the engine's resources
	// ----------------------------------------
	for (auto& assetType : m_assets)
		for (auto& asset : assetType.second)
			asset->DeallocateResources();

	// Delete the renderer
	// -------------------
	delete m_renderer;
	m_renderer = nullptr;
}

// Static Instance initialization
// ------------------------------
Core* Core::m_instance = nullptr;

// Static Methods
// --------------
Core* Core::GetInstance()
{
	if (!m_instance)
		m_instance = new Core();
	return m_instance;
}

void Core::DestroyInstance()
{
	delete m_instance;
	m_instance = nullptr;
}

// Public Methods
// --------------
void Core::InitOGL() const
{
	// Initialize the renderer
	// -----------------------
	if (!m_renderer->Init())
	{
		std::cerr << "Failed to initialize the renderer" << std::endl;
		return;
	}

	// Create the window and configure it
	// -----------------------------------
	m_renderer->CreateWindow(SCR_WIDTH, SCR_HEIGHT, "Test Window");
	m_renderer->ConfigureWindow();

	// Set the callback functions
	// --------------------------
	m_renderer->SetCallbackFunctions();

	// GLAD: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(m_renderer->GetProcAddress())))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return;
	}
	// Now the OpenGL context is set up, and we can use OpenGL functions

	// Set viewport
	// ------------
	m_renderer->SetViewport(SCR_WIDTH, SCR_HEIGHT);

	// Initialize the GUI
	// ------------------
	m_renderer->InitGUI();

	// OpenGL global state configuration
	// ---------------------------------
	glEnable(GL_DEPTH_TEST);
}

void Core::CompileShaders(const std::vector<std::string>& shaderNames,
						  const std::vector<std::string>& vertexShaderPaths,
						  const std::vector<std::string>& fragmentShaderPaths)
{
	for (GLuint i{}; i < shaderNames.size(); i++)
	{
		auto shader = std::make_unique<Shader>(shaderNames[i],
											   vertexShaderPaths[i].c_str(),
											   fragmentShaderPaths[i].c_str());
		AddAsset(std::move(shader));
	}
}

void Core::CompileShaders(const std::vector<std::string>& shaderNames,
						  const std::vector<std::string>& vertexShaderPaths,
						  const std::vector<std::string>& geometryShaderPaths,
						  const std::vector<std::string>& fragmentShaderPaths)
{
	for (GLuint i{}; i < shaderNames.size(); i++)
	{
		auto shader = std::make_unique<Shader>(shaderNames[i],
											   vertexShaderPaths[i].c_str(),
											   geometryShaderPaths[i].c_str(),
											   fragmentShaderPaths[i].c_str());
		AddAsset(std::move(shader));
	}
}

void Core::LoadTextures(const std::vector<std::string>& textureNames,
						const std::vector<std::string>& texturePaths,
						const std::vector<std::string>& textureTypes)
{
	for (GLuint i{}; i < textureNames.size(); i++)
	{
		auto texture = std::make_unique<Texture>(textureNames[i], texturePaths[i], TextureType::DIFFUSE);
		AddAsset(std::move(texture));
	}
}

void Core::MainLoop()
{
	// Get raw pointers to the shader and light objects through dynamic casting.
	// This is done because accessing objects via dynamic casting does not transfer ownership of the objects. 
	// Hence, these objects don't need to be manually deallocated, as the unique_ptrs in the m_assets map still 
	// own them and will deallocate them when the Core object is destroyed.
	// In short, these raw pointers are just temporary references to the objects managed by unique_ptrs.
	// -------------------------------------------------------------------------------------------------
	auto shader = dynamic_cast<Shader*>(m_assets["SHADER"].front().get());
	auto light = dynamic_cast<PointLight*>(m_assets["LIGHT"].front().get());

	// Set the main camera
	// -------------------
	m_camera = std::make_unique<Camera>(*dynamic_cast<Camera*>(m_assets["CAMERA"].front().get()));

	// Output the initially loaded assets to the console
	// --------------------------------------------------
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
	while (!m_renderer->ShouldClose())
	{
		// Poll IO events
		// --------------
		m_renderer->PollIOEvents();

		// Setup the GUI
		// -------------
		m_renderer->SetupGUI();

		// Per-frame time logic
		// --------------------
		GLfloat currentFrame = static_cast<GLfloat>(m_renderer->GetTime());
		m_deltaTime = currentFrame - m_lastFrameTime;
		m_lastFrameTime = currentFrame;

		// Input
		// -----
		processInput(m_renderer->ProcessKeyboardInput());

		// Render
		// ------
		m_renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		m_renderer->ClearBuffers();

		// activate shader
		shader->Use();

		// change the postion of the light source based on the GUI input
		glm::vec3 lightPos = light->GetPosition();
		shader->SetVec3("lightPos", lightPos);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_camera->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_camera->GetViewMatrix();
		shader->SetMat4("projection", projection);
		shader->SetMat4("view", view);

		// render the shapes using an algorithm to iterate over the vector of assets of type "MODEL":
		// render each shape, dynamically casting the Asset to a Shape object first,
		// then setting the shape's color and position based on the GUI input,
		// before finally rendering the shape
		std::for_each(m_assets["MODEL"].begin(), m_assets["MODEL"].end(),
					  [&](const std::unique_ptr<Asset>& asset)
		{
			auto shape = dynamic_cast<Shape*>(asset.get());

			// set the shape's color based on the GUI input
			shader->SetVec3("material.albedo", shape->GetAlbedo());

			// set the shape's position based on the GUI input
			glm::mat4 model{ 1.0f };
			model = glm::translate(model, shape->GetPosition());
			shader->SetMat4("model", model);

			// render the shape
			shape->Draw(*shader);
		});

		// render the GUI
		m_renderer->RenderGUI();

		// Swap buffers
		// ------------
		m_renderer->SwapBuffers();
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

const std::vector<std::unique_ptr<Asset>>& Core::GetAssets(const std::string& assetType) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	if (it != m_assets.end()) // ensure at least one asset of the type exists
		return it->second; // return the vector of assets of the specified type

	// return an empty vector if the asset type is not found
	static const std::vector<std::unique_ptr<Asset>> empty;
	return empty;
}

const std::unique_ptr<Asset>& Core::GetAssetByIndex(const std::string& assetType, GLuint index) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	if (it != m_assets.end()) // ensure at least one asset of the type exists
		return it->second[index]; // return the asset of the specified type at the specified index

	// return an empty unique_ptr if the asset type is not found
	static const std::unique_ptr<Asset> empty;
	return empty;
}

void Core::FramebufferSizeCallback(GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

void Core::CursorPosCallback(GLdouble xposIn, GLdouble yposIn, std::string input)
{
	static GLboolean rightMouseButtonPressed{ false };
	static GLboolean leftMouseButtonPressed{ false };

	if (input == "right_mouse_button_pressed")
	{
		rightMouseButtonPressed = true;
		leftMouseButtonPressed = false;
	}
	else if (input == "left_mouse_button_pressed")
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

	if (m_cameraControlEnabled) // only process mouse input if camera control is enabled
		if (rightMouseButtonPressed) // the right mouse button is used to rotate the camera
			m_camera->ProcessMouseRotation(xoffset, yoffset);
		else if (leftMouseButtonPressed) // the left mouse button is used to translate the camera in 2D
			m_camera->ProcessMouseTranslation(xoffset, yoffset, 0.025f);
}

void Core::ScrollCallback(GLdouble xoffset, GLdouble yoffset)
{
	if (m_cameraControlEnabled) // only process mouse scrolling if camera control is enabled
		m_camera->ProcessMouseScroll(yoffset, 2.0f);
}

// Private Functions
// -----------------
void Core::processInput(std::string input)
{
	if (input == "Esc_pressed")
		m_renderer->SetWindowShouldClose();
	else if (input == "R_pressed")
		m_camera->ResetCamera();
}
