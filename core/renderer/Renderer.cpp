/*
* Renderer.cpp
* Implements the Renderer interface, which is responsible for:
* - Initializing OpenGL
* - Creating and managing windows
* - Rendering graphics
* - Managing GUI elements
* - Event polling and buffer swapping
* - Handling input
* - Etc.
* This interface allows for different implementations of renderers based on different windowing libraries,
* such as GLFW, SDL, etc.
*/

#include "Renderer.h"

#include "../Core.h"

// Constructor
// -----------
Renderer::Renderer()
	: m_mainRenderPass{ new RenderPass() },
	m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0f },
	m_untexturedMattShapeShader{ nullptr }, m_assimpModelShader{ nullptr }, m_singleAlbedoShader{ nullptr }
{}

// Destructor
// ----------
Renderer::~Renderer()
{
	// deallocate the main render pass and nullify the pointer to avoid dangling pointer issues
	if (m_mainRenderPass) // check if the main render pass is not null
	{
		delete m_mainRenderPass; // deallocate the main render pass
		m_mainRenderPass = nullptr; // nullify the pointer to avoid dangling pointer issues
	}

	std::cout << "[RENDERER::~Renderer] Renderer destructor called" << std::endl;
}

// Public Methods
// --------------
void Renderer::ConfigOpenGL() const
{
	// depth buffer configuration:
	// 1. Enable the depth test
	// 2. Set the depth function to GL_LESS, which is the default depth function,
	//    i.e., discard fragments whose depth value is greater than or equal to 
	//    the current fragment's depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // default depth function (discard fragments behind the current fragment)

	// stencil buffer configuration:
	// 1. Enable the stencil test
	// 2. Set the stencil operation to the default operation, which means that
	//    the stencil value will not be modified by the stencil test whatever the outcome
	//    of the depth and stencil tests is
	// 3. Set the stencil function to pass only if the stencil value is not equal 
	//    to the reference value, which is set to 1 in this case
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
}

void Renderer::ClearBuffers(BufferType bufferType) const
{
	GLbitfield mask = 0; // initialize the mask to zero (no buffers cleared by default)
	switch (bufferType) // determine which buffers to clear based on the buffer type
	{
		case BufferType::COLOR:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case BufferType::DEPTH:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case BufferType::STENCIL:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::COLOR_DEPTH:
			mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
			break;
		case BufferType::COLOR_STENCIL:
			mask |= GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::DEPTH_STENCIL:
			mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::ALL:
		default:
			mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
	}
	glClear(mask); // clear the specified buffers
}

void Renderer::SetViewport(int width, int height) const
{
	// set the viewport to the specified width and height
	glViewport(0, 0, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a) const
{
	// set the clear color to the specified RGBA values (alpha defaults to 1.0f)
	glClearColor(r, g, b, a);
}

bool Renderer::SetShaderByName(const std::string& name, const std::shared_ptr<Shader>& shader)
{
	if (shader)
	{ // check if the shader is not null before proceeding
		// assign the shader to the appropriate member variable based on the name
		if (strcmp(name.c_str(), "Untextured Matt Shape Shader") == 0)
			m_untexturedMattShapeShader = shader;
		else if (strcmp(name.c_str(), "Assimp Model Shader") == 0)
			m_assimpModelShader = shader;
		else if (strcmp(name.c_str(), "Single Albedo Shader") == 0)
			m_singleAlbedoShader = shader;
		else
		{ // if the shader name is unknown, print an error message and return false
			std::cerr << "[ERROR::RENDERER::SetShader] Unknown shader name: " << name << std::endl;
			return false;
		}
	}
	else
	{ // if the shader is null, print an error message and return false
		std::cerr << "[ERROR::RENDERER::SetShader] Shader is null for name: " << name << std::endl;
		return false;
	}

	// if the shader was set successfully, print a success message and return true
	std::cout << "[SUCCESS::RENDERER::SetShader] Shader set successfully: " << name << std::endl;
	return true;
}

void Renderer::FrameStartConfig()
{
	PollIOEvents(); // poll input events (keyboard, mouse, etc.)

	BuildGUI(); // setup the GUI for the current frame

	// per-frame time logic
	GLfloat currentFrame = static_cast<GLfloat>(GetTime());
	m_deltaTime = currentFrame - m_lastFrameTime;
	m_lastFrameTime = currentFrame;

	// per-frame shader configuration
	SetClearColor(0.1f, 0.1f, 0.1f);
	ClearBuffers();
}

void Renderer::RenderScene()
{
	auto core = Core::GetInstance(); // get the core instance
	if (!core) // ensure the core instance is valid before proceeding
	{ // if the core instance is null, print an error message and return
		std::cerr << "[ERROR::RENDERER::RenderScene] Core instance is null" << std::endl;
		return;
	}

	// get the asset manager, scene manager, and camera from the core instance
	auto& assetManager = core->GetAssetManager();
	auto& sceneManager = core->GetSceneManager();
	auto& camera = sceneManager->GetCamera();

	// ensure camera and shaders are valid before proceeding
	if (!camera || !m_untexturedMattShapeShader || !m_assimpModelShader || !m_singleAlbedoShader)
	{ // if any of them are null, print an error message and return
		std::cerr << "[ERROR::RENDERER::RenderScene] Camera or shaders aren't set up correctly" << std::endl;
		return;
	}

	// compute view and projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->GetZoom()),
		static_cast<GLfloat>(core->GetScreenWidth()) / static_cast<GLfloat>(core->GetScreenHeight()),
		0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	// set the view and projection matrices for each shader program
	auto& shaders = assetManager->GetAssets(AssetType::SHADER);
	for (const auto& asset : shaders)
	{
		// dynamically cast the asset to a Shader object
		auto shader = std::dynamic_pointer_cast<Shader>(asset);

		shader->Use();
		shader->SetMat4("view", view);
		shader->SetMat4("projection", projection);
	}

	// set material uniforms
	m_untexturedMattShapeShader->Use();
	m_untexturedMattShapeShader->SetFloat("material.shininess", 32.0f); // shininess factor for the material

	m_assimpModelShader->Use();
	m_assimpModelShader->SetFloat("material.shininess", 32.0f); // shininess factor for the material

	// set light uniforms
	auto& lights = assetManager->GetAssets(AssetType::LIGHT);
	GLint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{};
	std::for_each(lights.begin(), lights.end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{
		// dynamically cast the asset to a Light object
		auto light = dynamic_cast<Light*>(asset.get());

		switch (light->GetLightType()) // switch based on the light type
		{
			case LightType::DIRECTIONAL_LIGHT:
			{
				// dynamically cast the light to a DirectionalLight object
				auto directionalLight = dynamic_cast<DirectionalLight*>(light);

				// prefix for fragment shader uniforms
				std::string prefix = "directionalLights[" + std::to_string(directionalLightIdx) + "].";

				// activate the untextured matt shape shader program
				m_untexturedMattShapeShader->Use();
				// vertex shader uniforms
				m_untexturedMattShapeShader->SetVec3(
					"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
					directionalLight->GetDirection()
				);
				// fragment shader uniforms
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", directionalLight->GetSpecular());

				// activate the assimp model shader program
				m_assimpModelShader->Use();
				// vertex shader uniforms
				m_assimpModelShader->SetVec3(
					"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
					directionalLight->GetDirection()
				);
				// fragment shader uniforms
				m_assimpModelShader->SetVec3(prefix + "ambient", directionalLight->GetAmbient());
				m_assimpModelShader->SetVec3(prefix + "diffuse", directionalLight->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", directionalLight->GetSpecular());

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
				m_untexturedMattShapeShader->Use();
				// vertex shader uniforms
				m_untexturedMattShapeShader->SetVec3(
					"pointLightPos[" + std::to_string(pointLightIdx) + "]",
					pointLight->GetPosition()
				);
				// fragment shader uniforms
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", pointLight->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", pointLight->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", pointLight->GetSpecular());
				m_untexturedMattShapeShader->SetFloat(prefix + "constant", pointLight->GetConstant());
				m_untexturedMattShapeShader->SetFloat(prefix + "linear", pointLight->GetLinear());
				m_untexturedMattShapeShader->SetFloat(prefix + "quadratic", pointLight->GetQuadratic());

				// activate the assimp model shader program
				m_assimpModelShader->Use();
				// vertex shader uniforms
				m_assimpModelShader->SetVec3(
					"pointLightPos[" + std::to_string(pointLightIdx) + "]",
					pointLight->GetPosition()
				);
				// fragment shader uniforms
				m_assimpModelShader->SetVec3(prefix + "ambient", pointLight->GetAmbient());
				m_assimpModelShader->SetVec3(prefix + "diffuse", pointLight->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", pointLight->GetSpecular());
				m_assimpModelShader->SetFloat(prefix + "constant", pointLight->GetConstant());
				m_assimpModelShader->SetFloat(prefix + "linear", pointLight->GetLinear());
				m_assimpModelShader->SetFloat(prefix + "quadratic", pointLight->GetQuadratic());

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
				m_untexturedMattShapeShader->Use();
				// vertex shader uniforms
				m_untexturedMattShapeShader->SetVec3(
					"spotlightPos[" + std::to_string(spotlightIdx) + "]",
					spotlight->GetPosition()
				);
				m_untexturedMattShapeShader->SetVec3(
					"spotlightDir[" + std::to_string(spotlightIdx) + "]",
					spotlight->GetDirection()
				);
				// fragment shader uniforms
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", spotlight->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", spotlight->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", spotlight->GetSpecular());
				m_untexturedMattShapeShader->SetFloat(prefix + "constant", spotlight->GetConstant());
				m_untexturedMattShapeShader->SetFloat(prefix + "linear", spotlight->GetLinear());
				m_untexturedMattShapeShader->SetFloat(prefix + "quadratic", spotlight->GetQuadratic());
				m_untexturedMattShapeShader->SetFloat(prefix + "innerCutOff", spotlight->GetInnerCutOff());
				m_untexturedMattShapeShader->SetFloat(prefix + "outerCutOff", spotlight->GetOuterCutOff());

				// activate the assimp model shader program
				m_assimpModelShader->Use();
				// vertex shader uniforms
				m_assimpModelShader->SetVec3(prefix + "ambient", spotlight->GetAmbient());
				m_assimpModelShader->SetVec3(
					"spotlightPos[" + std::to_string(spotlightIdx) + "]",
					spotlight->GetPosition()
				);
				m_assimpModelShader->SetVec3(
					"spotlightDir[" + std::to_string(spotlightIdx) + "]",
					spotlight->GetDirection()
				);
				// fragment shader uniforms
				m_assimpModelShader->SetVec3(prefix + "diffuse", spotlight->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", spotlight->GetSpecular());
				m_assimpModelShader->SetFloat(prefix + "constant", spotlight->GetConstant());
				m_assimpModelShader->SetFloat(prefix + "linear", spotlight->GetLinear());
				m_assimpModelShader->SetFloat(prefix + "quadratic", spotlight->GetQuadratic());
				m_assimpModelShader->SetFloat(prefix + "innerCutOff", spotlight->GetInnerCutOff());
				m_assimpModelShader->SetFloat(prefix + "outerCutOff", spotlight->GetOuterCutOff());

				spotlightIdx++; // increment the spotlight index for the next iteration
			}
			break;
			case LightType::UNDEFINED:
				// if the light type is undefined, print a message and return
				std::cerr << "[ERROR::RENDERER::RenderScene] Light type is undefined for light: "
					<< light->GetName() << std::endl;
				return;
			default:
				// if the light type is unknown, print an error message and return
				std::cerr << "[ERROR::RENDERER::RenderScene] Unknown light type for light: "
					<< light->GetName() << std::endl;
				return;

		}
	});
	m_untexturedMattShapeShader->Use();
	m_untexturedMattShapeShader->SetInt("nPointLights", pointLightIdx);
	m_untexturedMattShapeShader->SetInt("nSpotlights", spotlightIdx);
	m_untexturedMattShapeShader->SetInt("nDirectionalLights", directionalLightIdx);
	m_assimpModelShader->Use();
	m_assimpModelShader->SetInt("nPointLights", pointLightIdx);
	m_assimpModelShader->SetInt("nSpotlights", spotlightIdx);
	m_assimpModelShader->SetInt("nDirectionalLights", directionalLightIdx);

	// iterate over the vector of models and render them
	auto& models = assetManager->GetAssets(AssetType::MODEL);
	std::for_each(models.begin(), models.end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{
		// smart pointer to the shader program to use for rendering
		std::shared_ptr<Shader> renderShader;

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
				switch (model->GetModelType()) // switch based on the type of the model
				{
					case ModelType::ASSIMP_MODEL: // if the model is an Assimp model
					{
						// use the assimp model shader
						renderShader = m_assimpModelShader;
						// activate the current shader program
						renderShader->Use();
					}
					break;
					case ModelType::SHAPE: // if the model is an untextured matt shape 
					{
						// use the untextured matt shape shader
						renderShader = m_untexturedMattShapeShader;

						// activate the current shader program
						renderShader->Use();

						// set the color of the shape based on the model's albedo
						renderShader->SetVec3("material.albedo", model->GetAlbedo());
					}
					break;
					default:
						// if the model type is unknown, print an error message and return
						std::cerr << "[ERROR::RENDERER::RenderScene] Unknown model type for model: "
							<< model->GetName() << std::endl;
						return;
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
				renderShader = m_singleAlbedoShader;

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
			default: // if the gizmo type is unknown, print an error message and return
				std::cerr << "[ERROR::RENDERER::RenderScene] Unknown gizmo type for model: "
					<< model->GetName() << std::endl;
				return;
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
			m_singleAlbedoShader->Use();

			// set the projection and view matrices for the line						
			m_singleAlbedoShader->SetMat4("projection", projection);
			m_singleAlbedoShader->SetMat4("view", view);

			// the model matrix is not used for lines, but we set it to identity for consistency
			// (the line is drawn in world space, so it doesn't need a model matrix transformation)
			glm::mat4 model{ 1.0f };
			// set the model matrix for the line
			m_singleAlbedoShader->SetMat4("model", model);

			// set the color of the line based on the directional light's diffuse color
			m_singleAlbedoShader->SetVec3("albedo", dirLight->GetDiffuse());

			// render the directional light gizmo line with its bespoke Draw method
			line->Draw();
		}
	});
}

void Renderer::FrameEndConfig() const
{
	SwapBuffers(); // swap the front and back buffers to display the rendered content
}