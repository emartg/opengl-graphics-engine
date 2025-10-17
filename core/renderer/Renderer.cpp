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
#include "SKYBOX.h" // skybox vertex data
#include "SCREEN_QUAD.h" // screen-quad vertex data

// Constructor
// -----------
Renderer::Renderer()
	: m_mainRenderPass{ new RenderPass() },
	m_deltaTime{ 0.0f }, m_lastFrameTime{ 0.0f }
{}

// Destructor
// ----------
Renderer::~Renderer()
{
	// deallocate the main render pass and nullify the pointer to avoid dangling pointer issues
	if (m_mainRenderPass)
	{
		delete m_mainRenderPass;
		m_mainRenderPass = nullptr;
	}

	// delete screen-quad GL objects if created
	if (m_screenQuadEBO) glDeleteBuffers(1, &m_screenQuadEBO);
	if (m_screenQuadVBO) glDeleteBuffers(1, &m_screenQuadVBO);
	if (m_screenQuadVAO) glDeleteVertexArrays(1, &m_screenQuadVAO);

	// delete skybox GL objects if created
	if (m_skyboxEBO) glDeleteBuffers(1, &m_skyboxEBO);
	if (m_skyboxVBO) glDeleteBuffers(1, &m_skyboxVBO);
	if (m_skyboxVAO) glDeleteVertexArrays(1, &m_skyboxVAO);

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

	// texture configuration:
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // enable seamless cubemap sampling
}

void Renderer::ClearBuffers(BufferType bufferType) const
{
	GLbitfield mask = 0;
	switch (bufferType)
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
	glViewport(0, 0, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a); // alpha is optional, default is 1.0f
}

bool Renderer::SetShaderByName(const std::string& name, const std::shared_ptr<Shader>& shader)
{
	if (!shader)
	{ // if the shader is null, print an error message and return false
		std::cerr << "[ERROR::RENDERER::SetShader] Shader is null for name: " << name << std::endl;
		return false;
	}

	// check if the shader name matches any of the known shaders, and if so,
	// assign the shader to the corresponding member variable
	if (strcmp(name.c_str(), "Untextured Matt Shape Shader") == 0)
		m_untexturedMattShapeShader = shader;
	else if (strcmp(name.c_str(), "Assimp Model Shader") == 0)
		m_assimpModelShader = shader;
	else if (strcmp(name.c_str(), "Single Albedo Shader") == 0)
		m_singleAlbedoShader = shader;
	else if (strcmp(name.c_str(), "Screen Shader") == 0)
		m_screenShader = shader;
	else if (strcmp(name.c_str(), "Picking Shader") == 0)
		m_pickingShader = shader;
	else if (strcmp(name.c_str(), "Skybox Shader") == 0)
		m_skyboxShader = shader;
	else if (strcmp(name.c_str(), "Equirectangular to Cubemap Shader") == 0)
		m_equirectangularToCubemapShader = shader;
	else
	{ // if the shader name is unknown, print an error message and return false
		std::cerr << "[ERROR::RENDERER::SetShader] Unknown shader name: " << name << std::endl;
		return false;
	}

	// if the shader was set successfully, print a success message and return true
	std::cout << "[SUCCESS::RENDERER::SetShader] Shader set successfully: " << name << std::endl;
	return true;
}

void Renderer::FrameStartConfig()
{
	PollIOEvents(); // poll IO events (keyboard, mouse, etc.)

	BuildGUI(); // set up the GUI for the frame

	// update time attributes
	GLfloat currentFrame = static_cast<GLfloat>(GetTime());
	m_deltaTime = currentFrame - m_lastFrameTime;
	m_lastFrameTime = currentFrame;

	EnsureOffscreenRenderPass(); // ensure offscreen target matches current window size

	// process pending picking request before main scene rendering
	auto core = Core::GetInstance();
	auto& camera = core->GetSceneManager()->GetCamera();
	core->GetSelectionManager()->ProcessPendingPick(camera.get(), core->GetAssetManager().get());

	// bind offscreen FBO and clear it
	m_mainRenderPass->Bind();
	SetClearColor(0.1f, 0.1f, 0.1f);
	ClearBuffers(BufferType::ALL);
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
	if (!camera || !m_untexturedMattShapeShader || !m_assimpModelShader || !m_singleAlbedoShader
		|| !m_screenShader || !m_pickingShader || !m_skyboxShader || !m_equirectangularToCubemapShader)
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
	m_assimpModelShader->SetInt("material.albedoMap", 0);		// set albedo map to texture unit 0
	m_assimpModelShader->SetInt("material.metallicMap", 1);		// set metallic map to texture unit 1
	m_assimpModelShader->SetFloat("material.shininess", 32.0f); // set shininess factor for the material

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
	m_untexturedMattShapeShader->SetInt("nDirectionalLights", directionalLightIdx);
	m_untexturedMattShapeShader->SetInt("nPointLights", pointLightIdx);
	m_untexturedMattShapeShader->SetInt("nSpotlights", spotlightIdx);
	m_assimpModelShader->Use();
	m_assimpModelShader->SetInt("nDirectionalLights", directionalLightIdx);
	m_assimpModelShader->SetInt("nPointLights", pointLightIdx);
	m_assimpModelShader->SetInt("nSpotlights", spotlightIdx);

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

	// set the polygon mode back to fill for the skybox rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// create and render the skybox last if a skybox texture is set
	// (rendering it last leverages the early depth test optimization,
	// since the skybox is rendered at the farthest depth, so it will
	// fail the depth test for all other scene objects, avoiding unnecessary fragment shader invocations)
	auto& skyboxTexture = sceneManager->GetSkybox();
	if (skyboxTexture)
	{ // if a skybox texture is set, proceed to render the skybox
		// perform (or retry) HDR to cubemap conversion if needed
		ConvertHDRToCubemapIfNeeded();

		skyboxTexture = sceneManager->GetSkybox(); // refresh pointer (conversion replaces the texture)

		// render the skybox only if the texture is a cubemap now
		if (skyboxTexture && skyboxTexture->GetTextureType() == TextureType::CUBEMAP)
		{ // if the skybox texture is now a cubemap, render the skybox
			if (!m_skyboxVAO) InitSkyboxCube(); // initialize the skybox cube if not done yet
			RenderSkyboxCube(skyboxTexture, view, projection);
		}
		else if (skyboxTexture && skyboxTexture->GetTextureType() == TextureType::HDR_EQUIRECTANGULAR)
		{ // if the skybox is still HDR, print a warning message but continue rendering
			// only warn occasionally (avoid spamming every frame)
			static uint32_t warnCounter = 0;
			if ((warnCounter++ % 240) == 0)
				std::cerr << "[WARNING::RENDERER::RenderScene] "
				"Skybox still HDR(conversion pending)" << std::endl;
		}
	}

	// after the scene is rendered offscreen, composite to the default framebuffer
	CompositeToScreen();
}

void Renderer::FrameEndConfig() const
{
	SwapBuffers();
}

// Protected Methods
// -----------------
void Renderer::EnsureOffscreenRenderPass()
{
	// get the core instance and screen dimensions
	auto core = Core::GetInstance();
	const int width = core->GetScreenWidth();
	const int height = core->GetScreenHeight();

	if (width <= 0 || height <= 0) return; // ensure valid dimensions

	// if the dimensions have changed or the FBO is not created yet, 
	// recreate the offscreen render pass with the new dimensions
	if (width != m_offscreenWidth || height != m_offscreenHeight || m_mainRenderPass->GetFboId() == 0)
	{
		// reset the specification and set new values
		RenderPassSpecification spec{};
		spec.Width = width;
		spec.Height = height;
		spec.ColorAttachmentCount = 1;
		spec.HasDepthAttachment = true;
		spec.HasStencilAttachment = true;
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.DepthAsTexture = true;

		// create the main render pass with the new specification and dimensions
		m_mainRenderPass->Create(spec);
		m_offscreenWidth = width;
		m_offscreenHeight = height;

		// print a message indicating the offscreen render pass has been recreated
		std::cout << "[INFO::RENDERER::EnsureOffscreenRenderPass] "
			"\nOffscreen render pass recreated with updated dimensions:\n"
			<< m_mainRenderPass->GetSpecificationStr() << std::endl;
	}

	InitScreenQuad(); // initialize the screen quad if it hasn't been initialized yet
}

void Renderer::InitScreenQuad()
{
	if (m_screenQuadVAO) return; // if the screen quad VAO is already initialized, return

	// create the screen quad VAO, VBO, and EBO
	glGenVertexArrays(1, &m_screenQuadVAO);
	glGenBuffers(1, &m_screenQuadVBO);
	glGenBuffers(1, &m_screenQuadEBO);

	glBindVertexArray(m_screenQuadVAO); // bind the VAO to set up the vertex attributes

	// bind the VBO and EBO, and send the screen quad vertex and index data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, m_screenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER,
				 static_cast<GLsizeiptr>(screenQuadVerticesVec.size() * sizeof(GLfloat)),
				 screenQuadVerticesVec.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screenQuadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 static_cast<GLsizeiptr>(screenQuadIndicesVec.size() * sizeof(GLuint)),
				 screenQuadIndicesVec.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers for the screen quad
	// compute a local stride for the vertex attributes (5 floats per vertex: 3 pos, 2 tex coords)
	const GLsizei stride = static_cast<GLsizei>(5 * sizeof(GLfloat));
	// position attribute at index 0 (3 floats)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	// texture coordinate attribute at index 1 (2 floats)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(GLfloat)));

	glBindVertexArray(0); // unbind the VAO to avoid accidental modifications
}

void Renderer::CompositeToScreen()
{
	// get the core instance and screen dimensions
	auto core = Core::GetInstance();
	const GLuint width = core->GetScreenWidth();
	const GLuint height = core->GetScreenHeight();

	// get the selection manager from the core instance
	auto selectionManager = core->GetSelectionManager();

	// depending on the debug mode and whether there is a selection,
	// render the outline mask or the picking visualization
	if (m_screenDebugParams.debugMode <= 1 && selectionManager->GetSelectedAssetId() != 0)
	{
		// if debug mode is either Normal mode (1) or Inverted Colors mode (3), and there is a selection,
		// render the outline mask
		auto camera = core->GetSceneManager()->GetCamera();
		selectionManager->RenderOutlineMask(camera.get(), core->GetAssetManager().get());
	}
	else if (m_screenDebugParams.debugMode == 2)
	{
		// if debug mode is the Picking Colors mode (2), render picking visualization
		auto camera = core->GetSceneManager()->GetCamera();
		selectionManager->RenderPickingVisualization(camera.get(), core->GetAssetManager().get());
	}

	// unbind offscreen target, effectively switching back to the default framebuffer
	m_mainRenderPass->Unbind();

	// draw the offscreen color texture to the back buffer via a screen quad
	glViewport(0, 0, width, height); // set the viewport to the screen dimensions
	glDisable(GL_DEPTH_TEST); // depth test is not needed for screen quad rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure we are in fill mode for the screen quad

	// clear the default framebuffer color (keep depth and stencil buffers intact)
	ClearBuffers(BufferType::COLOR);

	if (!m_screenShader)
	{ // if the screen shader is not set, print an error message and return
		std::cerr << "[ERROR::RENDERER::CompositeToScreen] Screen Shader not set" << std::endl;
		return;
	}

	// use the screen shader and set the texture to be rendered
	m_screenShader->Use();
	// set the screen texture uniform to texture unit 0
	m_screenShader->SetInt("screenTexture", 0);
	// set screen debug parameters as uniforms in the screen fragment shader
	std::string prefix = "screenDebugParams."; // prefix for the screen debug parameters
	m_screenShader->SetInt(prefix + "debugMode", m_screenDebugParams.debugMode);
	m_screenShader->SetVec3(prefix + "solidColor", m_screenDebugParams.solidColor);
	m_screenShader->SetInt(prefix + "gridLineCount", m_screenDebugParams.gridLineCount);
	m_screenShader->SetFloat(prefix + "gridLineThickness", m_screenDebugParams.gridLineThickness);
	m_screenShader->SetVec3(prefix + "gridBgColor", m_screenDebugParams.gridBgColor);
	m_screenShader->SetVec3(prefix + "gridLineColor", m_screenDebugParams.gridLineColor);
	// set the screen dimensions as a uniform in the screen fragment shader
	m_screenShader->SetVec2("screenSize", glm::vec2(width, height));

	// bind the offscreen render pass texture to texture unit 0 and set it as the active texture
	glActiveTexture(GL_TEXTURE0);

	// if debug mode is Picking Colors mode (2), depending on whether the picking texture is available,
	// bind the picking texture or the main color texture; otherwise, bind the main color texture
	if (m_screenDebugParams.debugMode == 2)
	{
		if (selectionManager->GetPickingTextureId() != 0)
		{ // if the picking texture is available, bind it and set nearest filtering
			glBindTexture(GL_TEXTURE_2D, selectionManager->GetPickingTextureId());
			// use nearest filtering to avoid interpolating encoded IDs
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{ // if the picking texture is not available, print a warning and bind the main color texture instead
			std::cerr << "[WARNING::RENDERER::CompositeToScreen] Picking texture not available, "
				"binding main render pass color texture instead" << std::endl;
			glBindTexture(GL_TEXTURE_2D, m_mainRenderPass->GetTextureId(0));
		}
	}
	else
	{ // otherwise, bind the main render pass color texture
		glBindTexture(GL_TEXTURE_2D, m_mainRenderPass->GetTextureId(0));
	}

	// local variables for outline parameters
	// there is only and outline if there is a selected asset and the outline mask texture is available
	const bool hasOutline =
		selectionManager->GetSelectedAssetId() != 0 && selectionManager->GetOutlineMaskTextureId() != 0;
	glm::vec3 outlineColor{ 0.0f };
	GLuint outlineThickness = 0;

	auto& params = selectionManager->GetOutlineParams(); // get the outline parameters
	if (m_screenDebugParams.debugMode <= 1 && hasOutline)
	{
		// if debug mode is either Normal mode (1) or Inverted Colors mode (2)
		// and there is a selection, use the outline parameters from the selection manager
		outlineColor = params.color;
		outlineThickness = params.thickness;
	}

	// set outline parameters as uniforms in the screen fragment shader
	m_screenShader->SetInt("hasOutline", hasOutline);
	m_screenShader->SetVec3("outlineColor", outlineColor);
	m_screenShader->SetInt("outlineThickness", outlineThickness);

	// bind depth textures if available for depth-aware outline rendering and set related uniforms
	GLuint sceneDepthTex = m_mainRenderPass->GetDepthTextureId(); // retrieve the scene depth texture
	// for the outline depth texture, reuse selection manager's outline pass depth (valid if mask rendered)
	GLuint outlineDepthTex = 0;
	if (hasOutline)
	{ // if there is an outline to render, get the outline depth texture
		outlineDepthTex = selectionManager->GetOutlineDepthTextureId();
	}

	// inform the shader if depth textures are available and set a small bias to avoid z-fighting
	// (need at least the scene depth texture for depth-aware outline rendering, 
	// outline depth is tested within the shader if available)
	const bool depth = sceneDepthTex != 0;
	m_screenShader->SetInt("hasDepthTextures", depth ? 1 : 0);
	m_screenShader->SetFloat("outlineDepthBias", 0.0005f);

	// bind depth textures to texture units 2 and 3 (only if available)
	glActiveTexture(GL_TEXTURE2);
	m_screenShader->SetInt("sceneDepthTexture", 2);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
	glActiveTexture(GL_TEXTURE3);
	m_screenShader->SetInt("outlineDepthTexture", 3);
	glBindTexture(GL_TEXTURE_2D, outlineDepthTex);

	// bind the outline mask texture to texture unit 1 and set it as the active texture
	glActiveTexture(GL_TEXTURE1);
	m_screenShader->SetInt("outlineMaskTexture", 1);
	if (hasOutline) // if there is an outline to render, bind the outline mask texture
		glBindTexture(GL_TEXTURE_2D, selectionManager->GetOutlineMaskTextureId());
	else // otherwise, bind texture 0 to avoid undefined behavior in the shader
		glBindTexture(GL_TEXTURE_2D, 0);

	// bind the screen quad VAO and draw the screen quad
	glBindVertexArray(m_screenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	// unbind the texture to avoid accidental modifications
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST); // re-enable the depth test for subsequent rendering
}

void Renderer::InitSkyboxCube()
{
	// create skybox VAO, VBO, and EBO
	glGenVertexArrays(1, &m_skyboxVAO);
	glGenBuffers(1, &m_skyboxVBO);
	glGenBuffers(1, &m_skyboxEBO);

	// bind and set skybox VAO, VBO, and EBO
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxPositionsVec.size() * sizeof(GLfloat),
				 skyboxPositionsVec.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVec.size() * sizeof(GLuint),
				 skyboxIndicesVec.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindVertexArray(0); // unbind the VAO after setting it up
}

void Renderer::RenderSkyboxCube(std::shared_ptr<Texture> skyboxTexture,
								glm::mat4& view, glm::mat4& projection)
{
	// disable depth writing for the skybox to prevent it from overwriting
	// the depth values of the scene's objects (skybox is rendered at the farthest depth),
	// and set the depth function to GL_LEQUAL to ensure the skybox is rendered
	// correctly when depth values are equal (skybox depth is 1.0) - avoids z-fighting
	glDepthMask(GL_FALSE); glDepthFunc(GL_LEQUAL);

	// set view and projection matrices for the skybox shader
	m_skyboxShader->Use();
	// for the skybox, remove the translation from the view matrix, since the skybox 
	// should always be centered and appear infinitely far away, 
	// so we cast the mat4 to a mat3 and back to a mat4 to remove the translation component. 
	// This way, the skybox will not move when the camera moves
	// and will only rotate based on the camera's orientation
	glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view)); // remove translation with a mat3 cast
	m_skyboxShader->SetMat4("view", viewNoTranslation);
	m_skyboxShader->SetMat4("projection", projection);
	// explicitly set the skybox texture unit to 0 to prevent reliance on defaults or previous bindings
	m_skyboxShader->SetInt("skybox", 0);

	// bind the VAO for the skybox and bind the cubemap texture, then render the skybox
	glBindVertexArray(m_skyboxVAO);
	glActiveTexture(GL_TEXTURE0); // ensure the correct texture unit is active before binding (0)
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture->GetTextureId());
	glDrawElements(GL_TRIANGLES, nSkyboxIndices, GL_UNSIGNED_INT, 0);

	// reset depth function and re-enable depth writing after rendering the skybox
	glDepthFunc(GL_LESS); glDepthMask(GL_TRUE);

	glBindVertexArray(0); // unbind the VAO after rendering
}

void Renderer::ConvertHDRToCubemapIfNeeded()
{
	// get the core instance, the scene manager, and the current skybox and check if it exists
	auto core = Core::GetInstance();
	auto& sceneManager = core->GetSceneManager();
	auto& skybox = sceneManager->GetSkybox();
	if (!skybox) return; // if no skybox is set, return

	// in case the skybox is not HDR, there is nothing to convert, so return
	if (skybox->GetTextureType() != TextureType::HDR_EQUIRECTANGULAR) return;

	if (m_hdrSourceTexId != skybox->GetTextureId())
	{ // if the HDR source texture has changed, reset the converted flag
		m_hdrSourceTexId = skybox->GetTextureId();
		m_hdrToCubemapConverted = false;
	}
	// only convert if not converted yet, otherwise return
	if (m_hdrToCubemapConverted) return;

	if (!m_equirectangularToCubemapShader)
	{ // if the equirectangular to cubemap shader is not set, print an error message and return
		std::cerr << "[ERROR::RENDERER::ConvertHDRToCubemapIfNeeded] "
			"Equirectangular to Cubemap Shader not set" << std::endl;
		return;
	}

	// create HDR to cubemap FBO and RBO if not created yet
	// (they are capture FBO and RBOs because they are used to capture the six faces of the cubemap)
	if (!m_hdrToCubemapFBO)
	{ // if the capture FBO and RBO are not created yet, create them
		glGenFramebuffers(1, &m_hdrToCubemapFBO);
		glGenRenderbuffers(1, &m_hdrToCubemapRBO);
	}

	// save currently bound FBO (main offscreen pass) so it can be restored later
	GLint prevFBO{};
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO); // get currently bound FBO

	// set the capture resolution to 4096x4096 (sufficient for good quality skyboxes)
	const GLuint captureSize{ 4096 };

	// bind capture FBO and (re)configure depth RBO for the capture dimensions
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrToCubemapFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_hdrToCubemapRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hdrToCubemapRBO);

	// explicitely select color attachment 0 for rendering (avoids issues on some drivers and platforms)
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// validate capture FBO completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{ // if the capture FBO is not complete, print an error message, restore previous FBO, and return
		std::cerr << "[ERROR::RENDERER::ConvertHDRToCubemapIfNeeded] Capture FBO incomplete" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
		return;
	}

	// create empty cubemap texture to render to and attach to FBO (RGB16F for HDR)
	GLuint envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i) // allocate space for the 6 faces of the cubemap
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, captureSize, captureSize, 0,
					 GL_RGB, GL_FLOAT, nullptr);
	// set the cubemap texture parameters
	// set wrapping to clamp to edge to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// set filtering to linear and enable mipmaps for the cubemap
	// (i.e., use trilinear filtering when sampling the cubemap with mipmaps)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// view/projection matrices for capturing data onto the 6 cubemap face directions
	glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{ // 6 view matrices for the 6 faces of the cubemap (right, left, top, bottom, front, back)
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	if (!m_skyboxVAO) InitSkyboxCube(); // initialize the skybox cube if not done yet

	// activate the equirectangular to cubemap shader and set uniforms for conversion
	m_equirectangularToCubemapShader->Use();
	m_equirectangularToCubemapShader->SetInt("equirectMap", 0);
	m_equirectangularToCubemapShader->SetMat4("projection", captureProj);

	// bind the HDR equirectangular map to texture unit 0 for the shader to sample from
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox->GetTextureId());

	// save current viewport so it can be restored later
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);
	glViewport(0, 0, captureSize, captureSize); // set viewport to the capture dimensions

	// render to each face of the cubemap by attaching it to the FBO and rendering the scene
	for (unsigned int i = 0; i < 6; ++i)
	{ // for each of the 6 faces of the cubemap, render the scene using the capture view
		m_equirectangularToCubemapShader->SetMat4("view", captureViews[i]);
		// attach the face of the cubemap texture to the correponding FBO color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the necessary buffers
		glBindVertexArray(m_skyboxVAO); // bind the skybox VAO
		glDrawElements(GL_TRIANGLES, nSkyboxIndices, GL_UNSIGNED_INT, 0); // render the skybox cube
	}

	// generate mipmaps for the cubemap texture to enable trilinear filtering
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// restore previous FBO
	glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
	// restore original viewport
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

	// replace skybox with new cubemap texture
	GLuint oldHDR = skybox->GetTextureId(); // store old HDR texture id for deletion after replacement
	sceneManager->SetSkybox(std::make_shared<Texture>("Skybox Cubemap From HDR",
													  envCubemap, TextureType::CUBEMAP));
	// delete old HDR texture as it is no longer needed
	glDeleteTextures(1, &oldHDR);
	m_hdrToCubemapConverted = true; // mark as converted to avoid redundant conversions

	std::cout << "[SUCCESS::RENDERER::ConvertHDRToCubemapIfNeeded] Converted HDR (src texId = "
		<< m_hdrSourceTexId << ") to cubemap (texId = " << envCubemap << ")" << std::endl;

	// reset source id so a new HDR selection triggers fresh conversion logic properly
	m_hdrSourceTexId = envCubemap; // the active skybox is now the cubemap
}