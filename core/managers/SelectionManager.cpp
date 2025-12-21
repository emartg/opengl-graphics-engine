/*
* SelectionManager.cpp
* Implements SelectionManager for single-object selection via color picking.
*/

#include "SelectionManager.h"

#include "NodeManager.h"
#include "../Core.h"
#include "../Node.h"
#include "../shader/Shader.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/DirectionalLight.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"
#include "../renderer/Renderer.h"

// Constructor
// -----------
SelectionManager::SelectionManager() : m_width{ 0 }, m_height{ 0 }, m_selectedNodeId{ 0 } {}

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

void SelectionManager::ProcessPendingPick(const Camera* camera, NodeManager* nodeManager)
{
	// if there is no pending pick, no camera, or no node manager, return
	if (!m_pendingPick.has_value() || !camera || !nodeManager) return;
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
	std::uint32_t previousSelectedId = m_selectedNodeId;
	std::shared_ptr<Node> previousNode;
	std::string previousSelectedName;
	if (previousSelectedId != 0)
	{ // if there was a previous selection, get its name for logging
		previousNode = findNodeById(nodeManager, previousSelectedId);
		if (previousNode) // if the node still exists, get its name
			previousSelectedName = previousNode->GetName();
	}
	// check if previous selection was outline-eligible, i.e. a real model (not a gizmo)
	bool previousOutlineEligible = isOutlineEligible(previousNode);

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

	// render every model once; encode each node's own id (no root promotion)
	auto& modelNodes = nodeManager->GetNodes(NodeType::MODEL);

	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	std::unordered_set<std::uint32_t> visited; // track processed root models to avoid duplicates
	for (const auto& node : modelNodes)
	{ // iterate through all models in the scene
		// dynamically cast the node to a Model object
		auto modelNode = std::dynamic_pointer_cast<Node>(node);
		if (!modelNode) continue; // skip if not a model
		if (visited.count(modelNode->GetId())) continue; // skip if already visited

		// draw this node and all its descendants with their own ids
		std::vector<std::shared_ptr<Node>> stack{ modelNode };
		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& current = stack.back();
			stack.pop_back();
			if (!current) continue; // skip null nodes

			visited.insert(current->GetId()); // mark this node as visited

			// set model matrix and encoded id uniform, then draw the model
			m_pickingShader->SetMat4("model", current->GetWorldModelMatrix());
			m_pickingShader->SetInt("encodedId", static_cast<GLint>(current->GetId()));
			current->Draw(*m_pickingShader);

			// push child models onto the stack for processing
			for (const auto& child : current->GetChildren()) stack.push_back(child);
		}
	}

	// read the pixel that is currently under the mouse cursor
	GLint px = (*m_pendingPick).x;
	GLint py = (*m_pendingPick).y;
	std::uint32_t pickedId = readPixelId(px, py);

	m_pickingPass.Unbind(); // unbind FBO after rendering
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
	m_pendingPick.reset(); // clear pending pick

	// get current time for cycle-up logic
	double currentTime = static_cast<double>(Core::GetInstance()->GetRenderer()->GetTime());

	// a picked id of 0 means no selection, 
	// i.e. the user clicked on empty space or there was no previous selection
	if (pickedId == 0)
	{ // if no object was picked, log deselection (if any) and clear selection if needed
		if (previousSelectedId != 0)
		{ // if there was a previous selection, log its deselection
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected node "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		ClearSelection(); // ensures stale outline mask cannot persist
		return;
	}

	// if the user clicked on an object, find the corresponding node by id
	auto pickedNode = findNodeById(nodeManager, pickedId);
	if (!pickedNode)
	{ // if no node with that id exists, print a warning and clear selection (also clears outline mask)
		std::cout << "[WARNING::SELECTIONMANAGER::ProcessPendingPick] No node with textureId "
			<< pickedId << std::endl;
		ClearSelection(); // ensures stale outline mask cannot persist
		return;
	}

	NodeType pickedNodeType = pickedNode->GetNodeType();
	// if a gizmo model was picked, resolve it to its owning light and bypass cycle-up
	if ((pickedNodeType == NodeType::COMPOSITE_MODEL ||
		 pickedNodeType == NodeType::COMPOSITE_ASSIMP_MODEL ||
		 pickedNodeType == NodeType::COMPOSITE_SHAPE_MODEL ||
		 pickedNodeType == NodeType::ASSIMP_MODEL ||
		 pickedNodeType == NodeType::SHAPE_MODEL) // any model type
		&& (std::dynamic_pointer_cast<Node>(pickedNode)->GetGizmoType() != GizmoType::NONE))
	{
		if (auto resolved = resolveGizmoToLight(nodeManager, pickedNode))
			pickedNode = resolved; // switch to the owning light node
		// reset cycle state when a gizmo is selected
		m_lastPickedId = 0;
		m_lastPickTime = 0.0;
	}
	// cycle-up selection for models (non-gizmos)
	else if (pickedNodeType == NodeType::COMPOSITE_MODEL ||
			 pickedNodeType == NodeType::COMPOSITE_ASSIMP_MODEL ||
			 pickedNodeType == NodeType::COMPOSITE_SHAPE_MODEL ||
			 pickedNodeType == NodeType::ASSIMP_MODEL ||
			 pickedNodeType == NodeType::SHAPE_MODEL) // any model type
	{ // if the picked node is a model, check for cycle-up conditions
		auto modelComp = std::dynamic_pointer_cast<Node>(pickedNode);
		bool sameAsLast = (m_lastPickedId == modelComp->GetId());
		bool withinThreshold = (currentTime - m_lastPickTime) <= CYCLE_TIME_THRESHOLD;

		if (sameAsLast && withinThreshold)
		{ // if picking the same model within the threshold, climb to parent if any
			if (auto parent = modelComp->GetParent())
			{ // if there is a parent, switch selection to it
				pickedNode = parent;
				// update cycle state to the new parent to allow further climbing
				m_lastPickedId = parent->GetId();
				m_lastPickTime = currentTime;
			}
		}
		else
		{ // if picking a different model or outside the threshold, reset cycle state to the current node
			m_lastPickedId = modelComp->GetId();
			m_lastPickTime = currentTime;
		}
	}
	else
	{
		// reset cycle state if not a model
		m_lastPickedId = 0;
		m_lastPickTime = 0.0;
	}

	// check if the newly picked node is outline-eligible, i.e. a real model (not a gizmo)
	bool newOutlineEligible = isOutlineEligible(pickedNode);

	// update only if changed
	if (pickedNode->GetId() != previousSelectedId)
	{ // if the selection changed, update the selected node id, clear outline if needed, and log the change
		m_selectedNodeId = pickedNode->GetId();

		// clear outline mask when switching from an outline-eligible node to a non-eligible one
		if (previousOutlineEligible && !newOutlineEligible)
			clearOutlineMask();

		if (previousSelectedId != 0)
		{ // if switching from another selection, print implicit deselection as well
			std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Deselected node "
				<< previousSelectedName << " (ID " << previousSelectedId << ")" << std::endl;
		}
		// print info about the new selection
		std::cout << "[INFO::SELECTIONMANAGER::ProcessPendingPick] Selected node "
			<< pickedNode->GetName() << " (ID " << m_selectedNodeId << ")" << std::endl;
	}
	// clicking same selected node leads to no logging or state change
}

void SelectionManager::RenderPickingVisualization(const Camera* camera, NodeManager* nodeManager)
{
	// if shader, camera, or node manager are missing, return
	if (!camera || !m_pickingShader || !nodeManager) return;
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
	auto& modelNodes = nodeManager->GetNodes(NodeType::MODEL);
	m_pickingShader->Use();
	m_pickingShader->SetMat4("view", view);
	m_pickingShader->SetMat4("projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	std::unordered_set<std::uint32_t> visited; // track processed models to avoid duplicates
	for (const auto& node : modelNodes)
	{ // iterate through all models in the scene
		// dynamically cast the node to a Model object
		auto modelNode = std::dynamic_pointer_cast<Node>(node);
		if (!modelNode) continue; // skip if not a model
		if (visited.count(node->GetId())) continue; // skip if already visited

		// DFS to draw this node and all its descendants with their own ids
		std::vector<std::shared_ptr<Node>> stack{ modelNode };
		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& current = stack.back();
			stack.pop_back();
			if (!current) continue; // skip null nodes

			visited.insert(current->GetId()); // mark this node as visited

			// set model matrix and encoded id uniform, then draw the model
			m_pickingShader->SetMat4("model", current->GetWorldModelMatrix());
			m_pickingShader->SetInt("encodedId", static_cast<GLint>(current->GetId()));
			current->Draw(*m_pickingShader);

			// push child models onto the stack for processing
			for (const auto& child : current->GetChildren()) stack.push_back(child);
		}
	}

	m_pickingPass.Unbind(); // unbind FBO after rendering
	if (sRGBWasEnabled) glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
}

std::shared_ptr<Node> SelectionManager::GetSelectedNode(NodeManager* nodeManager) const
{
	if (m_selectedNodeId == 0) return nullptr; // no selection, return null
	// find and return the selected node by id (may be null if id is invalid)
	return findNodeById(nodeManager, m_selectedNodeId);
}

void SelectionManager::ClearSelection()
{
	// if there was a selection, clear it
	if (m_selectedNodeId != 0) m_selectedNodeId = 0;
	clearOutlineMask(); // explicit mask clear to prevent stale outline persistence
}

void SelectionManager::DeleteSelected(NodeManager* nodeManager)
{
	if (m_selectedNodeId == 0) return; // no selection, nothing to delete

	// find the selected node by id and check validity, if invalid clear selection and return
	auto node = findNodeById(nodeManager, m_selectedNodeId);
	if (!node) { ClearSelection(); return; }

	// if deleting an outlined model, clear outline before removal (avoid one-frame ghost)
	bool wasOutlineEligible = isOutlineEligible(node);
	if (wasOutlineEligible)
		clearOutlineMask();

	// remove the selected node itself, print info, and clear selection
	nodeManager->RemoveNodeById(node->GetId()); // use the node manager to remove the node
	std::cout << "[INFO::SELECTIONMANAGER::DeleteSelected] Selection (ID " << m_selectedNodeId << ") deleted"
		<< std::endl;
	m_selectedNodeId = 0; // clear selection after deletion (0 means none)
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

void SelectionManager::RenderOutlineMask(const Camera* camera, NodeManager* nodeManager)
{
	if (m_selectedNodeId == 0) return; // no selection, nothing to outline, return
	if (!camera || !nodeManager) return; // if no camera or node manager, return
	if (m_width == 0 || m_height == 0) return; // if the window is zero-sized, return

	// find the selected node; if it vanished, clear mask (avoid ghost) and return
	auto selected = findNodeById(nodeManager, m_selectedNodeId);
	if (!selected)
	{ // if the selected node no longer exists, clear mask (if any) and return
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

	// determine geometry to render for outline mask (light -> gizmo model, model -> itself)
	std::shared_ptr<Node> model;
	if (selected->GetNodeType() == NodeType::LIGHT)
	{ // if the selected node is a light, get its gizmo model
		// dynamically cast the node to a Light object
		auto light = static_cast<Light*>(selected.get());
		model = light->GetGizmo(); // may be null if no gizmo exists
	}
	else if (selected->GetNodeType() == NodeType::COMPOSITE_MODEL ||
			 selected->GetNodeType() == NodeType::COMPOSITE_ASSIMP_MODEL ||
			 selected->GetNodeType() == NodeType::COMPOSITE_SHAPE_MODEL ||
			 selected->GetNodeType() == NodeType::ASSIMP_MODEL ||
			 selected->GetNodeType() == NodeType::SHAPE_MODEL) // any model type
	{ // if the selected node is a model, use it directly
		model = std::dynamic_pointer_cast<Node>(selected);
	}

	if (!model) { clearOutlineMask(); return; } // no model to outline, clear mask and return

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

	// DFS traversal to render selected model and its children (no climb to root)
	std::vector<std::shared_ptr<Node>> stack;
	stack.push_back(model); // start from the selected model
	while (!stack.empty())
	{ // perform DFS to process all child models in the hierarchy
		auto& current = stack.back();
		stack.pop_back();
		if (!current) continue; // skip null nodes

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

bool SelectionManager::isOutlineEligible(const std::shared_ptr<Node>& node) const
{
	if (!node) return false; // null node is not outline-eligible

	// check node type for outline eligibility
	if (node->GetNodeType() == NodeType::COMPOSITE_ASSIMP_MODEL ||
		node->GetNodeType() == NodeType::COMPOSITE_SHAPE_MODEL ||
		node->GetNodeType() == NodeType::COMPOSITE_MODEL ||
		node->GetNodeType() == NodeType::ASSIMP_MODEL ||
		node->GetNodeType() == NodeType::SHAPE_MODEL) // any model type
	{ // outline any non-gizmo model
		auto model = dynamic_cast<Node*>(node.get());
		if (!model) return false;
		// outline any non-gizmo model
		return model->GetGizmoType() == GizmoType::NONE;
	}
	else if (node->GetNodeType() == NodeType::LIGHT)
	{ // outline lights only via their gizmo if present
		auto light = static_cast<Light*>(node.get());
		// outline via gizmo geometry if present
		return light && light->GetGizmo() != nullptr;
	}

	return false; // other node types are not outline-eligible
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

std::shared_ptr<Node> SelectionManager::findNodeById(NodeManager* nodeManager, std::uint32_t id) const
{
	for (auto type : { NodeType::MODEL, NodeType::LIGHT })
	{ // iterate over node types to search (models and lights)
		// search all nodes of the current type for a matching id
		for (const auto& node : nodeManager->GetNodes(type))
			if (node && node->GetId() == id) // if found, return the node
				return node;
	}
	return nullptr; // if not found, return null
}

std::shared_ptr<Node> SelectionManager::resolveGizmoToLight(NodeManager* nodeManager,
															const std::shared_ptr<Node>& gizmoModel) const
{
	if (!gizmoModel)
	{
		std::cerr << "[WARNING::SELECTIONMANAGER::resolveGizmoToLight] Invalid gizmo model provided"
			<< std::endl;
		return nullptr;
	}

	// return the parent of the gizmo model, which should be the owning light
	auto parent = gizmoModel->GetParent();
	if (parent && parent->GetNodeType() == NodeType::LIGHT)
	{
		std::cout << "[INFO::SELECTIONMANAGER::resolveGizmoToLight] Resolved gizmo model ID "
			<< gizmoModel->GetId() << " to owning parent light ID " << parent->GetId() << std::endl;
		return parent;
	}

	// if no valid parent light found, print a warning and return null
	std::cerr << "[WARNING::SELECTIONMANAGER::resolveGizmoToLight] Could not resolve gizmo model ID "
		<< gizmoModel->GetId() << " to an owning parent light" << std::endl;
	return nullptr;
}