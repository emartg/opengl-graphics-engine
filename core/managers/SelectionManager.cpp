/*
* SelectionManager.cpp
* Implements SelectionManager for single-object selection via color picking.
*/

#include "SelectionManager.h"

#include "AssetManager.h"
#include "../Core.h"
#include "../model/ModelComponent.h"
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
SelectionManager::~SelectionManager()
{
	// deallocate resources for each render pass
	m_pickingPass.DeallocateResources();
	m_outlinePass.DeallocateResources();
}

// Public Methods
// --------------
void SelectionManager::QueuePick(GLdouble mouseX, GLdouble mouseY,
								 GLsizei windowWidth, GLsizei windowHeight)
{
	// convert from windowing API's top-left to OpenGL bottom-left coordinates
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
	if (m_width == 0 || m_height == 0) return; // if the window is zero-sized, return

	// ensure FBO exists (just in case Resize was not called yet)
	if (m_pickingPass.GetFboId() == 0)
		ensurePickingPass();

	// previous selection state (for change detection and logging)
	std::uint32_t previousSelectedId = m_selectedAssetId;
	std::shared_ptr<Asset> previousAsset;
	std::string previousSelectedName;
	if (previousSelectedId != 0)
	{ // if there was a previous selection, get its name for logging
		previousAsset = findAssetById(assetManager, previousSelectedId);
		if (previousAsset) // if the asset still exists, get its name
			previousSelectedName = previousAsset->GetName();
	}
	// check if previous selection was outline-eligible, i.e. a real model (not a gizmo)
	bool previousOutlineEligible = isOutlineEligible(previousAsset);

	m_pickingPass.Bind(); // bind picking FBO
	// ensure sRGB transform does not corrupt ID encoding (if enabled elsewhere)
	GLboolean sRGBWasEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (sRGBWasEnabled) glDisable(GL_FRAMEBUFFER_SRGB);
	// enable depth testing for correct occlusion during picking
	glEnable(GL_DEPTH_TEST);
	// clear color and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black id (0) means no selection
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute view and projection matrices from the camera
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->GetZoom()),
		static_cast<float>(m_width) / static_cast<float>(m_height),
		0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	// render hierarchies by unique root ancestor, but still include standalone leaf models (such as gizmos)
	auto& assetModels = assetManager->GetAssets(AssetType::MODEL);

	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	std::unordered_set<std::uint32_t> processedRoots; // track processed root models to avoid duplicates
	for (const auto& asset : assetModels)
	{ // iterate through all models in the scene
		// dynamically cast the asset to a Model object
		auto modelComp = dynamic_cast<ModelComponent*>(asset.get());
		if (!modelComp) continue; // skip if not a model

		// find the ancestor root model (or itself if standalone)
		auto rootModel = modelComp->GetRootParent();
		if (!rootModel) continue; // skip if no valid root model

		// ensure each root model is processed only once
		if (processedRoots.find(rootModel->GetId()) != processedRoots.end())
			continue; // already processed this root model

		// use a stack for depth-first traversal of the model hierarchy,
		// starting from the root model, which represents the selectable asset,
		// and all its children and descendants will be drawn with the same encoded id
		const GLint encondedRootId = static_cast<GLint>(rootModel->GetId());
		// mark this root model as processed
		std::vector<std::shared_ptr<ModelComponent>> stack{ rootModel };

		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& currentModel = stack.back();
			stack.pop_back();
			if (!currentModel) continue;

			// set model matrix and encoded id root uniform, then draw the model
			m_pickingShader->SetMat4("model", currentModel->GetWorldModelMatrix());
			m_pickingShader->SetInt("encodedId", encondedRootId);
			currentModel->Draw(*m_pickingShader);

			// push child models onto the stack for processing
			for (const auto& child : currentModel->GetChildren()) { stack.push_back(child); }
		}
	}

	// read the pixel that is currently under the mouse cursor
	GLint px = (*m_pendingPick).x;
	GLint py = (*m_pendingPick).y;
	std::uint32_t pickedId = readPixelId(px, py);

	m_pickingPass.Unbind(); // unbind FBO after rendering
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
	m_pendingPick.reset(); // clear pending pick

	// a picked id of 0 means no selection, 
	// i.e. the user clicked on empty space or there was no previous selection
	if (pickedId == 0)
	{ // if no object was picked, log deselection (if any) and clear selection if needed
		if (previousSelectedId != 0)
		{ // if there was a previous selection, log its deselection
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected asset "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		ClearSelection(); // ensures stale outline mask cannot persist
		return;
	}

	// if the user clicked on an object, find the corresponding asset by id
	auto pickedAsset = findAssetById(assetManager, pickedId);
	if (!pickedAsset)
	{ // if no asset with that id exists, print a warning and clear selection (also clears outline mask)
		std::cout << "[WARNING::SELECTIONMANAGER::ProcessPendingPick] No asset with textureId "
			<< pickedId << std::endl;
		ClearSelection(); // ensures stale outline mask cannot persist
		return;
	}

	// if a gizmo model was picked, resolve it to its owning light
	if (pickedAsset->GetType() == AssetType::MODEL)
	{
		auto resolved = resolveGizmoToLight(assetManager, pickedAsset);
		if (resolved)
		{ // if it is indeed a gizmo, switch selection to the owning light
			pickedAsset = resolved;
		}
		else
		{ // otherwise, promote any picked model component (child or leaf) to its top-level parent
			if (auto modelComp = std::dynamic_pointer_cast<ModelComponent>(pickedAsset))
			{ // if it is a model component, get its root model (as the actual selectable asset)
				auto rootModel = modelComp->GetRootParent();
				if (rootModel) pickedAsset = rootModel;
			}
		}
	}

	// check if the newly picked asset is outline-eligible, i.e. a real model (not a gizmo)
	bool newOutlineEligible = isOutlineEligible(pickedAsset);

	// update only if changed
	if (pickedAsset->GetId() != previousSelectedId)
	{ // if the selection changed, update the selected asset id, clear outline if needed, and log the change
		m_selectedAssetId = pickedAsset->GetId();

		// if we are transitioning FROM outline-eligible TO non-eligible, clear stale outline mask
		if (previousOutlineEligible && !newOutlineEligible)
			clearOutlineMask();

		if (previousSelectedId != 0)
		{ // if switching from another selection, print implicit deselection as well
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected asset "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		// print info about the new selection
		std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Selected asset "
			<< pickedAsset->GetName() << " (ID " << m_selectedAssetId << ")" << std::endl;
	}
	// clicking same selected asset leads to no logging or state change
}

void SelectionManager::RenderPickingVisualization(const Camera* camera, AssetManager* assetManager)
{
	// if shader, camera, or asset manager are missing, return
	if (!camera || !m_pickingShader || !assetManager) return;
	if (m_width == 0 || m_height == 0) return; // if the window is zero-sized, return
	if (m_pickingPass.GetFboId() == 0) // ensure FBO exists
		ensurePickingPass();

	m_pickingPass.Bind(); // bind picking FBO
	// disable sRGB transform to avoid corrupting ID encoding (if enabled elsewhere)
	GLboolean sRGBWasEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (sRGBWasEnabled) glDisable(GL_FRAMEBUFFER_SRGB);
	// enable depth testing for correct occlusion during picking
	glEnable(GL_DEPTH_TEST);
	// clear color and depth buffers
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

	std::unordered_set<std::uint32_t> processedRoots; // track processed root models to avoid duplicates
	for (const auto& asset : assetModels)
	{ // iterate through all models in the scene
		// dynamically cast the asset to a Model object
		auto modelComp = dynamic_cast<ModelComponent*>(asset.get());
		if (!modelComp) continue; // skip if not a model

		// find the ancestor root model (or itself if standalone)
		auto rootModel = modelComp->GetRootParent();
		if (!rootModel) continue; // skip if no valid root model

		// ensure each root model is processed only once
		if (processedRoots.find(rootModel->GetId()) != processedRoots.end())
			continue; // already processed this root model

		// use a stack for depth-first traversal of the model hierarchy,
		// starting from the root model, which represents the selectable asset,
		// and all its children and descendants will be drawn with the same encoded id
		const GLint encondedRootId = static_cast<GLint>(rootModel->GetId());
		// mark this root model as processed
		std::vector<std::shared_ptr<ModelComponent>> stack{ rootModel };

		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& currentModel = stack.back();
			stack.pop_back();
			if (!currentModel) continue;

			// set model matrix and encoded id root uniform, then draw the model
			m_pickingShader->SetMat4("model", currentModel->GetWorldModelMatrix());
			m_pickingShader->SetInt("encodedId", encondedRootId);
			currentModel->Draw(*m_pickingShader);

			// push child models onto the stack for processing
			for (const auto& child : currentModel->GetChildren()) { stack.push_back(child); }
		}
	}

	m_pickingPass.Unbind(); // unbind FBO after rendering
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
}

std::shared_ptr<Asset> SelectionManager::GetSelectedAsset(AssetManager* assetManager) const
{
	if (m_selectedAssetId == 0) return nullptr; // no selection, return null
	// find and return the selected asset by id (may be null if id is invalid)
	return findAssetById(assetManager, m_selectedAssetId);
}

void SelectionManager::ClearSelection()
{
	// if there was a selection, clear it
	if (m_selectedAssetId != 0) m_selectedAssetId = 0;
	clearOutlineMask(); // explicit mask clear to prevent stale outline persistence
}

void SelectionManager::DeleteSelected(AssetManager* assetManager)
{
	if (m_selectedAssetId == 0) return; // no selection, nothing to delete

	// find the selected asset by id and check validity, if invalid clear selection and return
	auto asset = findAssetById(assetManager, m_selectedAssetId);
	if (!asset) { ClearSelection(); return; }

	// if deleting an outlined model, clear outline before removal (avoid one-frame ghost)
	bool wasOutlineEligible = isOutlineEligible(asset);
	if (wasOutlineEligible)
		clearOutlineMask();

	if (asset->GetType() == AssetType::LIGHT)
	{ // if the selected asset is a light, also remove its gizmo model (if any)
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

	bool sizeChanged = width != m_width || height != m_height; // flag for size change

	// if the window size is unchanged and the FBOs are already created, return
	if (!sizeChanged && m_pickingPass.GetFboId() != 0 && m_outlinePass.GetFboId() != 0)
		return;

	// update internal width and height
	m_width = width;
	m_height = height;

	// recreate the picking pass with an updated specification and print info
	{
		RenderPassSpecification spec{};
		spec.Width = m_width;
		spec.Height = m_height;
		spec.ColorAttachmentCount = 1;		// single channel for id encoding
		spec.HasDepthAttachment = true;		// need depth for correct occlusion
		spec.HasStencilAttachment = false;	// not needed for picking pass
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.DepthAsTexture = true;

		m_pickingPass.Create(spec);			// create or recreate the picking pass

		// prevent inaccurate id sampling when reading back by using GL_NEAREST filtering
		if (GLuint texId = m_pickingPass.GetTextureId(0); texId != 0)
		{
			glBindTexture(GL_TEXTURE_2D, m_pickingPass.GetTextureId(0));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		std::cout << "[INFO::SELECTIONMANAGER::Resize] Picking pass resized to "
			<< m_width << "x" << m_height << std::endl;
	}

	// recreate the outline pass with an updated specification and print info
	{
		RenderPassSpecification spec{};
		spec.Width = m_width;
		spec.Height = m_height;
		spec.ColorAttachmentCount = 1;		// single channel for mask
		spec.HasDepthAttachment = true;		// need depth for correct occlusion
		spec.HasStencilAttachment = false;	// not needed for outline pass
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.DepthAsTexture = true;

		m_outlinePass.Create(spec);			// create or recreate the outline pass

		// prevent edge shifts and bleeding by using GL_NEAREST filtering
		if (GLuint texId = m_outlinePass.GetTextureId(0); texId != 0)
		{
			glBindTexture(GL_TEXTURE_2D, m_outlinePass.GetTextureId(0));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		std::cout << "[INFO::SELECTIONMANAGER::Resize] Outline pass resized to "
			<< m_width << "x" << m_height << std::endl;
	}
}

GLuint SelectionManager::GetPickingTextureId() const
{
	// if the picking FBO is not created, return 0
	if (m_pickingPass.GetFboId() == 0) return 0;
	// otherwise, return the texture id of the first color attachment (0 if invalid index)
	return m_pickingPass.GetTextureId(0);
}

void SelectionManager::RenderOutlineMask(const Camera* camera, AssetManager* assetManager)
{
	if (m_selectedAssetId == 0) return; // no selection, nothing to outline, return
	if (!camera || !assetManager) return; // if no camera or asset manager, return
	if (m_width == 0 || m_height == 0) return; // if the window is zero-sized, return

	// find the selected asset; if it vanished, clear mask (avoid ghost) and return
	auto selected = findAssetById(assetManager, m_selectedAssetId);
	if (!selected)
	{ // if the selected asset no longer exists, clear mask (if any) and return
		clearOutlineMask();
		return;
	}
	if (!isOutlineEligible(selected))
	{ // if current selection is not outline eligible (light / gizmo), clear mask (if any) and return
		clearOutlineMask();
		return;
	}

	// ensure FBO exists
	if (m_outlinePass.GetFboId() == 0) ensureOutlinePass();

	// reuse picking shader for geometry submission with encodedId = 1 (mask)
	if (!m_pickingShader)
	{ // if no picking shader is set, print a warning and return
		std::cerr << "[WARNING::SELECTIONMANAGER::RenderOutlineMask] Picking shader not set;\n"
			"cannot build outline mask" << std::endl;
		return;
	}

	// before binding, preserve sRGB state and store previous clear color and viewport
	// to avoid introducing rendering artifacts downstream
	GLboolean sRGBWasEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (sRGBWasEnabled) glDisable(GL_FRAMEBUFFER_SRGB);
	GLfloat prevClearColor[4]; glGetFloatv(GL_COLOR_CLEAR_VALUE, prevClearColor);
	GLint prevViewport[4]; glGetIntegerv(GL_VIEWPORT, prevViewport);

	// bind outline FBO, enable depth testing for correct occlusion, and clear buffers
	m_outlinePass.Bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black means no outline (mask = 0)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// dynamically cast the selected asset to a Model object
	auto model = dynamic_cast<ModelComponent*>(selected.get());
	if (!model) return; // if the selected asset is not a model, return

	// promote to top-level parent to outline the whole composite
	model = model->GetRootParent().get();

	// compute view and projection matrices from the camera
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->GetZoom()),
		static_cast<float>(m_width) / static_cast<float>(m_height),
		0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	// render entire hierarchy of the selected root model with a constant mask value of 1
	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for outline mask

	// use a stack for depth-first traversal of the model hierarchy,
	// starting from the root model, to draw all its children and descendants
	// with the same encoded id of 1 (mask)
	std::vector<std::shared_ptr<ModelComponent>> stack{ model->GetRootParent() };
	while (!stack.empty())
	{
		auto& current = stack.back();
		stack.pop_back();
		if (!current) continue;

		// set model matrix and encoded id uniform, then draw the model
		m_pickingShader->SetMat4("model", current->GetWorldModelMatrix());
		m_pickingShader->SetInt("encodedId", 1); // constant mask value of 1 for outline
		current->Draw(*m_pickingShader);

		// push child models onto the stack for processing
		for (auto& child : current->GetChildren()) stack.push_back(child);
	}

	m_outlinePass.Unbind(); // unbind FBO after rendering

	// after unbinding, restore prior GL state, i.e. sRGB, clear color, and viewport
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3]);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

GLuint SelectionManager::GetOutlineMaskTextureId() const
{
	if (m_outlinePass.GetFboId() == 0) return 0; // if the outline FBO is not created, return 0
	return m_outlinePass.GetTextureId(0); // otherwise, return the texture id of the first color attachment
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
	// enable depth texture as it is needed later for depth-aware outline composite
	// (to sample the selected object's depth in the main scene)
	spec.DepthAsTexture = true;

	m_pickingPass.Create(spec); // create or recreate the picking pass

	// prevent inaccurate id sampling when reading back by using GL_NEAREST filtering
	if (GLuint texId = m_pickingPass.GetTextureId(0); texId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::cout << "[INFO::SELECTIONMANAGER::ensurePickingPass] Picking pass created with dimensions ("
		<< m_width << "x" << m_height << ")" << std::endl;
}

void SelectionManager::ensureOutlinePass()
{
	// if the outline FBO is already created and valid, return
	if (m_outlinePass.GetFboId() != 0) return;

	// create the outline FBO with the current window size among other specs and print info
	RenderPassSpecification spec{};
	spec.Width = m_width;
	spec.Height = m_height;
	spec.ColorAttachmentCount = 1;
	spec.HasDepthAttachment = true;
	spec.HasStencilAttachment = false;
	// enable depth texture as it is needed later for depth-aware outline composite
	// (to sample the selected object's depth in the main scene)
	spec.DepthAsTexture = true;

	m_outlinePass.Create(spec); // create or recreate the outline pass

	// prevent edge shifts and bleeding by using GL_NEAREST filtering
	if (GLuint texId = m_outlinePass.GetTextureId(0); texId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::cout << "[INFO::SELECTIONMANAGER::ensureOutlinePass] Outline pass created with dimensions ("
		<< m_width << "x" << m_height << ")" << std::endl;
}

void SelectionManager::clearOutlineMask()
{
	// if the outline FBO does not exist yet there is nothing to clear, return
	if (m_outlinePass.GetFboId() == 0) return;

	// preserve depth test enable state to avoid introducing rendering artifacts downstream
	GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

	// before binding, preserve sRGB state and store previous clear color and viewport
	// to avoid introducing rendering artifacts downstream
	GLboolean sRGBWasEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (sRGBWasEnabled) glDisable(GL_FRAMEBUFFER_SRGB);
	GLfloat prevClearColor[4]; glGetFloatv(GL_COLOR_CLEAR_VALUE, prevClearColor);
	GLint prevViewport[4]; glGetIntegerv(GL_VIEWPORT, prevViewport);

	// bind outline FBO, disable depth testing for full clear, and clear color and depth buffers
	m_outlinePass.Bind();
	glDisable(GL_DEPTH_TEST); // not needed for a full clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // fully transparent/black mask (no outline)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_outlinePass.Unbind();

	// restore prior depth state
	if (depthWasEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);

	// after unbinding, restore prior GL state, i.e. sRGB, clear color, and viewport
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3]);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

bool SelectionManager::isOutlineEligible(const std::shared_ptr<Asset>& asset) const
{
	// if no valid asset or not a model, return false
	if (!asset) return false;
	if (asset->GetType() != AssetType::MODEL) return false;

	// dynamically cast the asset to a Model object
	auto model = dynamic_cast<ModelComponent*>(asset.get());
	if (!model) return false; // if cast fails, return false

	// exclude gizmos (light representations) to keep outline only for actual scene geometry
	return model->GetGizmoType() == GizmoType::NONE;
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