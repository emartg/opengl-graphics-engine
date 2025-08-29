/*
* SelectionManager.cpp
* Implements SelectionManager for single-object selection via color picking.
*/

#include "SelectionManager.h"

#include "AssetManager.h"
#include "../Core.h"
#include "../model/Model.h"
#include "../shader/Shader.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/DirectionalLight.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"

// Constructor
// -----------
SelectionManager::SelectionManager() : m_width{ 0 }, m_height{ 0 }, m_selectedAssetId{ 0 } {}

// Destructor
// ----------
SelectionManager::~SelectionManager() { m_pickingPass.DeallocateResources(); }

// Public Methods
// --------------
void SelectionManager::QueuePick(GLdouble mouseX, GLdouble mouseY,
								 GLsizei windowWidth, GLsizei windowHeight)
{
	// convert from GLFW top-left to OpenGL bottom-left coordinates
	GLint oglY = static_cast<GLint>(windowHeight - 1 - mouseY);
	// store pending pick (i.e., do not process immediately, wait for frame start)
	m_pendingPick = glm::ivec2(static_cast<GLint>(mouseX), oglY);
}

void SelectionManager::ProcessPendingPick(const Camera* camera, AssetManager* assetManager)
{
	// if there is no pending pick, no camera, or no asset manager, return
	if (!m_pendingPick.has_value() || !camera || !assetManager) return;
	if (!m_pickingShader)
	{ // if no picking shader is set, print a warning and return
		std::cerr << "[WARNING::SELECTIONMANAGER::ProcessPendingPick] Picking shader not set" << std::endl;
		return;
	}

	// if the window is zero-sized, return
	if (m_width == 0 || m_height == 0) return;
	// ensure FBO exists (just in case Resize was not called yet)
	if (m_pickingPass.GetFboId() == 0)
		ensurePickingPass();

	// previous selection state (for change detection and logging)
	std::uint32_t previousSelectedId = m_selectedAssetId;
	std::string previousSelectedName;
	if (previousSelectedId != 0)
	{ // if there was a previous selection, get its name for logging
		if (auto prevAsset = findAssetById(assetManager, previousSelectedId))
			previousSelectedName = prevAsset->GetName();
	}

	// bind picking FBO, enable depth testing for correct occlusion, and clear buffers
	m_pickingPass.Bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black id (0) means no selection
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute view and projection matrices from the camera
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->GetZoom()),
		static_cast<float>(m_width) / static_cast<float>(m_height),
		0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	// render all selectable models (including gizmos - lights themselves are not drawn; their gizmos are)
	auto& assetModels = assetManager->GetAssets(AssetType::MODEL);
	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	for (const auto& asset : assetModels)
	{ // iterate through all models in the scene
		// dynamically cast the asset to a Model object
		auto model = dynamic_cast<Model*>(asset.get());
		if (!model) continue; // skip if not a model

		// encode model's unique id as a color for picking
		std::uint32_t id = model->GetId();

		// set model matrix and encoded id uniform, then draw the model
		m_pickingShader->SetMat4("model", model->GetModelMatrix());
		m_pickingShader->SetInt("encodedId", static_cast<GLint>(id));
		model->Draw(*m_pickingShader);
	}

	// read the pixel that is currently under the mouse cursor
	GLint px = (*m_pendingPick).x;
	GLint py = (*m_pendingPick).y;
	std::uint32_t pickedId = readPixelId(px, py);

	// unbind FBO and clear pending pick
	m_pickingPass.Unbind();
	m_pendingPick.reset();

	// a picked id of 0 means no selection, 
	// i.e. the user clicked on empty space or there was no previous selection
	if (pickedId == 0)
	{ // if no object was picked, clear selection and log deselection if any
		if (previousSelectedId != 0)
		{ // if there was a previous selection, clear it and log the deselection
			m_selectedAssetId = 0; // clear selection
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected asset "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		return;
	}

	// if the user clicked on an object, find the corresponding asset by id
	auto pickedAsset = findAssetById(assetManager, pickedId);
	if (!pickedAsset)
	{ // if no asset with that id exists, print a warning and clear selection
		m_selectedAssetId = 0;
		std::cout << "[WARNING::SELECTIONMANAGER::ProcessPendingPick] No asset with id "
			<< pickedId << std::endl;
		return;
	}

	// if a gizmo model was picked, resolve it to its owning light
	if (pickedAsset->GetType() == AssetType::MODEL)
	{
		auto resolved = resolveGizmoToLight(assetManager, pickedAsset);
		if (resolved) // if it is indeed a gizmo, switch selection to the owning light
			pickedAsset = resolved;
	}

	// only update and print a info message if the selection changed
	if (pickedAsset->GetId() != previousSelectedId)
	{ // if the selection changed, update the selected asset id and log the changes
		m_selectedAssetId = pickedAsset->GetId();
		if (previousSelectedId != 0)
		{ // if switching from another selection, print implicit deselection as well
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected asset "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		// print info about the new selection
		std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Selected asset "
			<< pickedAsset->GetName() << " (ID " << m_selectedAssetId << ")" << std::endl;
	}
	// else clicking the same selected asset: no logging and no state change
}

void SelectionManager::RenderPickingVisualization(const Camera* camera, AssetManager* assetManager)
{
	// if shader, camera, or asset manager are missing, return
	if (!camera || !m_pickingShader || !assetManager) return;
	if (m_width == 0 || m_height == 0) return; // if the window is zero-sized, return
	if (m_pickingPass.GetFboId() == 0) // ensure FBO exists
		ensurePickingPass();

	// bind picking FBO, enable depth testing for correct occlusion, and clear buffers
	m_pickingPass.Bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute view and projection matrices from the camera
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->GetZoom()),
		static_cast<float>(m_width) / static_cast<float>(m_height),
		0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	// render all selectable models (including gizmos - lights themselves are not drawn; their gizmos are)
	auto& assetModels = assetManager->GetAssets(AssetType::MODEL);
	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	for (const auto& asset : assetModels)
	{ // iterate through all models in the scene
		// dynamically cast the asset to a Model object
		auto model = dynamic_cast<Model*>(asset.get());
		if (!model) continue; // skip if not a model

		// set model matrix and encoded id uniform, then draw the model
		m_pickingShader->SetMat4("model", model->GetModelMatrix());
		m_pickingShader->SetInt("encodedId", static_cast<GLint>(model->GetId()));
		model->Draw(*m_pickingShader);
	}

	m_pickingPass.Unbind(); // unbind FBO after rendering
}

std::shared_ptr<Asset> SelectionManager::GetSelectedAsset(AssetManager* assetManager) const
{
	if (m_selectedAssetId == 0) return nullptr; // no selection, return null
	// find and return the selected asset by id (may be null if id is invalid)
	return findAssetById(assetManager, m_selectedAssetId);
}

void SelectionManager::DeleteSelected(AssetManager* assetManager)
{
	if (m_selectedAssetId == 0) return; // no selection, nothing to delete
	// find the selected asset by id, if not found clear selection and return
	auto asset = findAssetById(assetManager, m_selectedAssetId);
	if (!asset) { m_selectedAssetId = 0; return; }

	// if the selected asset is a light, also delete its gizmo model (if any)
	if (asset->GetType() == AssetType::LIGHT)
	{
		// dynamically cast the asset to a Light object
		auto light = static_cast<Light*>(asset.get());
		// if the light has a gizmo model, remove it from the asset manager
		if (auto& gizmo = light->GetGizmo())
			assetManager->RemoveAssetById(gizmo->GetId());
	}

	// remove the selected asset itself and print info
	assetManager->RemoveAssetById(asset->GetId());
	std::cout << "[INFO::SELECTIONMANAGER::DeleteSelected] Deleted selected asset ID "
		<< m_selectedAssetId << std::endl;
	m_selectedAssetId = 0;
}

void SelectionManager::Resize(GLuint width, GLuint height)
{
	// if the window size is zero, return
	if (width == 0 || height == 0) return;
	// if the window size is unchanged and FBO is valid (i.e. already created), return
	if (width == m_width && height == m_height && m_pickingPass.GetFboId() != 0) return;

	m_width = width;
	m_height = height;

	// update the specification, recreate the picking FBO and print info
	RenderPassSpecification spec{};
	spec.Width = m_width;
	spec.Height = m_height;
	spec.ColorAttachmentCount = 1;
	spec.HasDepthAttachment = true;
	spec.HasStencilAttachment = false; // not needed for picking pass

	m_pickingPass.Create(spec);
	std::cout << "[INFO::SELECTIONMANAGER::Resize] Picking pass resized to "
		<< m_width << "x" << m_height << std::endl;
}

GLuint SelectionManager::GetPickingTextureId() const
{
	// if the picking FBO is not created, return 0
	if (m_pickingPass.GetFboId() == 0) return 0;
	// otherwise, return the texture id of the first color attachment (0 if invalid index)
	return m_pickingPass.GetTextureId(0);
}

// Private Methods
// ---------------
void SelectionManager::ensurePickingPass()
{
	// if the picking FBO is already created and valid, return
	if (m_pickingPass.GetFboId() != 0) return;

	// create the picking FBO with the current window size among other specs and print info
	RenderPassSpecification spec{};
	spec.Width = m_width;
	spec.Height = m_height;
	spec.ColorAttachmentCount = 1;
	spec.HasDepthAttachment = true;
	spec.HasStencilAttachment = false; // not needed for picking pass

	m_pickingPass.Create(spec);
	std::cout << "[INFO::SELECTIONMANAGER::ensurePickingPass] Picking pass created with dimensions "
		<< m_width << "x" << m_height << std::endl;
}

std::uint32_t SelectionManager::readPixelId(GLint x, GLint y) const
{
	// if the coordinates are out of bounds, return 0 (no selection)
	if (x < 0 || y < 0 || x >= static_cast<GLint>(m_width) || y >= static_cast<GLint>(m_height))
		return 0;

	// read the pixel at (x, y) from the picking FBO's color attachment
	unsigned char data[4]{ 0, 0, 0, 0 }; // RGBA
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	std::uint32_t id =
		static_cast<std::uint32_t>(data[0]) |
		(static_cast<std::uint32_t>(data[1]) << 8) |
		(static_cast<std::uint32_t>(data[2]) << 16);
	return id; // return the decoded id from RGB
}

std::shared_ptr<Asset> SelectionManager::findAssetById(AssetManager* assetManager,
													   std::uint32_t id) const
{
	for (auto type : { AssetType::MODEL, AssetType::LIGHT })
	{ // iterate over asset types to search (models and lights)
		// search all assets of the current type for a matching id
		for (const auto& asset : assetManager->GetAssets(type))
			if (asset && asset->GetId() == id) // if found, return the asset
				return asset;
	}
	return nullptr; // if not found, return null
}

std::shared_ptr<Asset> SelectionManager::resolveGizmoToLight(AssetManager* assetManager,
															 const std::shared_ptr<Asset>& gizmoModel) const
{
	// a light owns a gizmo model whose pointer id we can match
	for (const auto& asset : assetManager->GetAssets(AssetType::LIGHT))
	{ // iterate through all lights in the scene
		// dynamically cast the asset to a Light object
		auto light = dynamic_cast<Light*>(asset.get());
		if (!light) continue; // skip if not a light

		// if the light's gizmo matches the given model, return the light
		auto& gizmo = light->GetGizmo();
		if (gizmo && gizmo->GetId() == gizmoModel->GetId())
			return asset; // return owning light
	}
	return nullptr; // if no owning light found, return null
}
