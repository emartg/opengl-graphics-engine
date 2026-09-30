/*
 * Selection_Manager.h
 * Manages selection of nodes in the scene via picking passes,
 * and an outlining mask pass for the selected node.
 */

#pragma once

#include <iostream>
#include <unordered_set>
#include <optional>
#include <cstdint>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "../renderer/Render_Pass.h"

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Node;
class Camera;
class Shader;
class Node_Manager;

// Struct to hold outline parameters for the selected node
struct Outline_Params
{
	glm::vec3 color{ 0.0f, 0.95f, 1.0f }; // outline color - highly visible cyan by default
	GLuint    thickness{ 2 };             // outline thickness in pixels - 2 by default (must be >= 1)
};

class Selection_Manager
{
public:
	// Constructor
	// -----------
	Selection_Manager();

	// Destructor
	// ----------
	~Selection_Manager();

	// Public Methods
	// --------------
	// Queues a pick request at the given window coordinates (origin top-left from the windowing API)
	void queue_pick(GLdouble mouse_x, GLdouble mouse_y, GLsizei window_width, GLsizei window_height);

	// Called from the renderer at frame start to perform the queued pick (if any)
	void process_pending_pick(const Camera* camera, Node_Manager* node_manager);

	// Renders (or re-renders) the full picking buffer for visualization (no selection readback)
	void render_picking_visualization(const Camera* camera, Node_Manager* node_manager);

	// Get currently selected node id (0 means none)
	std::uint32_t get_selected_node_id() const { return selected_node_id; }

	// Set the selected node id directly (for programmatic selection, e.g., from GUI)
	void set_selected_node_id(std::uint32_t id) { selected_node_id = id; }

	// Returns a shared_ptr to the currently selected node (may be null)
	std::shared_ptr<Node> get_selected_node(Node_Manager* node_manager) const;

	// Clears the selection and explicitly clears the outline mask
	void clear_selection();

	// Deletes currently selected node (and associated gizmo if a light)
	void delete_selected(Node_Manager* node_manager);

	// Ensure internal picking FBO matches window size
	void resize(GLuint width, GLuint height);

	// Sets picking shader (obtained after compilation)
	void set_picking_shader(const std::shared_ptr<Shader>& shader) { picking_shader = shader; }

	// Returns the texture id of the picking color attachment (0 if unavailable or FBO not created)
	GLuint get_picking_texture_id() const;

	// Renders the outline mask for the selected node (if any)
	void render_outline_mask(const Camera* camera, Node_Manager* node_manager);

	// Returns the texture id of the outline mask color attachment (0 if unavailable or FBO not created)
	GLuint get_outline_mask_texture_id() const;

	// Returns the outline parameters
	const Outline_Params& get_outline_params() const { return outline_params; }

	// Sets the outline parameters
	void set_outline_params(const Outline_Params& params) { outline_params = params; }

	// Gets the depth texture id of the outline pass (for sampling in the main scene), 0 if unavailable
	GLuint get_outline_depth_texture_id() const { return outline_pass.get_depth_texture_id(); }

private:
	// Private Attributes
	// ------------------
	// general selection manager attributes
	GLuint        width, height;
	std::uint32_t selected_node_id;

	// cycle-up selection helpers
	std::uint32_t last_picked_id{ 0 };   // id of last picked node for cycling
	double        last_pick_time{ 0.0 }; // time of last pick

	// picking attributes
	Render_Pass               picking_pass;
	std::shared_ptr<Shader>   picking_shader;
	std::optional<glm::ivec2> pending_pick; // screen coords (OpenGL origin bottom-left)

	// outline attributes
	Render_Pass    outline_pass;   // FBO for rendering the outline mask (single channel via RGBA8)
	Outline_Params outline_params; // parameters for the outline effect

	// Private Methods
	// ---------------
	// Ensures the picking pass is created
	void ensure_picking_pass();

	// Ensures the outline pass is created
	void ensure_outline_pass();

	// Clears the outline mask FBO (used when selection changes to a non-outline-eligible node or is cleared)
	void clear_outline_mask();

	// Returns true if the node should have an outline mask generated (real scene model, not a gizmo)
	bool is_outline_eligible(const std::shared_ptr<Node>& node) const;

	// Reads the pixel id at the given coordinates from the picking FBO
	std::uint32_t read_pixel_id(GLint x, GLint y) const;

	// Finds an node by its id among models and lights
	std::shared_ptr<Node> find_node_by_id(Node_Manager* node_manager, std::uint32_t id) const;

	// If the given node is a gizmo model, resolves it to the owning parent light node
	std::shared_ptr<Node> resolve_gizmo_to_light(Node_Manager* node_manager, const std::shared_ptr<Node>& gizmo_model) const;

	// Private Static Attributes
	// -------------------------
	static constexpr double CYCLE_TIME_THRESHOLD = 0.5; // time threshold in seconds for cycle-up selection
};