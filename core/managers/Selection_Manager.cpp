/*
 * Selection_Manager.cpp
 * Implements Selection_Manager for single-object selection via color picking.
 */

#include "Selection_Manager.h"

#include "Node_Manager.h"
#include "../Core.h"
#include "../Node.h"
#include "../shader/Shader.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/Directional_Light.h"
#include "../light/Point_Light.h"
#include "../light/Spotlight.h"
#include "../renderer/Renderer.h"

// Constructor
// -----------
Selection_Manager::Selection_Manager() : width{ 0 }, height{ 0 }, selected_node_id{ 0 } {}

// Destructor
// ----------
Selection_Manager::~Selection_Manager()
{
	// deallocate resources for each render pass
	picking_pass.deallocate_resources();
	outline_pass.deallocate_resources();
}

// Public Methods
// --------------
void Selection_Manager::queue_pick(GLdouble mouse_x, GLdouble mouse_y, GLsizei window_width, GLsizei window_height)
{
	// convert from windowing API's top-left to OpenGL bottom-left coordinates
	GLint ogl_y = static_cast<GLint>(window_height - 1 - mouse_y);
	// store pending pick (i.e., do not process immediately, wait for frame start)
	pending_pick = glm::ivec2(static_cast<GLint>(mouse_x), ogl_y);
}

void Selection_Manager::process_pending_pick(const Camera* camera, Node_Manager* node_manager)
{
	// if there is no pending pick, no camera, or no node manager, return
	if (!pending_pick.has_value() || !camera || !node_manager)
		return;
	if (!picking_shader)
	{ // if no picking shader is set, print a warning and return
		std::cerr << "[WARNING::SELECTIONMANAGER::process_pending_pick] Picking shader not set" << std::endl;
		return;
	}
	if (width == 0 || height == 0)
		return; // if the window is zero-sized, return

	// ensure FBO exists (just in case resize was not called yet)
	if (picking_pass.get_fbo_id() == 0)
		ensure_picking_pass();

	// previous selection state (for change detection and logging)
	std::uint32_t         previous_selected_id = selected_node_id;
	std::shared_ptr<Node> previous_node;
	std::string           previous_selected_name;
	if (previous_selected_id != 0)
	{ // if there was a previous selection, get its name for logging
		previous_node = find_node_by_id(node_manager, previous_selected_id);
		if (previous_node) // if the node still exists, get its name
			previous_selected_name = previous_node->get_name();
	}
	// check if previous selection was outline-eligible, i.e. a real model (not a gizmo)
	bool previous_outline_eligible = is_outline_eligible(previous_node);

	picking_pass.bind(); // bind picking FBO
	// ensure sRGB transform does not corrupt id encoding (if enabled elsewhere)
	GLboolean s_rgb_was_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (s_rgb_was_enabled)
		glDisable(GL_FRAMEBUFFER_SRGB);
	// disable culling to avoid missing backfacing geometry during picking
	GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);
	if (cull_was_enabled)
		glDisable(GL_CULL_FACE);
	// enable depth testing for correct occlusion during picking
	glEnable(GL_DEPTH_TEST);
	// clear color and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black id (0) means no selection
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute view and projection matrices from the camera
	glm::mat4 projection =
	    glm::perspective(glm::radians(camera->get_zoom()), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
	glm::mat4 view = camera->get_view_matrix();

	// render every model once; encode each node's own id (no root promotion)
	auto& model_nodes = node_manager->get_nodes(Node_Type::MODEL);

	picking_shader->use();
	picking_shader->set_mat4("u_view", view);
	picking_shader->set_mat4("u_projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	std::unordered_set<std::uint32_t> visited; // track processed root models to avoid duplicates
	for (const auto& node : model_nodes)
	{ // iterate through all models in the scene
		// dynamically cast the node to a Model object
		auto model_node = std::dynamic_pointer_cast<Node>(node);
		if (!model_node)
			continue; // skip if not a model
		if (visited.count(model_node->get_id()))
			continue; // skip if already visited

		// draw this node and all its descendants with their own ids
		std::vector<std::shared_ptr<Node>> stack{ model_node };
		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& current = stack.back();
			stack.pop_back();
			if (!current)
				continue; // skip null nodes
			if (!current->get_is_visible())
				continue; // skip invisible nodes

			visited.insert(current->get_id()); // mark this node as visited

			// set model matrix and encoded id uniform, then draw the model
			picking_shader->set_mat4("u_model", current->get_world_model_matrix());
			picking_shader->set_int("u_encoded_id", static_cast<GLint>(current->get_id()));
			current->draw(*picking_shader);

			// push child models onto the stack for processing
			for (const auto& child : current->get_children()) stack.push_back(child);
		}
	}

	// read the pixel that is currently under the mouse cursor
	GLint         px        = (*pending_pick).x;
	GLint         py        = (*pending_pick).y;
	std::uint32_t picked_id = read_pixel_id(px, py);

	picking_pass.unbind(); // unbind FBO after rendering
	if (cull_was_enabled)
		glEnable(GL_CULL_FACE); // restore culling state if needed
	if (s_rgb_was_enabled)
		glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
	pending_pick.reset();              // clear pending pick

	// get current time for cycle-up logic
	double current_time = static_cast<double>(Core::get_instance()->get_renderer()->get_time());

	// a picked id of 0 means no selection,
	// i.e. the user clicked on empty space or there was no previous selection
	if (picked_id == 0)
	{ // if no object was picked, log deselection (if any) and clear selection if needed
		if (previous_selected_id != 0)
		{ // if there was a previous selection, log its deselection
			std::cout << "[INFO::SELECTIONMANAGER::process_pending_pick] Deselected node '" << previous_selected_name << "' (id "
			          << previous_selected_id << ")" << std::endl;
		}
		clear_selection(); // ensures stale outline mask cannot persist
		return;
	}

	// if the user clicked on an object, find the corresponding node by id
	auto picked_node = find_node_by_id(node_manager, picked_id);
	if (!picked_node)
	{ // if no node with that id exists, print a warning and clear selection (also clears outline mask)
		std::cout << "[WARNING::SELECTIONMANAGER::process_pending_pick] No node with texture_id " << picked_id << std::endl;
		clear_selection(); // ensures stale outline mask cannot persist
		return;
	}

	Node_Type picked_node_type = picked_node->get_type();
	// if a gizmo model was picked, resolve it to its owning light and bypass cycle-up
	if ((picked_node_type == Node_Type::COMPOSITE_MODEL || picked_node_type == Node_Type::COMPOSITE_ASSIMP_MODEL ||
	     picked_node_type == Node_Type::COMPOSITE_SHAPE_MODEL || picked_node_type == Node_Type::ASSIMP_MODEL ||
	     picked_node_type == Node_Type::SHAPE_MODEL) // any model type
	    && (std::dynamic_pointer_cast<Node>(picked_node)->get_gizmo_type() != Gizmo_Type::NONE))
	{
		if (auto resolved = resolve_gizmo_to_light(node_manager, picked_node))
			picked_node = resolved; // switch to the owning light node
		// reset cycle state when a gizmo is selected
		last_picked_id = 0;
		last_pick_time = 0.0;
	}
	// cycle-up selection for models (non-gizmos)
	else if (
	    picked_node_type == Node_Type::COMPOSITE_MODEL || picked_node_type == Node_Type::COMPOSITE_ASSIMP_MODEL ||
	    picked_node_type == Node_Type::COMPOSITE_SHAPE_MODEL || picked_node_type == Node_Type::ASSIMP_MODEL ||
	    picked_node_type == Node_Type::SHAPE_MODEL) // any model type
	{                                               // if the picked node is a model, check for cycle-up conditions
		auto model_comp       = std::dynamic_pointer_cast<Node>(picked_node);
		bool same_as_last     = (last_picked_id == model_comp->get_id());
		bool within_threshold = (current_time - last_pick_time) <= CYCLE_TIME_THRESHOLD;

		if (same_as_last && within_threshold)
		{ // if picking the same model within the threshold, climb to parent if any
			if (auto parent = model_comp->get_parent())
			{ // if there is a parent, switch selection to it
				picked_node = parent;
				// update cycle state to the new parent to allow further climbing
				last_picked_id = parent->get_id();
				last_pick_time = current_time;
			}
		}
		else
		{ // if picking a different model or outside the threshold, reset cycle state to the current node
			last_picked_id = model_comp->get_id();
			last_pick_time = current_time;
		}
	}
	else
	{
		// reset cycle state if not a model
		last_picked_id = 0;
		last_pick_time = 0.0;
	}

	// check if the newly picked node is outline-eligible, i.e. a real model (not a gizmo)
	bool new_outline_eligible = is_outline_eligible(picked_node);

	// update only if changed
	if (picked_node->get_id() != previous_selected_id)
	{ // if the selection changed, update the selected node id, clear outline if needed, and log the change
		selected_node_id = picked_node->get_id();

		// clear outline mask when switching from an outline-eligible node to a non-eligible one
		if (previous_outline_eligible && !new_outline_eligible)
			clear_outline_mask();

		if (previous_selected_id != 0)
		{ // if switching from another selection, print implicit deselection as well
			std::cout << "[INFO::SELECTIONMANAGER::process_pending_pick] Deselected node '" << previous_selected_name << "' (id "
			          << previous_selected_id << ")" << std::endl;
		}
		// print info about the new selection
		std::cout << "[INFO::SELECTIONMANAGER::process_pending_pick] Selected node '" << picked_node->get_name() << "' (id "
		          << selected_node_id << ")" << std::endl;
	}
	// clicking same selected node leads to no logging or state change
}

void Selection_Manager::render_picking_visualization(const Camera* camera, Node_Manager* node_manager)
{
	// if shader, camera, or node manager are missing, return
	if (!camera || !picking_shader || !node_manager)
		return;
	if (width == 0 || height == 0)
		return;                         // if the window is zero-sized, return
	if (picking_pass.get_fbo_id() == 0) // ensure FBO exists
		ensure_picking_pass();

	picking_pass.bind(); // bind picking FBO
	// disable sRGB transform to avoid corrupting id encoding (if enabled elsewhere)
	GLboolean s_rgb_was_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (s_rgb_was_enabled)
		glDisable(GL_FRAMEBUFFER_SRGB);
	// disable culling to avoid missing backfacing geometry during picking
	GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);
	if (cull_was_enabled)
		glDisable(GL_CULL_FACE);
	// enable depth testing for correct occlusion during picking
	glEnable(GL_DEPTH_TEST);
	// clear color and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black id (0) means no selection
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// compute view and projection matrices from the camera
	glm::mat4 projection =
	    glm::perspective(glm::radians(camera->get_zoom()), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
	glm::mat4 view = camera->get_view_matrix();

	// render all selectable models (including gizmos - lights themselves are not drawn; their gizmos are)
	auto& model_nodes = node_manager->get_nodes(Node_Type::MODEL);
	picking_shader->use();
	picking_shader->set_mat4("u_view", view);
	picking_shader->set_mat4("u_projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for picking

	std::unordered_set<std::uint32_t> visited; // track processed models to avoid duplicates
	for (const auto& node : model_nodes)
	{ // iterate through all models in the scene
		// dynamically cast the node to a Model object
		auto model_node = std::dynamic_pointer_cast<Node>(node);
		if (!model_node)
			continue; // skip if not a model
		if (visited.count(node->get_id()))
			continue; // skip if already visited

		// DFS to draw this node and all its descendants with their own ids
		std::vector<std::shared_ptr<Node>> stack{ model_node };
		while (!stack.empty())
		{ // perform DFS to process all child models in the hierarchy
			auto& current = stack.back();
			stack.pop_back();
			if (!current)
				continue; // skip null nodes
			if (!current->get_is_visible())
				continue; // skip invisible nodes

			visited.insert(current->get_id()); // mark this node as visited

			// set model matrix and encoded id uniform, then draw the model
			picking_shader->set_mat4("u_model", current->get_world_model_matrix());
			picking_shader->set_int("u_encoded_id", static_cast<GLint>(current->get_id()));
			current->draw(*picking_shader);

			// push child models onto the stack for processing
			for (const auto& child : current->get_children()) stack.push_back(child);
		}
	}

	picking_pass.unbind(); // unbind FBO after rendering
	if (cull_was_enabled)
		glEnable(GL_CULL_FACE); // restore culling state if needed
	if (s_rgb_was_enabled)
		glEnable(GL_FRAMEBUFFER_SRGB); // restore sRGB state if needed
}

std::shared_ptr<Node> Selection_Manager::get_selected_node(Node_Manager* node_manager) const
{
	if (selected_node_id == 0)
		return nullptr; // no selection, return null
	// find and return the selected node by id (may be null if id is invalid)
	return find_node_by_id(node_manager, selected_node_id);
}

void Selection_Manager::clear_selection()
{
	// if there was a selection, clear it
	if (selected_node_id != 0)
		selected_node_id = 0;
	clear_outline_mask(); // explicit mask clear to prevent stale outline persistence
}

void Selection_Manager::delete_selected(Node_Manager* node_manager)
{
	if (selected_node_id == 0)
		return; // no selection, nothing to delete

	// find the selected node by id and check validity, if invalid clear selection and return
	auto node = find_node_by_id(node_manager, selected_node_id);
	if (!node)
	{
		clear_selection();
		return;
	}

	if (node->get_type() == Node_Type::CAMERA)
	{ // prevent deletion of cameras for now
		std::cout << "[INFO::SELECTIONMANAGER::delete_selected] Cameras cannot be deleted for now "
		             "(id "
		          << selected_node_id << ")" << std::endl;
		return;
	}

	// if deleting an outlined model, clear outline before removal (avoid one-frame ghost)
	bool wasOutlineEligible = is_outline_eligible(node);
	if (wasOutlineEligible)
		clear_outline_mask();

	// remove the selected node itself, print info, and clear selection
	node_manager->remove_node_by_id(node->get_id()); // use the node manager to remove the node
	std::cout << "[INFO::SELECTIONMANAGER::delete_selected] Selection (id " << selected_node_id << ") deleted" << std::endl;
	selected_node_id = 0; // clear selection after deletion (0 means none)
}

void Selection_Manager::resize(GLuint width, GLuint height)
{
	// if the window size is zero, return
	if (width == 0 || height == 0)
		return;

	bool size_changed = this->width != width || this->height != height; // flag for size change

	// if the window size is unchanged and the FBOs are already created, return
	if (!size_changed && picking_pass.get_fbo_id() != 0 && outline_pass.get_fbo_id() != 0)
		return;

	// update internal width and height
	this->width  = width;
	this->height = height;

	// recreate the picking pass with an updated specification and print info
	{
		Render_Pass_Specification spec{};
		spec.width                  = width;
		spec.height                 = height;
		spec.color_attachment_count = 1;     // single channel for id encoding
		spec.has_depth_attachment   = true;  // need depth for correct occlusion
		spec.has_stencil_attachment = false; // not needed for picking pass
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.depth_as_texture = true;

		picking_pass.create(spec); // create or recreate the picking pass

		// prevent inaccurate id sampling when reading back by using GL_NEAREST filtering
		if (GLuint texId = picking_pass.get_texture_id(0); texId != 0)
		{
			glBindTexture(GL_TEXTURE_2D, picking_pass.get_texture_id(0));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		std::cout << "[INFO::SELECTIONMANAGER::resize] Picking pass resized to " << width << "x" << height << std::endl;
	}

	// recreate the outline pass with an updated specification and print info
	{
		Render_Pass_Specification spec{};
		spec.width                  = width;
		spec.height                 = height;
		spec.color_attachment_count = 1;     // single channel for mask
		spec.has_depth_attachment   = true;  // need depth for correct occlusion
		spec.has_stencil_attachment = false; // not needed for outline pass
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.depth_as_texture = true;

		outline_pass.create(spec); // create or recreate the outline pass

		// prevent edge shifts and bleeding by using GL_NEAREST filtering
		if (GLuint texId = outline_pass.get_texture_id(0); texId != 0)
		{
			glBindTexture(GL_TEXTURE_2D, outline_pass.get_texture_id(0));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		std::cout << "[INFO::SELECTIONMANAGER::resize] Outline pass resized to " << width << "x" << height << std::endl;
	}
}

GLuint Selection_Manager::get_picking_texture_id() const
{
	// if the picking FBO is not created, return 0
	if (picking_pass.get_fbo_id() == 0)
		return 0;
	// otherwise, return the texture id of the first color attachment (0 if invalid index)
	return picking_pass.get_texture_id(0);
}

void Selection_Manager::render_outline_mask(const Camera* camera, Node_Manager* node_manager)
{
	if (selected_node_id == 0)
		return; // no selection, nothing to outline, return
	if (!camera || !node_manager)
		return; // if no camera or node manager, return
	if (width == 0 || height == 0)
		return; // if the window is zero-sized, return

	// find the selected node; if it vanished, clear mask (avoid ghost) and return
	auto selected = find_node_by_id(node_manager, selected_node_id);
	if (!selected)
	{ // if the selected node no longer exists, clear mask (if any) and return
		clear_outline_mask();
		return;
	}

	if (selected->get_type() == Node_Type::LIGHT)
	{ // if the selected node is a light with a missing or invisible gizmo, no outline should be rendered
		// dynamically cast the node to a Light object
		auto light = static_cast<Light*>(selected.get());
		auto gizmo = light->get_gizmo(); // may be null if no gizmo exists
		if (!gizmo || !gizmo->get_is_visible())
		{ // if no gizmo or invisible, clear mask and return
			clear_outline_mask();
			return;
		}
	}

	if (!is_outline_eligible(selected))
	{ // if current selection is not outline eligible (light / gizmo), clear mask (if any) and return
		clear_outline_mask();
		return;
	}

	// ensure FBO exists
	if (outline_pass.get_fbo_id() == 0)
		ensure_outline_pass();

	// reuse picking shader for geometry submission with encodedId = 1 (mask)
	if (!picking_shader)
	{ // if no picking shader is set, print a warning and return
		std::cerr << "[WARNING::SELECTIONMANAGER::render_outline_mask] Picking shader not set;\n"
		             "cannot build outline mask"
		          << std::endl;
		return;
	}

	// before binding, preserve sRGB state and store previous clear color and viewport
	// to avoid introducing rendering artifacts downstream
	GLboolean s_rgb_was_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (s_rgb_was_enabled)
		glDisable(GL_FRAMEBUFFER_SRGB);
	GLfloat prevClearColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, prevClearColor);
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	// bind outline FBO, set state, and clear buffers
	outline_pass.bind();
	// disable culling to avoid missing backfacing geometry during outline mask rendering
	GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);
	if (cull_was_enabled)
		glDisable(GL_CULL_FACE);
	// enable depth testing for correct occlusion
	glEnable(GL_DEPTH_TEST);
	// clear color and depth buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black means no outline (mask = 0)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// determine geometry to render for outline mask (light -> gizmo model, model -> itself)
	std::shared_ptr<Node> model;
	if (selected->get_type() == Node_Type::LIGHT)
	{ // if the selected node is a light, get its gizmo model
		// dynamically cast the node to a Light object
		auto light = static_cast<Light*>(selected.get());
		model      = light->get_gizmo(); // may be null if no gizmo exists
	}
	else if (
	    selected->get_type() == Node_Type::COMPOSITE_MODEL || selected->get_type() == Node_Type::COMPOSITE_ASSIMP_MODEL ||
	    selected->get_type() == Node_Type::COMPOSITE_SHAPE_MODEL || selected->get_type() == Node_Type::ASSIMP_MODEL ||
	    selected->get_type() == Node_Type::SHAPE_MODEL) // any model type
	{                                                   // if the selected node is a model, use it directly
		model = std::dynamic_pointer_cast<Node>(selected);
	}

	if (!model)
	{
		clear_outline_mask();
		return;
	} // no model to outline, clear mask and return

	// compute view and projection matrices from the camera
	glm::mat4 projection =
	    glm::perspective(glm::radians(camera->get_zoom()), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
	glm::mat4 view = camera->get_view_matrix();

	// render entire hierarchy of the selected root model with a constant mask value of 1
	picking_shader->use();
	picking_shader->set_mat4("u_view", view);
	picking_shader->set_mat4("u_projection", projection);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure solid fill for outline mask

	// DFS traversal to render selected model and its children (no climb to root)
	std::vector<std::shared_ptr<Node>> stack;
	stack.push_back(model); // start from the selected model
	while (!stack.empty())
	{ // perform DFS to process all child models in the hierarchy
		auto& current = stack.back();
		stack.pop_back();
		if (!current)
			continue; // skip null nodes

		// set model matrix and encoded id uniform, then draw the model
		picking_shader->set_mat4("u_model", current->get_world_model_matrix());
		picking_shader->set_int("u_encoded_id", 1); // constant mask value of 1 for outline
		current->draw(*picking_shader);

		// push child models onto the stack for processing
		for (auto& child : current->get_children()) stack.push_back(child);
	}

	outline_pass.unbind(); // unbind FBO after rendering

	// after unbinding, restore prior GL state, i.e. sRGB, clear color, and viewport
	if (cull_was_enabled)
		glEnable(GL_CULL_FACE); // restore culling state if needed
	if (s_rgb_was_enabled)
		glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3]);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

GLuint Selection_Manager::get_outline_mask_texture_id() const
{
	if (outline_pass.get_fbo_id() == 0)
		return 0;                          // if the outline FBO is not created, return 0
	return outline_pass.get_texture_id(0); // otherwise, return the texture id of the first color attachment
}

// Private Methods
// ---------------
void Selection_Manager::ensure_picking_pass()
{
	// if the picking FBO is already created and valid, return
	if (picking_pass.get_fbo_id() != 0)
		return;

	// create the picking FBO with the current window size among other specs and print info
	Render_Pass_Specification spec{};
	spec.width                  = width;
	spec.height                 = height;
	spec.color_attachment_count = 1;
	spec.has_depth_attachment   = true;
	spec.has_stencil_attachment = false; // not needed for picking pass
	// enable depth texture as it is needed later for depth-aware outline composite
	// (to sample the selected object's depth in the main scene)
	spec.depth_as_texture = true;

	picking_pass.create(spec); // create or recreate the picking pass

	// prevent inaccurate id sampling when reading back by using GL_NEAREST filtering
	if (GLuint texId = picking_pass.get_texture_id(0); texId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::cout << "[INFO::SELECTIONMANAGER::ensure_picking_pass] Picking pass created with dimensions (" << width << "x" << height << ")"
	          << std::endl;
}

void Selection_Manager::ensure_outline_pass()
{
	// if the outline FBO is already created and valid, return
	if (outline_pass.get_fbo_id() != 0)
		return;

	// create the outline FBO with the current window size among other specs and print info
	Render_Pass_Specification spec{};
	spec.width                  = width;
	spec.height                 = height;
	spec.color_attachment_count = 1;
	spec.has_depth_attachment   = true;
	spec.has_stencil_attachment = false;
	// enable depth texture as it is needed later for depth-aware outline composite
	// (to sample the selected object's depth in the main scene)
	spec.depth_as_texture = true;

	outline_pass.create(spec); // create or recreate the outline pass

	// prevent edge shifts and bleeding by using GL_NEAREST filtering
	if (GLuint texId = outline_pass.get_texture_id(0); texId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::cout << "[INFO::SELECTIONMANAGER::ensure_outline_pass] Outline pass created with dimensions (" << width << "x" << height << ")"
	          << std::endl;
}

void Selection_Manager::clear_outline_mask()
{
	// if the outline FBO does not exist yet there is nothing to clear, return
	if (outline_pass.get_fbo_id() == 0)
		return;

	// preserve depth test enable state to avoid introducing rendering artifacts downstream
	GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

	// before binding, preserve sRGB state and store previous clear color and viewport
	// to avoid introducing rendering artifacts downstream
	GLboolean s_rgb_was_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	if (s_rgb_was_enabled)
		glDisable(GL_FRAMEBUFFER_SRGB);
	GLfloat prevClearColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, prevClearColor);
	GLint prevViewport[4];
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	// bind outline FBO, disable depth testing for full clear, and clear color and depth buffers
	outline_pass.bind();
	glDisable(GL_DEPTH_TEST);             // not needed for a full clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // fully transparent/black mask (no outline)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	outline_pass.unbind();

	// restore prior depth state
	if (depthWasEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// after unbinding, restore prior GL state, i.e. sRGB, clear color, and viewport
	if (s_rgb_was_enabled)
		glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3]);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

bool Selection_Manager::is_outline_eligible(const std::shared_ptr<Node>& node) const
{
	if (!node)
		return false; // null node is not outline-eligible

	// check node type for outline eligibility
	if (node->get_type() == Node_Type::COMPOSITE_ASSIMP_MODEL || node->get_type() == Node_Type::COMPOSITE_SHAPE_MODEL ||
	    node->get_type() == Node_Type::COMPOSITE_MODEL || node->get_type() == Node_Type::ASSIMP_MODEL ||
	    node->get_type() == Node_Type::SHAPE_MODEL) // any model type
	{                                               // outline any non-gizmo model
		auto model = dynamic_cast<Node*>(node.get());
		if (!model)
			return false;
		// outline any non-gizmo model
		return model->get_gizmo_type() == Gizmo_Type::NONE;
	}
	else if (node->get_type() == Node_Type::LIGHT)
	{ // outline lights only via their gizmo if present
		auto light = static_cast<Light*>(node.get());
		// outline via gizmo geometry if present
		return light && light->get_gizmo() != nullptr;
	}

	return false; // other node types are not outline-eligible
}

std::uint32_t Selection_Manager::read_pixel_id(GLint x, GLint y) const
{
	// if the coordinates are out of bounds, return 0 (no selection)
	if (x < 0 || y < 0 || x >= static_cast<GLint>(width) || y >= static_cast<GLint>(height))
		return 0;

	// read the pixel at (x, y) from the picking FBO's color attachment
	unsigned char data[4]{ 0, 0, 0, 0 }; // RGBA
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	std::uint32_t id =
	    static_cast<std::uint32_t>(data[0]) | (static_cast<std::uint32_t>(data[1]) << 8) | (static_cast<std::uint32_t>(data[2]) << 16);
	return id; // return the decoded id from RGB
}

std::shared_ptr<Node> Selection_Manager::find_node_by_id(Node_Manager* node_manager, std::uint32_t id) const
{
	for (auto type : { Node_Type::CAMERA, Node_Type::LIGHT, Node_Type::MODEL })
	{ // iterate over node types to search (models and lights)
		// search all nodes of the current type for a matching id
		for (const auto& node : node_manager->get_nodes(type))
			if (node && node->get_id() == id) // if found, return the node
				return node;
	}
	return nullptr; // if not found, return null
}

std::shared_ptr<Node> Selection_Manager::resolve_gizmo_to_light(Node_Manager* node_manager, const std::shared_ptr<Node>& gizmoModel) const
{
	if (!gizmoModel)
	{
		std::cerr << "[WARNING::SELECTIONMANAGER::resolve_gizmo_to_light] Invalid gizmo model provided" << std::endl;
		return nullptr;
	}

	// return the parent of the gizmo model, which should be the owning light
	auto parent = gizmoModel->get_parent();
	if (parent && parent->get_type() == Node_Type::LIGHT)
	{
		std::cout << "[INFO::SELECTIONMANAGER::resolve_gizmo_to_light] Resolved gizmo model id " << gizmoModel->get_id()
		          << " to owning parent light id " << parent->get_id() << std::endl;
		return parent;
	}

	// if no valid parent light found, print a warning and return null
	std::cerr << "[WARNING::SELECTIONMANAGER::resolve_gizmo_to_light] Could not resolve gizmo model id " << gizmoModel->get_id()
	          << " to an owning parent light" << std::endl;
	return nullptr;
}