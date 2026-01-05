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

#include "SKYBOX.h" // skybox vertex data
#include "SCREEN_QUAD.h" // screen-quad vertex data
#include "RenderPass.h"
#include "../Core.h"
#include "../Node.h"
#include "../gizmos/Line.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/DirectionalLight.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"
#include "../shader/Shader.h"
#include "../texture/Texture.h"
#include "../managers/NodeManager.h"
#include "../managers/SceneManager.h"
#include "../managers/SelectionManager.h"

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

	// clean up dynamic environment map FBOs, RBOs, and cubemap textures
	for (auto [id, entry] : m_dynamicEnvMaps)
	{
		if (entry.fbo) glDeleteFramebuffers(1, &entry.fbo);
		if (entry.rbo) glDeleteRenderbuffers(1, &entry.rbo);
		if (entry.cubemapTexId) glDeleteTextures(1, &entry.cubemapTexId);
		if (entry.prevCubemapTexId) glDeleteTextures(1, &entry.prevCubemapTexId);
	}
	m_dynamicEnvMaps.clear();

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

	// face culling configuration:
	glEnable(GL_CULL_FACE); // enable face culling
	glCullFace(GL_BACK); // cull back faces (default)
	glFrontFace(GL_CCW); // counter-clockwise wound faces are front faces (default)

	// blending configuration:
	glEnable(GL_BLEND); // enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // standard alpha blending function (default)
	glBlendEquation(GL_FUNC_ADD); // standard blend equation (default)
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
	else if (strcmp(name.c_str(), "Reflective Shader") == 0)
		m_reflectiveShader = shader;
	else if (strcmp(name.c_str(), "Refractive Shader") == 0)
		m_refractiveShader = shader;
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

	auto core = Core::GetInstance();
	auto& sceneManager = core->GetSceneManager();
	auto& camera = sceneManager->GetCamera();

	// perform all pre-render passes (cubemap generation, dynamic reflections, etc.)
	// before binding the main render pass and setting up the main camera

	// ensure the skybox is a valid cubemap texture early (needed for environment mapping)
	// and update per-object dynamic environment maps (if any)
	if (sceneManager->GetSkybox()) ConvertHDRToCubemapIfNeeded();
	UpdateDynamicEnvMaps();

	// process pending picking request before main scene rendering
	core->GetSelectionManager()->ProcessPendingPick(camera.get(), core->GetNodeManager().get());

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

	// get the node manager, scene manager, and camera from the core instance
	auto& nodeManager = core->GetNodeManager();
	auto& sceneManager = core->GetSceneManager();
	auto& camera = sceneManager->GetCamera();

	// ensure camera and shaders are valid before proceeding
	if (!camera || !m_untexturedMattShapeShader || !m_assimpModelShader || !m_singleAlbedoShader
		|| !m_screenShader || !m_pickingShader || !m_skyboxShader || !m_equirectangularToCubemapShader
		|| !m_reflectiveShader || !m_refractiveShader)
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

	// transpose of the upper-left 3x3 submatrix of the view matrix, used for environment mapping
	// (i.e., the inverse matrix of the rotation part of the view matrix)
	glm::mat3 invViewRot = glm::transpose(glm::mat3(view));

	// set the view and projection matrices for each shader program
	auto& shaders = nodeManager->GetNodes(NodeType::SHADER);
	for (const auto& node : shaders)
	{
		// dynamically cast the node to a Shader object
		auto shader = std::dynamic_pointer_cast<Shader>(node);

		shader->Use();
		shader->SetMat4("view", view);
		shader->SetMat4("projection", projection);
	}

	// set constant material uniforms
	m_untexturedMattShapeShader->Use();
	m_untexturedMattShapeShader->SetFloat("material.shininess", 32.0f); // shininess factor for the material
	m_assimpModelShader->Use();
	m_assimpModelShader->SetInt("material.albedoMap", 0);		// set albedo map to texture unit 0
	m_assimpModelShader->SetInt("material.metallicMap", 1);		// set metallic map to texture unit 1
	m_assimpModelShader->SetFloat("material.shininess", 32.0f); // set shininess factor for the material

	// set constant uniforms for the reflective and refractive shaders
	m_reflectiveShader->Use();
	m_reflectiveShader->SetMat3("invViewRot", invViewRot); // set inverse view rotation matrix
	m_reflectiveShader->SetInt("skybox", 0); // set skybox texture unit to 0
	m_refractiveShader->Use();
	m_refractiveShader->SetMat3("invViewRot", invViewRot); // set inverse view rotation matrix
	m_refractiveShader->SetFloat("ratio", 1.00f / 1.52f); // air to glass refraction index ratio
	m_refractiveShader->SetInt("skybox", 0); // set skybox texture unit to 0

	// set light uniforms
	auto& lights = nodeManager->GetNodes(NodeType::LIGHT);
	GLint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{};
	std::for_each(lights.begin(), lights.end(),
				  [&](const std::shared_ptr<Node>& node)
	{
		// dynamically cast the node to a Light object
		auto light = dynamic_cast<Light*>(node.get());

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
	auto& models = nodeManager->GetNodes("MODEL");
	for (const auto& node : models)
	{
		// dynamically cast the node to a Node object
		auto model = std::dynamic_pointer_cast<Node>(node);
		if (!model) continue; // if the cast fails, skip to the next node

		if (model->GetParent()) continue; // skip child models (they are rendered by their parent composite)

		RenderModel(model); // render the node
	}

	// disable face culling for light gizmos to ensure they are always visible
	glDisable(GL_CULL_FACE);

	// set rendering mode to wireframe for light gizmos
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// iterate over the vector of lights and render their gizmo children (if any)
	for (const auto& node : lights)
	{
		// dynamically cast the node to a Light object
		auto light = std::dynamic_pointer_cast<Light>(node);
		if (!light) continue; // if the cast fails, skip to the next node

		// set up the single albedo shader for rendering light gizmos
		m_singleAlbedoShader->Use();
		m_singleAlbedoShader->SetMat4("projection", projection);
		m_singleAlbedoShader->SetMat4("view", view);

		auto gizmo = light->GetGizmo();
		if (gizmo)
		{

			// set the model matrix and albedo color for the light gizmo
			m_singleAlbedoShader->SetMat4("model", gizmo->GetModelMatrix());
			// use light's diffuse color for the gizmo (to match light color)
			m_singleAlbedoShader->SetVec3("albedo", light->GetDiffuse());

			gizmo->Draw(); // draw the light gizmo

			// for directional lights, also render the direction line as part of the gizmo
			if (light->GetLightType() == LightType::DIRECTIONAL_LIGHT)
			{
				auto directionalLight = std::dynamic_pointer_cast<DirectionalLight>(light);
				if (directionalLight)
				{
					auto& directionLine = directionalLight->GetGizmoDirectionLine();
					if (directionLine)
					{
						// set the model matrix and albedo color for the direction line
						// identity matrix for the line (as it does not need any further transformation)
						m_singleAlbedoShader->SetMat4("model", glm::mat4(1.0f));
						// use light's diffuse color for the direction line (to match light color)
						m_singleAlbedoShader->SetVec3("albedo", directionalLight->GetDiffuse());

						directionLine->Draw(); // draw the directional light direction line
					}
				}
			}

		}
	}
	// reset rendering mode to fill after rendering the light gizmos
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// re-enable face culling after rendering the light gizmos
	glEnable(GL_CULL_FACE);

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
				"Skybox still HDR (conversion pending)" << std::endl;
		}
	}

	// after the scene is rendered offscreen, composite to the default framebuffer
	CompositeToScreen();
}

void Renderer::FrameEndConfig() const { SwapBuffers(); }

void Renderer::RegisterModelForDynamicEnvMapCapture(std::uint32_t modelId, GLuint resolution)
{
	auto& entry = m_dynamicEnvMaps[modelId]; // get or create the dynamic env map entry
	entry.resolution = resolution; // set the resolution for the dynamic env map
	entry.hasPrevCubemap = false; // initially, there is no valid previous cubemap

	std::cout << "[INFO::RENDERER::RegisterModelForDynamicEnvMapCapture] "
		"Registered dynamic environment map for model id " << modelId
		<< " with resolution " << resolution << std::endl;
}

void Renderer::UnregisterModelForDynamicEnvMapCapture(std::uint32_t modelId)
{
	auto it = m_dynamicEnvMaps.find(modelId); // find the dynamic env map entry by model id
	if (it == m_dynamicEnvMaps.end()) return; // if not found, return

	auto& entry = it->second; // get the dynamic env map entry
	entry.hasPrevCubemap = false; // explicitly mark previous cubemap as invalid before deletion
	// delete the FBO, RBO, and cubemap texture associated with the dynamic env map entry if they exist
	if (entry.fbo) glDeleteFramebuffers(1, &entry.fbo);
	if (entry.rbo) glDeleteRenderbuffers(1, &entry.rbo);
	if (entry.cubemapTexId) glDeleteTextures(1, &entry.cubemapTexId);
	m_dynamicEnvMaps.erase(it); // remove the entry from the map

	std::cout << "[INFO::RENDERER::UnregisterModelForDynamicEnvMapCapture] "
		"Unregistered dynamic environment map for model id " << modelId << std::endl;
}

// Protected Methods
// -----------------
void Renderer::RenderModel(const std::shared_ptr<Node>& model)
{
	if (!model)
	{ // if the model is null, print an error message and return
		std::cerr << "[ERROR::RENDERER::RenderModel] Node is null" << std::endl;
		return;
	}

	// auxiliary shared pointer to the shader program used for rendering the model and its children (if any)
	std::shared_ptr<Shader> renderShader;

	// switch based on the gizmo type to:
	// - set the current shader program accordingly for rendering
	// - set the appropiate properties for the model for rendering
	// - set the polygon mode (fill or line) for rendering
	switch (model->GetGizmoType())
	{
		case GizmoType::NONE: // if the model is not a gizmo
		{
			switch (model->GetNodeType()) // use the appropriate shader based on the model type
			{
				case NodeType::COMPOSITE_MODEL: // basic composite models do not have their own meshes
				{
					// assign a default shader for composite models without meshes
					renderShader = m_singleAlbedoShader;
					renderShader->Use(); // activate the current shader program

					// set a default albedo color (e.g., gray) for composite models without meshes
					renderShader->SetVec3("albedo", glm::vec3(0.25f, 0.25f, 0.25f));
				}
				break;
				case NodeType::COMPOSITE_ASSIMP_MODEL: // composite Assimp models have their own meshes
				case NodeType::ASSIMP_MODEL:
				{
					renderShader = m_assimpModelShader; // use the Assimp model shader
					renderShader->Use(); // activate the current shader program
				}
				break;
				case NodeType::COMPOSITE_SHAPE_MODEL: // composite shape models have their own meshes
				case NodeType::SHAPE_MODEL:
				{
					renderShader = m_untexturedMattShapeShader; // use the untextured matt shape shader
					renderShader->Use(); // activate the current shader program
					// set the color of the shape based on the model's albedo
					renderShader->SetVec3("material.albedo", model->GetAlbedo());
				}
				break;
				default:
					// if the model type is unknown, print an error message and return
					std::cerr << "[ERROR::RENDERER::RenderModel] Unknown model type for " << model->GetName()
						<< std::endl;
					return;
			}

			// set the polygon mode to fill for regular models
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		}
		break;
		case GizmoType::DIRECTIONAL_LIGHT: // if the model is a directional light gizmo
		case GizmoType::POINT_LIGHT: // if the model is a point light gizmo
		case GizmoType::SPOTLIGHT: // if the model is a spotlight gizmo
		{
			renderShader = m_singleAlbedoShader; // use the single albedo shader
			renderShader->Use(); // activate the current shader program
			// set the color of the gizmo shape based on the model's albedo
			renderShader->SetVec3("albedo", model->GetAlbedo());

			// set the polygon mode to line for light gizmos
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		break;
		default: // if the gizmo type is unknown, print an error message and return
			std::cerr << "[ERROR::RENDERER::RenderModel] Unknown gizmo type for model: " << model->GetName()
				<< std::endl;
			return;
	}

	if (renderShader)
	{ // if a valid shader program is set, proceed to render the model and its children (if any)
		// set the model matrix for the current model (hierarchical world transformation)
		renderShader->SetMat4("model", model->GetWorldModelMatrix());
		// draw the model using its Draw method, passing the current shader program
		model->Draw(*renderShader);
		// recursively render each child model
		for (const auto& childModel : model->GetChildren()) RenderModel(childModel);
	}
}

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
	if (m_screenDebugParams.debugMode <= 1 && selectionManager->GetSelectedNodeId() != 0)
	{
		// if debug mode is either Normal mode (1) or Inverted Colors mode (3), and there is a selection,
		// render the outline mask
		auto camera = core->GetSceneManager()->GetCamera();
		selectionManager->RenderOutlineMask(camera.get(), core->GetNodeManager().get());
	}
	else if (m_screenDebugParams.debugMode == 2)
	{
		// if debug mode is the Picking Colors mode (2), render picking visualization
		auto camera = core->GetSceneManager()->GetCamera();
		selectionManager->RenderPickingVisualization(camera.get(), core->GetNodeManager().get());
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
	// there is only and outline if there is a selected node and the outline mask texture is available
	const bool hasOutline =
		selectionManager->GetSelectedNodeId() != 0 && selectionManager->GetOutlineMaskTextureId() != 0;
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

	// disable face culling for skybox rendering since camera is inside the cube
	// looking at the interior (back) faces. This ensures the skybox is rendered correctly, 
	// and avoids winding order ambiguity when rendering from the inside
	glDisable(GL_CULL_FACE);

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

	// re-enable face culling after skybox rendering
	glEnable(GL_CULL_FACE);

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
	for (unsigned int i{}; i < 6; ++i) // allocate space for the 6 faces of the cubemap
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
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),		// +X
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),		// -X
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),		// +Y
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),		// -Y
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),		// +Z
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))		// -Z
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

void Renderer::UpdateDynamicEnvMaps()
{
	// if already capturing or no dynamic env maps, return
	if (m_isCapturingDynamicEnvMap || m_dynamicEnvMaps.empty()) return;

	auto core = Core::GetInstance(); // get the core instance
	auto& NodeManager = core->GetNodeManager(); // get the node manager
	auto& models = NodeManager->GetNodes(NodeType::MODEL); // get the models from the node manager
	if (models.empty()) return; // if no models, return

	// ensure a skybox cubemap exists for the dynamic env map captures
	auto& skyboxTexture = core->GetSceneManager()->GetSkybox();
	if (skyboxTexture) ConvertHDRToCubemapIfNeeded();

	// flip buffers at the start of the update
	// (this makes last frame's "write" buffer -cubemapTexId- this frame's "read" buffer -prevCubemapTexId-)
	for (auto& [id, entry] : m_dynamicEnvMaps)
	{
		std::swap(entry.cubemapTexId, entry.prevCubemapTexId);
		entry.hasPrevCubemap = true; // mark that a valid "previous" cubemap now exists for reading
	}

	// set capturing flag to prevent re-entrance
	m_isCapturingDynamicEnvMap = true;

	// iterate over the models and capture dynamic env maps for those registered
	for (const auto& node : models)
	{
		// dynamically cast the node to a Model object
		auto model = dynamic_cast<Node*>(node.get());
		// if the cast fails or the model has a gizmo, skip to next model
		if (!model || model->GetGizmoType() != GizmoType::NONE) continue;

		// check if the model has a registered dynamic env map
		auto it = m_dynamicEnvMaps.find(model->GetId());
		if (it == m_dynamicEnvMaps.end()) continue; // if not found, skip to next model

		auto& entry = it->second; // get the dynamic env map entry
		// dynamically cast the model to a shared pointer for passing to the capture function
		auto modelPtr = std::dynamic_pointer_cast<Node>(node);
		// capture the dynamic environment map for the model into the "write" buffer (cubemapTexId)
		CaptureDynamicEnvMapForModel(modelPtr, entry);
	}

	m_isCapturingDynamicEnvMap = false; // reset capturing flag after processing all models
}

void Renderer::CaptureDynamicEnvMapForModel(const std::shared_ptr<Node>& model,
											DynamicEnvMapEntry& entry)
{
	// lazy initialization of cubemap texture, FBO, and RBO for the dynamic env map entry
	if (!entry.initialized)
	{
		// current cubemap texture initialization and configuration
		glGenTextures(1, &entry.cubemapTexId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, entry.cubemapTexId);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
						 entry.resolution, entry.resolution, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// previous cubemap texture initialization and configuration (for temporal effects)
		glGenTextures(1, &entry.prevCubemapTexId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, entry.prevCubemapTexId);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
						 entry.resolution, entry.resolution, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// FBO and RBO initialization
		glGenFramebuffers(1, &entry.fbo);
		glGenRenderbuffers(1, &entry.rbo);

		entry.initialized = true; // mark the entry as initialized
		entry.hasPrevCubemap = false; // no previous cubemap yet (no data captured)
	}

	GLint prevFBO{}, prevViewport[4];
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO); // save currently bound FBO
	glGetIntegerv(GL_VIEWPORT, prevViewport); // save current viewport

	glBindFramebuffer(GL_FRAMEBUFFER, entry.fbo); // bind the dynamic env map FBO
	glBindRenderbuffer(GL_RENDERBUFFER, entry.rbo); // bind the RBO
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
						  entry.resolution, entry.resolution); // configure RBO storage
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, entry.rbo); // attach RBO to FBO depth attachment
	glDrawBuffer(GL_COLOR_ATTACHMENT0); // select color attachment 0 for rendering
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{ // validate FBO completeness, if incomplete, print error, restore FBO, and return
		std::cerr << "[ERROR::RENDERER::CaptureDynamicEnvMapForModel] "
			"Dynamic Env Map FBO incomplete for model id " << model->GetId() << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, prevFBO); // restore previous FBO
		return;
	}

	// set up capture projection and views for the 6 cubemap faces
	glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::vec3 pos = model->GetPosition(); // get the model's position for the capture
	glm::mat4 captureViews[] =
	{ // 6 view matrices for the 6 faces of the cubemap (right, left, top, bottom, front, back)
		glm::lookAt(pos, pos + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// +X
		glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// -X
		glm::lookAt(pos, pos + glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),	// +Y
		glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),	// -Y
		glm::lookAt(pos, pos + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// +Z
		glm::lookAt(pos, pos + glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))	// -Z
	};

	glViewport(0, 0, entry.resolution, entry.resolution); // set viewport to capture resolution

	// render the scene from the model's position to each face of the cubemap
	for (GLuint i{}; i < 6; ++i)
	{
		// attach the face of the cubemap texture to the FBO color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, entry.cubemapTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear necessary buffers

		// render the scene for the current capture view, excluding the model itself
		RenderSceneForEnvMapCapture(captureViews[i], captureProj, model);
	}

	// generate mipmaps for the cubemap texture to enable trilinear filtering
	glBindTexture(GL_TEXTURE_CUBE_MAP, entry.cubemapTexId);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// restore previous FBO and viewport
	glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void Renderer::RenderSceneForEnvMapCapture(const glm::mat4& captureView, const glm::mat4& captureProj,
										   const std::shared_ptr<Node>& excludeModel)
{
	auto core = Core::GetInstance(); // get the core instance
	auto& nodeManager = core->GetNodeManager(); // get the node manager
	auto& sceneManager = core->GetSceneManager(); // get the scene manager

	// pre-compute uniforms common to all shaders used for env map capture
	GLfloat refractionIndexRatio = 1.00f / 1.52f; // air to glass refraction index ratio
	glm::mat3 invViewRot = glm::transpose(glm::mat3(captureView)); // inverse of rotation part of view matrix

	// set parameters on shaders that are used for the env map capture
	m_untexturedMattShapeShader->Use();
	m_untexturedMattShapeShader->SetMat4("view", captureView);
	m_untexturedMattShapeShader->SetMat4("projection", captureProj);
	m_untexturedMattShapeShader->SetFloat("material.shininess", 32.0f);

	m_assimpModelShader->Use();
	m_assimpModelShader->SetMat4("view", captureView);
	m_assimpModelShader->SetMat4("projection", captureProj);
	m_assimpModelShader->SetFloat("material.shininess", 32.0f);

	m_reflectiveShader->Use();
	m_reflectiveShader->SetMat4("view", captureView);
	m_reflectiveShader->SetMat4("projection", captureProj);
	m_reflectiveShader->SetMat3("invViewRot", invViewRot);
	m_reflectiveShader->SetInt("skybox", 0);

	m_refractiveShader->Use();
	m_refractiveShader->SetMat4("view", captureView);
	m_refractiveShader->SetMat4("projection", captureProj);
	m_refractiveShader->SetFloat("ratio", refractionIndexRatio);
	m_refractiveShader->SetMat3("invViewRot", invViewRot);
	m_refractiveShader->SetInt("skybox", 0);

	// set light uniforms
	auto& lights = nodeManager->GetNodes(NodeType::LIGHT); // get lights from the node manager
	GLint pointLightIdx{}, spotlightIdx{}, directionalLightIdx{}; // light type indices
	for (const auto& node : lights)
	{ // iterate over the lights and set their parameters in the shaders
		// dynamically cast the node to a Light object
		auto light = dynamic_cast<Light*>(node.get());

		switch (light->GetLightType()) // set light parameters based on light type
		{
			case LightType::DIRECTIONAL_LIGHT:
			{
				auto dl = dynamic_cast<DirectionalLight*>(light);
				std::string prefix = "directionalLights[" + std::to_string(directionalLightIdx) + "].";
				m_untexturedMattShapeShader->Use();
				m_untexturedMattShapeShader->SetVec3(
					"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
					dl->GetDirection()
				);
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", dl->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", dl->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", dl->GetSpecular());
				m_assimpModelShader->Use();
				m_assimpModelShader->SetVec3(
					"directionalLightDir[" + std::to_string(directionalLightIdx) + "]",
					dl->GetDirection()
				);
				m_assimpModelShader->SetVec3(prefix + "ambient", dl->GetAmbient());
				m_assimpModelShader->SetVec3(prefix + "diffuse", dl->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", dl->GetSpecular());
				directionalLightIdx++;
			}
			break;
			case LightType::POINT_LIGHT:
			{
				auto pl = dynamic_cast<PointLight*>(light);
				std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
				m_untexturedMattShapeShader->Use();
				m_untexturedMattShapeShader->SetVec3(
					"pointLightPos[" + std::to_string(pointLightIdx) + "]",
					pl->GetPosition()
				);
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", pl->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", pl->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", pl->GetSpecular());
				m_untexturedMattShapeShader->SetFloat(prefix + "constant", pl->GetConstant());
				m_untexturedMattShapeShader->SetFloat(prefix + "linear", pl->GetLinear());
				m_untexturedMattShapeShader->SetFloat(prefix + "quadratic", pl->GetQuadratic());
				m_assimpModelShader->Use();
				m_assimpModelShader->SetVec3(
					"pointLightPos[" + std::to_string(pointLightIdx) + "]",
					pl->GetPosition()
				);
				m_assimpModelShader->SetVec3(prefix + "ambient", pl->GetAmbient());
				m_assimpModelShader->SetVec3(prefix + "diffuse", pl->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", pl->GetSpecular());
				m_assimpModelShader->SetFloat(prefix + "constant", pl->GetConstant());
				m_assimpModelShader->SetFloat(prefix + "linear", pl->GetLinear());
				m_assimpModelShader->SetFloat(prefix + "quadratic", pl->GetQuadratic());
				pointLightIdx++;
			}
			break;
			case LightType::SPOTLIGHT:
			{
				auto sl = dynamic_cast<Spotlight*>(light);
				std::string prefix = "spotlights[" + std::to_string(spotlightIdx) + "].";
				m_untexturedMattShapeShader->Use();
				m_untexturedMattShapeShader->SetVec3(
					"spotlightPos[" + std::to_string(spotlightIdx) + "]",
					sl->GetPosition()
				);
				m_untexturedMattShapeShader->SetVec3(
					"spotlightDir[" + std::to_string(spotlightIdx) + "]", sl->GetDirection()
				);
				m_untexturedMattShapeShader->SetVec3(prefix + "ambient", sl->GetAmbient());
				m_untexturedMattShapeShader->SetVec3(prefix + "diffuse", sl->GetDiffuse());
				m_untexturedMattShapeShader->SetVec3(prefix + "specular", sl->GetSpecular());
				m_untexturedMattShapeShader->SetFloat(prefix + "constant", sl->GetConstant());
				m_untexturedMattShapeShader->SetFloat(prefix + "linear", sl->GetLinear());
				m_untexturedMattShapeShader->SetFloat(prefix + "quadratic", sl->GetQuadratic());
				m_untexturedMattShapeShader->SetFloat(prefix + "innerCutOff", sl->GetInnerCutOff());
				m_untexturedMattShapeShader->SetFloat(prefix + "outerCutOff", sl->GetOuterCutOff());
				m_assimpModelShader->Use();
				m_assimpModelShader->SetVec3(
					"spotlightPos[" + std::to_string(spotlightIdx) + "]",
					sl->GetPosition()
				);
				m_assimpModelShader->SetVec3(
					"spotlightDir[" + std::to_string(spotlightIdx) + "]", sl->GetDirection()
				);
				m_assimpModelShader->SetVec3(prefix + "ambient", sl->GetAmbient());
				m_assimpModelShader->SetVec3(prefix + "diffuse", sl->GetDiffuse());
				m_assimpModelShader->SetVec3(prefix + "specular", sl->GetSpecular());
				m_assimpModelShader->SetFloat(prefix + "constant", sl->GetConstant());
				m_assimpModelShader->SetFloat(prefix + "linear", sl->GetLinear());
				m_assimpModelShader->SetFloat(prefix + "quadratic", sl->GetQuadratic());
				m_assimpModelShader->SetFloat(prefix + "innerCutOff", sl->GetInnerCutOff());
				m_assimpModelShader->SetFloat(prefix + "outerCutOff", sl->GetOuterCutOff());
				spotlightIdx++;
			}
			break;
			default:
				break;
		}
	}
	// set the number of lights of each type in the shader
	m_untexturedMattShapeShader->Use();
	m_untexturedMattShapeShader->SetInt("nDirectionalLights", directionalLightIdx);
	m_untexturedMattShapeShader->SetInt("nPointLights", pointLightIdx);
	m_untexturedMattShapeShader->SetInt("nSpotlights", spotlightIdx);
	m_assimpModelShader->Use();
	m_assimpModelShader->SetInt("nDirectionalLights", directionalLightIdx);
	m_assimpModelShader->SetInt("nPointLights", pointLightIdx);
	m_assimpModelShader->SetInt("nSpotlights", spotlightIdx);

	// render all models in the scene except the excluded model
	auto& models = nodeManager->GetNodes(NodeType::MODEL); // get models from the node manager
	for (const auto& node : models)
	{ // iterate over the models and render them
		// dynamically cast the node to a Model Component object
		auto model = dynamic_cast<Node*>(node.get());

		// skip excluded model (the one for which we are capturing the env map) and gizmos early
		if (excludeModel && model == excludeModel.get()) continue;
		if (model->GetGizmoType() != GizmoType::NONE) continue;

		// render the model for enviroment map capture, using recursive rendering for composite models
		if (model->IsComposite())
		{ // if the model is composite, render all its child models recursively in a depth-first traversal
			std::vector<std::shared_ptr<Node>> stack{ model->GetChildren() };
			while (!stack.empty())
			{ // process models in the stack until empty
				auto& currentModel = stack.back(); // get the model at the top of the stack
				stack.pop_back(); // remove the model from the stack

				// if the current model is null, excluded, or a gizmo, skip it
				if (!currentModel) continue;
				if (excludeModel && currentModel == excludeModel) continue;
				if (currentModel->GetGizmoType() != GizmoType::NONE) continue;

				if (currentModel->IsComposite())
				{ // if the current model is composite, add its children to the stack for further processing
					auto children = currentModel->GetChildren();
					stack.insert(stack.end(), children.begin(), children.end());
				}

				// leaf model rendering based on the model type (child models)
				std::shared_ptr<Shader> renderShader;

				switch (currentModel->GetNodeType())// use the appropriate shader based on the model type
				{
					case NodeType::COMPOSITE_MODEL: // basic composite models do not have their own meshes
					{
						// assign a default shader for composite models without meshes
						renderShader = m_singleAlbedoShader;
						renderShader->Use(); // activate the current shader program

						// set a default albedo color (e.g., gray) for composite models without meshes
						renderShader->SetVec3("albedo", glm::vec3(0.25f, 0.25f, 0.25f));
					}
					break;
					case NodeType::COMPOSITE_ASSIMP_MODEL: // composite Assimp models have their own meshes
					case NodeType::ASSIMP_MODEL:
					{
						renderShader = m_assimpModelShader; // use the Assimp model shader
						renderShader->Use(); // activate the current shader program
					}
					break;
					case NodeType::COMPOSITE_SHAPE_MODEL: // composite shape models have their own meshes
					case NodeType::SHAPE_MODEL:
					{
						renderShader = m_untexturedMattShapeShader; // use the untextured matt shape shader
						renderShader->Use(); // activate the current shader program
						// set the color of the shape based on the model's albedo
						renderShader->SetVec3("material.albedo", currentModel->GetAlbedo());
					}
					break;
					default:
						// if the model type is unknown, print an error message and return
						std::cerr << "[ERROR::RENDERER::RenderSceneForEnvMapCapture] "
							"Unknown model type for " << currentModel->GetName() << std::endl;
						return;

						// set the polygon mode to fill for regular models
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}

				if (renderShader)
				{ // if a valid shader is set, proceed with rendering
					// set the model matrix uniform (hierarchical world transformation)
					renderShader->SetMat4("model", currentModel->GetWorldModelMatrix());
					// render the current node
					currentModel->Draw(*renderShader);
					// children rendering is handled via the stack, so no recursive call here
				}
			}
		}
		else
		{ // leaf model rendering based on the model type (non-composite models)
			std::shared_ptr<Shader> renderShader;
			switch (model->GetNodeType())// use the appropriate shader based on the model type
			{
				case NodeType::COMPOSITE_MODEL: // basic composite models do not have their own meshes
				{
					// assign a default shader for composite models without meshes
					renderShader = m_singleAlbedoShader;
					renderShader->Use(); // activate the current shader program

					// set a default albedo color (e.g., gray) for composite models without meshes
					renderShader->SetVec3("albedo", glm::vec3(0.25f, 0.25f, 0.25f));
				}
				break;
				case NodeType::COMPOSITE_ASSIMP_MODEL: // composite Assimp models have their own meshes
				case NodeType::ASSIMP_MODEL:
				{
					renderShader = m_assimpModelShader; // use the Assimp model shader
					renderShader->Use(); // activate the current shader program
				}
				break;
				case NodeType::COMPOSITE_SHAPE_MODEL: // composite shape models have their own meshes
				case NodeType::SHAPE_MODEL:
				{
					renderShader = m_untexturedMattShapeShader; // use the untextured matt shape shader
					renderShader->Use(); // activate the current shader program
					// set the color of the shape based on the model's albedo
					renderShader->SetVec3("material.albedo", model->GetAlbedo());
				}
				break;
				default:
					// if the model type is unknown, print an error message and return
					std::cerr << "[ERROR::RENDERER::RenderSceneForEnvMapCapture] "
						"Unknown model type for " << model->GetName() << std::endl;
					return;
			}

			// set the polygon mode to fill for regular models
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// set the model matrix for the model (hierarchical world transformation)
			renderShader->SetMat4("model", model->GetWorldModelMatrix());
			// render the current model
			model->Draw(*renderShader);
		}
	}

	// render the skybox as background if available
	auto& skyboxTexture = sceneManager->GetSkybox();
	if (skyboxTexture && skyboxTexture->GetTextureType() == TextureType::CUBEMAP)
	{ // only render if skybox is a cubemap
		if (!m_skyboxVAO) InitSkyboxCube(); // initialize skybox cube if not done yet
		// render the skybox cube using the skybox shader and the provided view and projection matrices
		// (const_cast is used here to match the function signature)
		RenderSkyboxCube(skyboxTexture, const_cast<glm::mat4&>(captureView),
						 const_cast<glm::mat4&>(captureProj));
	}
}