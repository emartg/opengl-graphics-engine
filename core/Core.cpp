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
	// Retrieve and store the assets already loaded in the engine
	// ----------------------------------------------------------
	// set the main camera
	m_camera = std::make_shared<Camera>(*dynamic_cast<Camera*>(m_assets["CAMERA"].front().get()));

	// get raw pointers to the shader and light objects through dynamic casting
	auto shaders = std::vector<Shader*>();
	for (const auto& asset : m_assets["SHADER"])
		shaders.push_back(dynamic_cast<Shader*>(asset.get()));

	// declare pointers to the shaders to use in the rendering loop
	// (provisional, allows us to get rid of shader indexes in the code, 
	// but introduces raw pointer usage and name checking)
	Shader* untexturedMattShapeShader = nullptr; // pointer to the untextured matt shape shader
	Shader* assimpModelShader = nullptr; // pointer to the Assimp model shader
	Shader* gizmoShapeShader = nullptr; // pointer to the gizmo shape shader

	// assign shaders to the local raw pointers based on their names
	// (provisional, allows us to get rid of shader indexes in the code, 
	// but introduces raw pointer usage and name checking)
	for (auto& shader : shaders)
	{
		if (strcmp(shader->GetName().c_str(), "Untextured Matt Shape Shader Program") == 0)
			untexturedMattShapeShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Assimp Model Shader Program") == 0)
			assimpModelShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Gizmo Shape Shader Program") == 0)
			gizmoShapeShader = shader;
		else
		{
			std::cerr << "Unknown shader name: " << shader->GetName() << std::endl;
			continue; // skip this shader if it is not recognized
		}
	}

	// Shader configuration
	// --------------------
	// untextured matt shape shader configuration (if it exists)
	if (untexturedMattShapeShader)
	{
		// activate the shader program
		untexturedMattShapeShader->Use();
		// vertex shader constant uniforms
		untexturedMattShapeShader->SetFloat("material.shininess", 32.0f);
	}
	else
	{
		std::cerr << "Untextured Matt Shape Shader Program not found!" << std::endl;
		return; // exit the function if the untextured matt shape shader is not found
	}

	// Assimp model shader configuration (if it exists)
	if (assimpModelShader)
	{
		// activate the shader program
		assimpModelShader->Use();
		// vertex shader constant uniforms
		assimpModelShader->SetFloat("material.shininess", 32.0f);
	}
	else
	{
		std::cerr << "Assimp Model Shader Program not found!" << std::endl;
		return; // exit the function if the Assimp model shader is not found
	}

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

		// Per-frame shader configuration
		// ------------------------------
		m_renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		m_renderer->ClearBuffers();

		// compute view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(m_camera->GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT),
												0.1f, 100.0f);
		glm::mat4 view = m_camera->GetViewMatrix();
		// set the view and projection matrices for each shader program
		for (auto& shader : shaders)
		{
			shader->Use();
			shader->SetMat4("view", view);
			shader->SetMat4("projection", projection);
		}

		// indices for lights whose properties are to be set (counters for each type of light)
		GLuint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{};
		// iterate over the vector of assets of type "LIGHT" and set the properties of the lights
		// for every model shader program (provisional, allows us to get rid of shader indexes in the code,
		// but forces us to set the uniforms for each model shader program separately)
		std::for_each(m_assets["LIGHT"].begin(), m_assets["LIGHT"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Light object and get its type
			auto light = dynamic_cast<Light*>(asset.get());

			LightType lightType = light->GetLightType(); // get the type of the light
			switch (lightType) // switch based on the type of the light
			{
				case LightType::DIRECTIONAL_LIGHT:
				{
					// dynamically cast the light to a DirectionalLight object
					auto directionalLight = dynamic_cast<DirectionalLight*>(light);

					// prefix for fragment shader uniforms
					std::string prefix = "directionalLights[" + std::to_string(directionalLightIdx) + "].";

					// activate the untextured matt shape shader program
					untexturedMattShapeShader->Use();
					// vertex shader uniforms
					untexturedMattShapeShader->SetVec3("directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
													   directionalLight->GetDirection());
					// fragment shader uniforms
					untexturedMattShapeShader->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
					untexturedMattShapeShader->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
					untexturedMattShapeShader->SetVec3(prefix + "specular", directionalLight->GetSpecular());

					// activate the Assimp model shader program
					assimpModelShader->Use();
					// vertex shader uniforms
					assimpModelShader->SetVec3("directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
											   directionalLight->GetDirection());
					// fragment shader uniforms
					assimpModelShader->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
					assimpModelShader->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
					assimpModelShader->SetVec3(prefix + "specular", directionalLight->GetSpecular());

					directionalLightIdx++; // increment the directional light index for the next iteration
				}
				break;
				case LightType::POINT_LIGHT:
				{
					// dynamically cast the light to a PointLight object
					auto pointLight = dynamic_cast<PointLight*>(light);

					// prefix for fragment shader uniforms
					std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";

					// activate the untextured matt shape shader program
					untexturedMattShapeShader->Use();
					// vertex shader uniforms
					untexturedMattShapeShader->SetVec3("pointLightPos[" + std::to_string(pointLightIdx) + "]",
													   pointLight->GetPosition());
					// fragment shader uniforms
					untexturedMattShapeShader->SetVec3(prefix + "ambient", pointLight->GetAmbient());
					untexturedMattShapeShader->SetVec3(prefix + "diffuse", pointLight->GetDiffuse());
					untexturedMattShapeShader->SetVec3(prefix + "specular", pointLight->GetSpecular());
					untexturedMattShapeShader->SetFloat(prefix + "constant", pointLight->GetConstant());
					untexturedMattShapeShader->SetFloat(prefix + "linear", pointLight->GetLinear());
					untexturedMattShapeShader->SetFloat(prefix + "quadratic", pointLight->GetQuadratic());

					// activate the Assimp model shader program
					assimpModelShader->Use();
					// vertex shader uniforms
					assimpModelShader->SetVec3("pointLightPos[" + std::to_string(pointLightIdx) + "]",
											   pointLight->GetPosition());
					// fragment shader uniforms
					assimpModelShader->SetVec3(prefix + "ambient", pointLight->GetAmbient());
					assimpModelShader->SetVec3(prefix + "diffuse", pointLight->GetDiffuse());
					assimpModelShader->SetVec3(prefix + "specular", pointLight->GetSpecular());
					assimpModelShader->SetFloat(prefix + "constant", pointLight->GetConstant());
					assimpModelShader->SetFloat(prefix + "linear", pointLight->GetLinear());
					assimpModelShader->SetFloat(prefix + "quadratic", pointLight->GetQuadratic());

					pointLightIdx++; // increment the point light index for the next iteration
				}
				break;
				case LightType::SPOTLIGHT:
				{
					// dynamically cast the light to a Spotlight object
					auto spotlight = dynamic_cast<Spotlight*>(light);

					// prefix for fragment shader uniforms
					std::string prefix = "spotlights[" + std::to_string(spotlightIdx) + "].";

					// activate the untextured matt shape shader program
					untexturedMattShapeShader->Use();
					// vertex shader uniforms
					untexturedMattShapeShader->SetVec3("spotlightPos[" + std::to_string(spotlightIdx) + "]",
													   spotlight->GetPosition());
					untexturedMattShapeShader->SetVec3("spotlightDir[" + std::to_string(spotlightIdx) + "]",
													   spotlight->GetDirection());
					// fragment shader uniforms
					untexturedMattShapeShader->SetVec3(prefix + "ambient", spotlight->GetAmbient());
					untexturedMattShapeShader->SetVec3(prefix + "diffuse", spotlight->GetDiffuse());
					untexturedMattShapeShader->SetVec3(prefix + "specular", spotlight->GetSpecular());
					untexturedMattShapeShader->SetFloat(prefix + "constant", spotlight->GetConstant());
					untexturedMattShapeShader->SetFloat(prefix + "linear", spotlight->GetLinear());
					untexturedMattShapeShader->SetFloat(prefix + "quadratic", spotlight->GetQuadratic());
					untexturedMattShapeShader->SetFloat(prefix + "innerCutOff", spotlight->GetInnerCutOff());
					untexturedMattShapeShader->SetFloat(prefix + "outerCutOff", spotlight->GetOuterCutOff());

					// activate the Assimp model shader program
					assimpModelShader->Use();
					// vertex shader uniforms
					assimpModelShader->SetVec3(prefix + "ambient", spotlight->GetAmbient());
					assimpModelShader->SetVec3("spotlightPos[" + std::to_string(spotlightIdx) + "]",
											   spotlight->GetPosition());
					assimpModelShader->SetVec3("spotlightDir[" + std::to_string(spotlightIdx) + "]",
											   spotlight->GetDirection());
					// fragment shader uniforms
					assimpModelShader->SetVec3(prefix + "diffuse", spotlight->GetDiffuse());
					assimpModelShader->SetVec3(prefix + "specular", spotlight->GetSpecular());
					assimpModelShader->SetFloat(prefix + "constant", spotlight->GetConstant());
					assimpModelShader->SetFloat(prefix + "linear", spotlight->GetLinear());
					assimpModelShader->SetFloat(prefix + "quadratic", spotlight->GetQuadratic());
					assimpModelShader->SetFloat(prefix + "innerCutOff", spotlight->GetInnerCutOff());
					assimpModelShader->SetFloat(prefix + "outerCutOff", spotlight->GetOuterCutOff());

					spotlightIdx++; // increment the spotlight index for the next iteration
				}
				break;
				case LightType::UNDEFINED: // if the Light is of an undefined type
					std::cerr << "UNDEFINED light type for light: " << light->GetName() << std::endl;
					return; // exit the function if the light type is undefined
				default: // if the Light is of an unknown type
					std::cerr << "Unknown light type for light: " << light->GetName() << std::endl;
					return; // exit the function if the light type is unknown
			}
		});
		// set the current number of each type of light for the untextured matt shape shader
		untexturedMattShapeShader->Use();
		untexturedMattShapeShader->SetInt("nPointLights", static_cast<GLint>(pointLightIdx));
		untexturedMattShapeShader->SetInt("nSpotlights", static_cast<GLint>(spotlightIdx));
		untexturedMattShapeShader->SetInt("nDirectionalLights", static_cast<GLint>(directionalLightIdx));
		// set the current number of each type of light for the Assimp model shader
		assimpModelShader->Use();
		assimpModelShader->SetInt("nPointLights", static_cast<GLint>(pointLightIdx));
		assimpModelShader->SetInt("nSpotlights", static_cast<GLint>(spotlightIdx));
		assimpModelShader->SetInt("nDirectionalLights", static_cast<GLint>(directionalLightIdx));

		// Render the models and gizmos
		// ----------------------------
		// iterate over the vector of assets of type "MODEL" and render the different types of models
		std::for_each(m_assets["MODEL"].begin(), m_assets["MODEL"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			GizmoType gizmoType = model->GetGizmoType(); // get the type of the model's gizmo

			// declare a current shader pointer to use for rendering
			Shader* currentShader = nullptr;

			// switch based on the gizmo type to:
			// - set the current shader program accordingly for rendering
			// - set the appropiate properties for the model for rendering
			// - set the polygon mode (fill or line) for rendering
			switch (gizmoType)
			{
				case GizmoType::NONE: // if the model is not a gizmo
				{
					ModelType modelType = model->GetModelType(); // get the type of the model
					switch (modelType) // switch based on the type of the model
					{
						case ModelType::ASSIMP_MODEL: // if the model is an Assimp model
						{
							// use the Assimp model shader
							currentShader = assimpModelShader;

							// activate the current shader program
							currentShader->Use();
						}
						break;
						case ModelType::SHAPE: // if the model is an untextured matt shape 
						{
							// use the untextured matt shape shader
							currentShader = untexturedMattShapeShader;

							// activate the current shader program
							currentShader->Use();

							// set the color of the shape based on the model's albedo
							currentShader->SetVec3("material.albedo", model->GetAlbedo());
						}
						break;
						default:
						{
							std::cerr << "Unknown model type for model: " << model->GetName() << std::endl;
							return; // exit the function if the model type is unknown
						}
					}

					// set the model matrix for the model
					currentShader->SetMat4("model", model->GetModelMatrix());

					// set the polygon mode to fill for regular models
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				break;
				case GizmoType::DIRECTIONAL_LIGHT: // if the model is a directional light gizmo
				case GizmoType::POINT_LIGHT: // if the model is a point light gizmo
				case GizmoType::SPOTLIGHT: // if the model is a spotlight gizmo
				{
					// use the gizmo shape shader
					currentShader = gizmoShapeShader;

					// activate the current shader program
					currentShader->Use();

					// set the color of the gizmo shape based on the model's albedo
					currentShader->SetVec3("albedo", model->GetAlbedo());

					// set the model matrix for the gizmo model
					currentShader->SetMat4("model", model->GetModelMatrix());

					// set the polygon mode to line for light gizmos
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				break;
				default: // if the model is of an unknown gizmo type
					std::cerr << "Unknown gizmo type for model: " << model->GetName() << std::endl;
					return; // exit the function if the gizmo type is unknown
			}

			// draw the model using the current shader program
			model->Draw(*currentShader);
		});

		// iterate over the vector of assets of type "LIGHT" and render the directional light gizmo lines
		std::for_each(m_assets["LIGHT"].begin(), m_assets["LIGHT"].end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Light object
			auto light = dynamic_cast<Light*>(asset.get());

			LightType lightType = light->GetLightType(); // get the type of the light

			// check if the light is a directional light
			if (lightType == LightType::DIRECTIONAL_LIGHT)
			{
				// cast the light to a DirectionalLight object
				auto dirLight = dynamic_cast<DirectionalLight*>(light);

				// update the line's vertices to match the light's current position and direction
				// (prevents constantly re-creating Line objects)
				dirLight->UpdateGizmoDirectionLine();

				// retrieve the line from the directional light
				auto& line = dirLight->GetGizmoDirectionLine();

				// activate the gizmo shape shader for rendering the line
				gizmoShapeShader->Use();

				// set the projection and view matrices for the line						
				gizmoShapeShader->SetMat4("projection", projection);
				gizmoShapeShader->SetMat4("view", view);

				// the model matrix is not used for lines, but we set it to identity for consistency
				// (the line is drawn in world space, so it doesn't need a model matrix transformation)
				glm::mat4 model{ 1.0f };
				// set the model matrix for the line
				gizmoShapeShader->SetMat4("model", model);

				// set the color of the line based on the directional light's diffuse color
				gizmoShapeShader->SetVec3("albedo", dirLight->GetDiffuse());

				// render the directional light gizmo line with its bespoke Draw method
				line->Draw();
			}
		});

		// Render the GUI
		// --------------
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
