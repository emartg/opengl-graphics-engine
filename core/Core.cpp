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
	m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0f }, // initialize time settings
	m_screenWidth{ 1400 }, m_screenHeight{ 1000 } // default screen settings 
{}

// Destructor
// ----------
Core::~Core()
{
	std::cout << "Core destructor called" << std::endl;
}

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
			std::cout << "Destroying renderer..." << std::endl;
			m_instance->m_renderer->ShutdownGUI(); // the GUI must be shut down before the renderer
			delete m_instance->m_renderer; // destroy the renderer before the Core instance
			m_instance->m_renderer = nullptr; // nullify the pointer to avoid dangling pointer issues
		}
		else
		{
			std::cerr << "Renderer is null during Core destruction!" << std::endl;
		}

		std::cout << "Destroying Core instance..." << std::endl;
		delete m_instance; // destroy the Core instance
		m_instance = nullptr; // nullify the pointer to avoid dangling pointer issues
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
	m_renderer->CreateWindow(m_screenWidth, m_screenHeight, "Test Window");
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
	m_renderer->SetViewport(m_screenWidth, m_screenHeight);

	// Initialize the GUI
	// ------------------
	m_renderer->InitGUI();

	// OpenGL global state configuration
	// ---------------------------------
	// depth buffer configuration:
	// 1. Enable the depth test
	// 2. Set the depth function to GL_LESS, which is the default depth function,
	//    i.e., discard fragments whose depth value is greater than or equal to 
	//    the current fragment's depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // default depth function (discard fragments behind the current fragment)

	// stencil buffer configuration:
	// 1. Enable the stencil test
	// 2. Set the stencil operation to replace the stencil value with the reference value 
	//	  if both the stencil test and depth test pass
	// 3. Set the stencil function to pass only if the stencil value is not equal 
	//    to the reference value, which is set to 1 in this case
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
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
		m_assetManager->AddAsset(std::move(shader));
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
		m_assetManager->AddAsset(std::move(shader));
	}
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

void Core::MainLoop()
{
	// Retrieve and store the assets already loaded in the engine
	// ----------------------------------------------------------
	// get raw pointers to the shaders from the asset manager
	auto camera = m_sceneManager->GetCamera(); // get the main camera from the scene manager
	auto shaders = std::vector<Shader*>();
	for (const auto& asset : m_assetManager->GetAssets(AssetType::SHADER))
	{ // get all the shaders from the asset manager
		if (auto shader = std::dynamic_pointer_cast<Shader>(asset))
			shaders.push_back(shader.get()); // store the raw pointer to the shader
		else
			std::cerr << "Asset " << asset->GetName() << " is not a Shader!" << std::endl;
	}

	// declare raw pointers to the shaders to use in the rendering loop
	Shader* untexturedMattShapeShader{ nullptr }; // pointer to the untextured matt shape shader
	Shader* assimpModelShader{ nullptr }; // pointer to the Assimp model shader
	Shader* singleAlbedoShader{ nullptr }; // pointer to the single albedo shader
	Shader* assimpModelOutlineShader{ nullptr }; // pointer to the Assimp model outline shader
	Shader* shapeOutlineShader{ nullptr }; // pointer to the shape outline shader

	// assign shaders to the local raw pointers based on their names
	// (provisional, allows us to get rid of shader indexes in the code, 
	// but introduces raw pointer usage and name checking)
	for (auto& shader : shaders)
	{
		if (strcmp(shader->GetName().c_str(), "Untextured Matt Shape Shader Program") == 0)
			untexturedMattShapeShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Assimp Model Shader Program") == 0)
			assimpModelShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Single Albedo Shader Program") == 0)
			singleAlbedoShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Assimp Model Outline Shader Program") == 0)
			assimpModelOutlineShader = shader;
		else if (strcmp(shader->GetName().c_str(), "Shape Outline Shader Program") == 0)
			shapeOutlineShader = shader;
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

	// single albedo shader configuration (if it exists)
	if (!singleAlbedoShader)
	{
		std::cerr << "Single Albedo Shader Program not found!" << std::endl;
		return; // exit the function if the single albedo shader is not found
	}

	// assimp model outline shader configuration (if it exists)
	if (assimpModelOutlineShader)
	{
		// activate the shader program
		assimpModelOutlineShader->Use();
		// vertex shader constant uniforms
		assimpModelOutlineShader->SetFloat("outlineThickness", 0.5f); // default outline thickness
		// fragment shader constant uniforms
		assimpModelOutlineShader->SetVec3("outlineAlbedo", glm::vec3{ 0.8f }); // default outline color (light gray)
	}
	else
	{
		std::cerr << "Assimp Model Outline Shader Program not found!" << std::endl;
		return; // exit the function if the Assimp model outline shader is not found
	}

	// shape outline shader configuration (if it exists)
	if (shapeOutlineShader)
	{
		// activate the shader program
		shapeOutlineShader->Use();
		// fragment shader constant uniforms
		assimpModelOutlineShader->SetVec3("outlineAlbedo", glm::vec3{ 0.8f }); // default outline color (light gray)
	}
	else
	{
		std::cerr << "Shape Outline Shader Program not found!" << std::endl;
		return; // exit the function if the shape outline shader is not found
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

		// Per-frame shader configuration
		// ------------------------------
		// set clear color and clear the color, depth, and stencil buffers
		m_renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		m_renderer->ClearBuffers();

		// compute view/projection transformations
		glm::mat4 projection = glm::perspective(
			glm::radians(camera->GetZoom()),
			static_cast<GLfloat>(m_screenWidth) / static_cast<GLfloat>(m_screenHeight),
			0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		// set the view and projection matrices for each shader program
		for (auto& shader : shaders)
		{
			shader->Use();
			shader->SetMat4("view", view);
			shader->SetMat4("projection", projection);
		}

		// iterators for the asset vectors from the asset manager
		auto& models = m_assetManager->GetAssets(AssetType::MODEL);
		auto& lights = m_assetManager->GetAssets(AssetType::LIGHT);

		// iterate over the vector of lights and set their properties
		GLint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{}; // counters for different light types
		std::for_each(lights.begin(), lights.end(),
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
					untexturedMattShapeShader->SetVec3(
						"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
						directionalLight->GetDirection()
					);
					// fragment shader uniforms
					untexturedMattShapeShader->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
					untexturedMattShapeShader->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
					untexturedMattShapeShader->SetVec3(prefix + "specular", directionalLight->GetSpecular());

					// activate the Assimp model shader program
					assimpModelShader->Use();
					// vertex shader uniforms
					assimpModelShader->SetVec3(
						"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
						directionalLight->GetDirection()
					);
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
					untexturedMattShapeShader->SetVec3(
						"pointLightPos[" + std::to_string(pointLightIdx) + "]",
						pointLight->GetPosition()
					);
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
					assimpModelShader->SetVec3(
						"pointLightPos[" + std::to_string(pointLightIdx) + "]",
						pointLight->GetPosition()
					);
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
					untexturedMattShapeShader->SetVec3(
						"spotlightPos[" + std::to_string(spotlightIdx) + "]",
						spotlight->GetPosition()
					);
					untexturedMattShapeShader->SetVec3(
						"spotlightDir[" + std::to_string(spotlightIdx) + "]",
						spotlight->GetDirection()
					);
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
					assimpModelShader->SetVec3(
						"spotlightPos[" + std::to_string(spotlightIdx) + "]",
						spotlight->GetPosition()
					);
					assimpModelShader->SetVec3(
						"spotlightDir[" + std::to_string(spotlightIdx) + "]",
						spotlight->GetDirection()
					);
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
		untexturedMattShapeShader->SetInt("nPointLights", pointLightIdx);
		untexturedMattShapeShader->SetInt("nSpotlights", spotlightIdx);
		untexturedMattShapeShader->SetInt("nDirectionalLights", directionalLightIdx);
		// set the current number of each type of light for the Assimp model shader
		assimpModelShader->Use();
		assimpModelShader->SetInt("nPointLights", pointLightIdx);
		assimpModelShader->SetInt("nSpotlights", spotlightIdx);
		assimpModelShader->SetInt("nDirectionalLights", directionalLightIdx);

		// I. First render pass: render the models as normal, writing to the stencil buffer
		// stencil buffer configuration:
		// 1. Set the stencil function to always pass
		// 2. Set the stencil mask to write to the stencil buffer
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		// iterate over the vector of models and render them
		std::for_each(models.begin(), models.end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// raw pointer meant to hold the current shader program for rendering
			Shader* renderShader{ nullptr };

			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			// switch based on the gizmo type to:
			// - set the current shader program accordingly for rendering
			// - set the appropiate properties for the model for rendering
			// - set the polygon mode (fill or line) for rendering
			switch (model->GetGizmoType())
			{
				case GizmoType::NONE: // if the model is not a gizmo
				{
					ModelType modelType = model->GetModelType(); // get the type of the model
					switch (modelType) // switch based on the type of the model
					{
						case ModelType::ASSIMP_MODEL: // if the model is an Assimp model
						{
							// use the Assimp model shader
							renderShader = assimpModelShader;

							// activate the current shader program
							renderShader->Use();
						}
						break;
						case ModelType::SHAPE: // if the model is an untextured matt shape 
						{
							// use the untextured matt shape shader
							renderShader = untexturedMattShapeShader;

							// activate the current shader program
							renderShader->Use();

							// set the color of the shape based on the model's albedo
							renderShader->SetVec3("material.albedo", model->GetAlbedo());
						}
						break;
						default:
						{
							std::cerr << "Unknown model type for model: " << model->GetName() << std::endl;
							return; // exit the function if the model type is unknown
						}
					}

					// set the model matrix for the model
					renderShader->SetMat4("model", model->GetModelMatrix());

					// set the polygon mode to fill for regular models
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				break;
				case GizmoType::DIRECTIONAL_LIGHT: // if the model is a directional light gizmo
				case GizmoType::POINT_LIGHT: // if the model is a point light gizmo
				case GizmoType::SPOTLIGHT: // if the model is a spotlight gizmo
				{
					// use the single albedo shader
					renderShader = singleAlbedoShader;

					// activate the current shader program
					renderShader->Use();

					// set the model matrix for the gizmo model
					renderShader->SetMat4("model", model->GetModelMatrix());

					// set the color of the gizmo shape based on the model's albedo
					renderShader->SetVec3("albedo", model->GetAlbedo());

					// set the polygon mode to line for light gizmos
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				break;
				default: // if the model is of an unknown gizmo type
					std::cerr << "Unknown gizmo type for model: " << model->GetName() << std::endl;
					return; // exit the function if the gizmo type is unknown
			}

			// draw the model using the current shader program
			model->Draw(*renderShader);
		});

		// set the polygon mode to line for the directional light gizmo lines
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// iterate over the vector of lights and render the directional light gizmo lines
		std::for_each(lights.begin(), lights.end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Light object
			auto light = dynamic_cast<Light*>(asset.get());

			// check if the light is a directional light
			if (light->GetLightType() == LightType::DIRECTIONAL_LIGHT)
			{
				// cast the light to a DirectionalLight object
				auto dirLight = dynamic_cast<DirectionalLight*>(light);

				// update the line's vertices to match the light's current position and direction
				// (prevents constantly re-creating Line objects)
				dirLight->UpdateGizmoDirectionLine();

				// retrieve the line from the directional light
				auto& line = dirLight->GetGizmoDirectionLine();

				// activate the single albedo shader for rendering the line
				singleAlbedoShader->Use();

				// set the projection and view matrices for the line						
				singleAlbedoShader->SetMat4("projection", projection);
				singleAlbedoShader->SetMat4("view", view);

				// the model matrix is not used for lines, but we set it to identity for consistency
				// (the line is drawn in world space, so it doesn't need a model matrix transformation)
				glm::mat4 model{ 1.0f };
				// set the model matrix for the line
				singleAlbedoShader->SetMat4("model", model);

				// set the color of the line based on the directional light's diffuse color
				singleAlbedoShader->SetVec3("albedo", dirLight->GetDiffuse());

				// render the directional light gizmo line with its bespoke Draw method
				line->Draw();
			}
		});

		// II. Second render pass: render outlines for the non-gizmo models
		//     - Render outlines for the Assimp models by extruding vertices 
		//       along their normals in the vertex shader. 
		//       This creates a uniform outline around the objects, regardless of their shape or size.
		//       It might not be consistent when working with Assimp models whose sizes differ significantly, 
		//       but it is a simple and effective way to render outlines.
		//	     The outline is rendered using a dedicated outline shader with a thickness value
		//     - Render outlines for the Shape models by scaling them up slightly
		//       and rendering them with a different color.
		//       The outline is rendered using a dedicated outline shader (a simple vertex shader).
		// stencil buffer configuration:
		// 1. Set the stencil function to pass only if the stencil value is not equal 
		//    to the reference value, which is set to 1 in this case
		// 2. Set the stencil mask to not write to the stencil buffer
		// 3. Disable the depth test to ensure the outline is rendered on top of the models
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		// set the polygon mode to fill for the outlines
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// activate the assimp model outline shader
		assimpModelOutlineShader->Use();
		// set the base outline thickness
		assimpModelOutlineShader->SetFloat("outlineThickness", 0.5f);

		// iterate over the vector of assets and render the outlines for the non-gizmo models
		std::for_each(models.begin(), models.end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{
			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			// skip rendering outlines for models that are gizmos
			if (model->GetGizmoType() != GizmoType::NONE)
				return;

			// depending on the model type, we may need to use a different outline shader
			// and set different properties for the outline rendering, thus the use of a raw pointer 
			// that will point to the appropriate outline shader
			Shader* outlineShader{ nullptr };

			glm::mat4 modelMatrix = model->GetModelMatrix(); // get the model matrix for the outline

			switch (model->GetModelType()) // switch based on the type of the model
			{
				case ModelType::ASSIMP_MODEL:
				{
					// use the Assimp model outline shader for Assimp models
					outlineShader = assimpModelOutlineShader;

					// activate the outline shader program
					outlineShader->Use();
				}
				break;
				case ModelType::SHAPE:
				{
					// use the shape outline shader for untextured matt shapes
					outlineShader = shapeOutlineShader;

					// activate the outline shader program
					outlineShader->Use();

					// scale the model matrix by 1.05 to create a slight outline effect
					modelMatrix = glm::scale(modelMatrix, glm::vec3(1.05f));
				}
				break;
				default:
				{
					std::cerr << "Unknown model type for model: " << model->GetName() << std::endl;
					return; // exit the function if the model type is unknown
				}
			}

			// set the outline color to light gray
			outlineShader->SetVec3("outlineAlbedo", glm::vec3{ 0.8f });

			// set the model matrix for the outline shader
			outlineShader->SetMat4("model", modelMatrix);

			model->Draw(*outlineShader);
		});

		// stencil buffer configuration
		// 4. Re-enable writing to the stencil buffer
		// 5. Set the stencil function to always pass again
		// 6. Re-enable the depth test
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_DEPTH_TEST);

		// Render the GUI
		// --------------
		m_renderer->RenderGUI();

		// Swap buffers
		// ------------
		m_renderer->SwapBuffers();
	}
}

void Core::FramebufferSizeCallback(GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

// Private Functions
// -----------------
void Core::processInput(std::string input)
{
	if (input == "ESC_PRESSED")
	{
		// if the escape key is pressed, set the window to close (debugging feature)
		m_renderer->SetWindowShouldClose();
		std::cout << "Escape key pressed, closing the window..." << std::endl;
	}
	else if (input == "R_PRESSED")
	{
		// if the 'R' key is pressed, reset the camera (debugging feature)
		m_sceneManager->GetCamera()->ResetCamera();
		std::cout << "Camera reset to default position and orientation" << std::endl;
	}
}
