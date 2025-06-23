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
#include "gizmos/Line.h" // for directional light gizmo rendering
#include "gizmos/RECTANGULAR_PLANE.h" // for directional light gizmo rendering

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
	std::cout << "Core destructor called" << std::endl;

	// Deallocate all of the engine's resources
	// ----------------------------------------
	for (auto& assetType : m_assets)
		for (auto& asset : assetType.second)
			asset->DeallocateResources();
}

// Static Instance initialization
// ------------------------------
Core* Core::m_instance = nullptr;

// Static Methods
// --------------
Core* Core::GetInstance()
{
	if (!m_instance)
	{
		std::cout << "Creating Core instance..." << std::endl;
		m_instance = new Core();
	}
	return m_instance;
}

void Core::DestroyInstance()
{
	if (m_instance)
	{
		if (m_instance->m_renderer)
		{
			m_instance->m_renderer->ShutdownGUI(); // the GUI must be shut down before the renderer
			std::cout << "Destroying renderer..." << std::endl;
			delete m_instance->m_renderer; // destroy the renderer before the Core instance
			m_instance->m_renderer = nullptr;
		}
		else
		{
			std::cerr << "Renderer is null during Core destruction!" << std::endl;
		}

		std::cout << "Destroying Core instance..." << std::endl;
		delete m_instance; // destroy the Core instance
		m_instance = nullptr;
	}
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
		auto shader = std::make_shared<Shader>(shaderNames[i],
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
		auto shader = std::make_shared<Shader>(shaderNames[i],
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
		auto texture = std::make_shared<Texture>(textureNames[i], texturePaths[i], TextureType::DIFFUSE);
		AddAsset(std::move(texture));
	}
}

void Core::MainLoop()
{
	// Get raw pointers to the shader and light objects through dynamic casting
	// ------------------------------------------------------------------------
	auto shaders = std::vector<Shader*>();
	for (const auto& asset : m_assets["SHADER"])
		shaders.push_back(dynamic_cast<Shader*>(asset.get()));

	// Set the main camera
	// -------------------
	m_camera = std::make_shared<Camera>(*dynamic_cast<Camera*>(m_assets["CAMERA"].front().get()));

	// Shader configuration
	// --------------------
	// model shader configuration
	shaders[0]->Use();
	// fragment shader constant uniforms
	shaders[0]->SetFloat("material.shininess", 32.0f);

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

		// activate the shader program for shapes
		shaders[0]->Use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_camera->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_camera->GetViewMatrix();
		shaders[0]->SetMat4("projection", projection);
		shaders[0]->SetMat4("view", view);

		// set the properties of the point lights iterate over the vector of assets of type "LIGHT"
		GLuint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{}; // indices for lights whose properties are to be set
		std::for_each(m_assets["LIGHT"].begin(), m_assets["LIGHT"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Light object and get its type
			auto light = dynamic_cast<Light*>(asset.get());
			LightType lightType = light->GetLightType();

			switch (lightType)
			{
				case LightType::DIRECTIONAL_LIGHT:
				{
					auto directionalLight = dynamic_cast<DirectionalLight*>(light);

					// vertex shader uniforms
					shaders[0]->SetVec3("directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
										directionalLight->GetDirection());

					// fragment shader uniforms
					std::string prefix = "directionalLights[" + std::to_string(directionalLightIdx) + "].";
					shaders[0]->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
					shaders[0]->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
					shaders[0]->SetVec3(prefix + "specular", directionalLight->GetSpecular());

					directionalLightIdx++; // increment the directional light index for the next iteration
				}
				break;
				case LightType::POINT_LIGHT:
				{
					auto pointLight = dynamic_cast<PointLight*>(light); // get the PointLight object

					// vertex shader uniforms
					shaders[0]->SetVec3("pointLightPos[" + std::to_string(pointLightIdx) + "]",
										pointLight->GetPosition());

					// fragment shader uniforms
					std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
					shaders[0]->SetVec3(prefix + "ambient", pointLight->GetAmbient());
					shaders[0]->SetVec3(prefix + "diffuse", pointLight->GetDiffuse());
					shaders[0]->SetVec3(prefix + "specular", pointLight->GetSpecular());
					shaders[0]->SetFloat(prefix + "constant", pointLight->GetConstant());
					shaders[0]->SetFloat(prefix + "linear", pointLight->GetLinear());
					shaders[0]->SetFloat(prefix + "quadratic", pointLight->GetQuadratic());

					pointLightIdx++; // increment the point light index for the next iteration
				}
				break;
				case LightType::SPOTLIGHT:
				{
					auto spotlight = dynamic_cast<Spotlight*>(light);

					// vertex shader uniforms
					shaders[0]->SetVec3("spotlightPos[" + std::to_string(spotlightIdx) + "]",
										spotlight->GetPosition());
					shaders[0]->SetVec3("spotlightDir[" + std::to_string(spotlightIdx) + "]",
										spotlight->GetDirection());

					// fragment shader uniforms
					std::string prefix = "spotlights[" + std::to_string(spotlightIdx) + "].";
					shaders[0]->SetVec3(prefix + "ambient", spotlight->GetAmbient());
					shaders[0]->SetVec3(prefix + "diffuse", spotlight->GetDiffuse());
					shaders[0]->SetVec3(prefix + "specular", spotlight->GetSpecular());
					shaders[0]->SetFloat(prefix + "constant", spotlight->GetConstant());
					shaders[0]->SetFloat(prefix + "linear", spotlight->GetLinear());
					shaders[0]->SetFloat(prefix + "quadratic", spotlight->GetQuadratic());
					shaders[0]->SetFloat(prefix + "innerCutOff", spotlight->GetInnerCutOff());
					shaders[0]->SetFloat(prefix + "outerCutOff", spotlight->GetOuterCutOff());

					spotlightIdx++; // increment the spotlight index for the next iteration
				}
				break;
				case LightType::UNDEFINED: // if the Light is of an undefined type
					std::cerr << "UNDEFINED light type for light: " << light->GetName() << std::endl;
					return;
				default: // if the Light is of an unknown type
					std::cerr << "Unknown light type for light: " << light->GetName() << std::endl;
					return;
			}
		});

		// set the current number of point lights
		shaders[0]->SetInt("nPointLights", static_cast<GLint>(pointLightIdx));
		// set the current number of spotlights
		shaders[0]->SetInt("nSpotlights", static_cast<GLint>(spotlightIdx));
		// set the current number of directional lights
		shaders[0]->SetInt("nDirectionalLights", static_cast<GLint>(directionalLightIdx));

		// activate the shader program for gizmos
		shaders[1]->Use();

		// view/projection transformations
		shaders[1]->SetMat4("projection", projection);
		shaders[1]->SetMat4("view", view);

		// render the models using an algorithm to iterate over the vector of assets of type "MODEL"
		std::for_each(m_assets["MODEL"].begin(), m_assets["MODEL"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			GLuint shaderIdx = 0; // default shader index to select a shader program (shape shader program)

			GizmoType gizmoType = model->GetGizmoType(); // get the type of the model's gizmo

			switch (gizmoType) // switch based on the gizmo type
			{
				// if the model is not a gizmo
				case GizmoType::NONE:
				{
					shaderIdx = 0; // set the shader index to 0 (shape shader program)

					// activate the shader program
					shaders[shaderIdx]->Use();

					// set the color based on the GUI input
					shaders[shaderIdx]->SetVec3("material.albedo", model->GetAlbedo());

					// set the model matrix for the model
					shaders[shaderIdx]->SetMat4("model", model->GetModelMatrix());

					// use fill mode to render regular models
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				break;
				// if the model is a DirectionalLight gizmo, PointLight gizmo, or Spotlight gizmo
				case GizmoType::DIRECTIONAL_LIGHT:
				case GizmoType::POINT_LIGHT:
				case GizmoType::SPOTLIGHT:
				{
					shaderIdx = 1; // set the shader index to 1 (gizmo shader program)

					// activate the shader program
					shaders[shaderIdx]->Use();

					// set the color based on the GUI input
					shaders[shaderIdx]->SetVec3("albedo", model->GetAlbedo());

					// set the model matrix for the model
					shaders[shaderIdx]->SetMat4("model", model->GetModelMatrix());

					// use wireframe mode to render gizmo models
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				break;
				default: // if the model is of an unknown gizmo type
					std::cerr << "Unknown gizmo model type for model: " << model->GetName() << std::endl;
					return;
			}

			// draw the model or gizmo model using the corresponding shader program
			model->Draw(*shaders[shaderIdx]);

		});

		// render additional gizmos using an algorithm to iterate over the vector of assets of type "LIGHT" again
		// (this includes directional light gizmos, which are rendered as lines)
		std::for_each(m_assets["LIGHT"].begin(), m_assets["LIGHT"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			auto light = dynamic_cast<Light*>(asset.get());
			if (light->GetLightType() == LightType::DIRECTIONAL_LIGHT)
			{
				auto dirLight = dynamic_cast<DirectionalLight*>(light);

				// update the line's vertices to match the light's current position and direction
				// with UpdateGizmoDirectionLine() (prevents constantly re-creating Line objects)
				dirLight->UpdateGizmoDirectionLine();

				// retrieve the line from the directional light
				auto& line = dirLight->GetGizmoDirectionLine();
				// use the gizmo shader program for rendering the line
				shaders[1]->Use();
				// set the projection and view matrices for the line						
				shaders[1]->SetMat4("projection", projection);
				shaders[1]->SetMat4("view", view);
				// the model matrix is not used for lines, but we set it to identity for consistency
				// (the line is drawn in world space, so it doesn't need a model matrix transformation)
				glm::mat4 model{ 1.0f };
				shaders[1]->SetMat4("model", model);
				// set the color of the line
				shaders[1]->SetVec3("albedo", dirLight->GetDiffuse());
				// render the directional light gizmo line with its bespoke Draw method
				line->Draw();
			}
		});

		// render the GUI
		m_renderer->RenderGUI();

		// Swap buffers
		// ------------
		m_renderer->SwapBuffers();
	}
}

void Core::AddAsset(std::shared_ptr<Asset> asset)
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
	m_assets[assetType].emplace_back(asset);
	// print the type and name of the asset added to the console
	std::cout << "Asset added: " << assetType << "\t| " << m_assets[assetType].back()->GetName() << std::endl;
}

const std::vector<std::shared_ptr<Asset>>& Core::GetAssets(const std::string& assetType) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	if (it != m_assets.end()) // ensure at least one asset of the type exists
		return it->second; // return the vector of assets of the specified type

	// return an empty vector if the asset type is not found
	static const std::vector<std::shared_ptr<Asset>> empty;
	return empty;
}

const std::shared_ptr<Asset>& Core::GetAssetByIndex(const std::string& assetType, GLuint index) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	if (it != m_assets.end()) // ensure at least one asset of the type exists
		return it->second[index]; // return the asset of the specified type at the specified index

	// return an empty shared_ptr if the asset type is not found
	static const std::shared_ptr<Asset> empty;
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
		m_camera->ProcessMouseScroll(yoffset, 2.5f);
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
