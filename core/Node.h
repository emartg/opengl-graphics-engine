/*
* Node.h
* This file defines the Node class, which represents is the abstract base class
* for any every scene graph node (mesh-bearing or composite).
* It integrates child management directly to allow any node to become composite.
*/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // enable experimental features in GLM
#include <glm/gtx/quaternion.hpp> // for quaternion operations

// Forward declarations of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Shader;
class Mesh;
class Texture;

// Enumeration class for different node types (components of the scene graph)
enum class Node_Type
{
	UNDEFINED = 0,
	CAMERA,
	LIGHT,
	MODEL, COMPOSITE_MODEL, ASSIMP_MODEL, COMPOSITE_ASSIMP_MODEL, SHAPE_MODEL, COMPOSITE_SHAPE_MODEL,
	SHADER,
	TEXTURE
};

// Enumeration class that allows to disriminate between different types of gizmos 
// and also indicate if the model is not a gizmo without a boolean flag
enum class Gizmo_Type { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

// public std::enable_shared_from_this to allow shared pointers to 'this' object
class Node : public std::enable_shared_from_this<Node>
{
public:
	// Constructors
	// ------------
	// This constructor initializes the node with a unique id, a name, a node type,
	// and a series of attributes defining its transform and appearance whose values
	// are set to defaults if not specified.
	// The id is based on a static counter that increments each time a new node is created.
	// It is pre-incremented to ensure the first node has id 1, not 0 (since 0 is reserved
	// as a sentinel value for undefined nodes, e.g., when no nodes are selected)
	Node(const std::string& name, const Node_Type type = Node_Type::UNDEFINED,
		 const glm::vec4 albedo = ALBEDO, const glm::vec3 position = POSITION,
		 const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
		 const glm::vec3 forward = FORWARD, const glm::vec3 mesh_forward = FORWARD,
		 const Gizmo_Type gizmo_type = Gizmo_Type::NONE,
		 bool is_draggable = true, bool can_be_parent = true)
		: id{ ++node_count }, name{ name }, type{ type },
		albedo{ albedo }, position{ position }, rotation{ rotation },
		scale{ scale }, forward{ forward }, mesh_forward{ mesh_forward },
		gizmo_type{ gizmo_type }, is_visible{ true },
		is_draggable{ is_draggable }, can_be_parent{ can_be_parent },
		children{} // initialize children vector as empty
	{}

	// Virtual destructor
	// ------------------
	virtual ~Node() = default;

	// Public Functions
	// ----------------
	// Loads the node and its resources, including its children if the node is composite
	virtual void load() { for (const auto& child : children) if (child) child->load(); }

	// Deallocates all the resources of the node, including its children if the node is composite
	virtual void deallocate_resources()
	{
		for (const auto& child : children) if (child) child->deallocate_resources();
	}

	// Draws the node if it is a mesh-bearing node, including its children if the node is composite
	virtual void draw() const = 0;
	// Draws the node if it is a mesh-bearing node, including its children if the node is composite,
	// using the specified shader (binds the textures before drawing)
	virtual void draw(const Shader& shader) const = 0;

	// Getters and Setters
	std::uint32_t get_id() const { return id; }
	const std::string& get_name() const { return name; }
	const Node_Type& get_type() const { return type; }

	void set_id(std::uint32_t id) { this->id = id; }
	void set_name(const std::string& name) { this->name = name; }
	void set_type(const Node_Type& type) { this->type = type; }

	const glm::vec4& get_albedo() const { return albedo; }
	const glm::vec3& get_position() const { return position; }
	const glm::quat& get_rotation() const { return rotation; }
	const glm::vec3 get_rotation_in_euler_angles() const; // returns the rotation as Euler angles in degrees
	const glm::vec3& get_scale() const { return scale; }
	const glm::vec3 get_forward() const; // returns the forward vector in world space and normalized
	const glm::vec3& get_mesh_forward() const { return mesh_forward; }
	const Gizmo_Type& get_gizmo_type() const { return gizmo_type; }
	bool get_is_visible() const { return is_visible; }
	bool get_is_two_sided() const { return is_two_sided; }
	bool get_is_draggable() const { return is_draggable; }
	bool get_can_be_parent() const { return can_be_parent; }

	void set_albedo(const glm::vec4& albedo) { this->albedo = albedo; }
	void set_position(const glm::vec3& position) { this->position = position; }
	void set_rotation(const glm::quat& rotation); // sets the rotation and updates the fwd vtr accordingly
	void set_rotation_in_euler_angles(const glm::vec3 euler_angles_degrees); // from Euler angles in degrees
	void set_scale(const glm::vec3& scale) { this->scale = scale; }
	void set_forward(const glm::vec3& world_forward); // aligns node's fwd vtr to the given world fwd vtr
	void set_mesh_forward(const glm::vec3& mesh_forward) { this->mesh_forward = mesh_forward; }
	void set_gizmo_type(Gizmo_Type gizmo_type) { this->gizmo_type = gizmo_type; }
	void set_is_visible(bool is_visible) { this->is_visible = is_visible; }
	void set_is_two_sided(bool is_two_sided) { is_two_sided = is_two_sided; }
	void set_is_draggable(bool is_draggable) { is_draggable = is_draggable; }
	void set_can_be_parent(bool can_be_parent) { can_be_parent = can_be_parent; }

	// Gets the world position of the node, taking into account the hierarchical transformations
	const glm::vec3 get_world_position() const;

	// Gets the model matrix, the hierarchical world model matrix, or any of its components separately
	glm::mat4 get_model_matrix() const;
	glm::mat4 get_world_model_matrix() const;
	glm::mat4 get_translation_matrix() const;
	glm::mat4 get_rotation_matrix() const;
	glm::mat4 get_scale_matrix() const;

	// Gets the size of the node's bounding box
	glm::vec3 get_bounding_box_size() const { return bounding_box_max - bounding_box_min; }

	// Getter and setter for the parent node (weak pointer to avoid circular references)
	virtual std::shared_ptr<Node> get_parent() const { return parent.lock(); }
	virtual void set_parent(const std::shared_ptr<Node>& parent) { this->parent = parent; }

	// Child management functions to avoid exposing any concrete implementation to the client code
	virtual void add_child(const std::shared_ptr<Node>& child);
	virtual void remove_child(const std::shared_ptr<Node>& child);
	// Getter for all children of the node (would be empty if it is a leaf)
	virtual std::vector<std::shared_ptr<Node>> get_children() const { return children; }
	// Returns a boolean indicating if the model has children (i.e, does not imply whether it is composite)
	virtual bool is_composite() const { return !children.empty(); }
	// Returns the top-level ancestor of the node in the scene graph or this if it has no parent
	virtual std::shared_ptr<Node> get_root_node() const;

	// Static Public Functions
	// -----------------------
	static std::uint32_t get_node_count() { return node_count; }

private:
	// Static Private Attributes
	// ---------------------------
	static std::uint32_t node_count; // total number of nodes created

protected:
	// Protected Attributes
	// --------------------
	// fundamental node attributes
	std::uint32_t id; // unique identifier for the node in the scene
	std::string name;
	Node_Type type; // type of node (e.g., CAMERA, COMPOSITE_MODEL, SHADER, etc.)

	// content attributes (for mesh-bearing nodes, empty for pure composite nodes)
	std::vector<std::shared_ptr<Mesh>> meshes; // meshes of the node itself (if any)
	std::vector<std::shared_ptr<Texture>> textures; // textures of the node itself (if any)
	std::string directory; // directory of the model file of this node (if any)

	// transform attributes
	glm::vec3 position; // position of the node in local space
	glm::quat rotation; // rotation of the node in local space
	glm::vec3 scale; // scale of the node in local space
	glm::vec3 forward; // forward vector of the node in world space
	glm::vec3 mesh_forward; // original forward vector of the mesh

	// appearance attributes
	glm::vec4 albedo; // albedo color of the node

	// state attributes
	bool is_visible; // visibility of the node in the scene
	bool is_two_sided{ false }; // whether the node is two-sided (for culling)

	// hierarchy attributes
	std::weak_ptr<Node> parent; // weak pointer to the parent node
	std::vector<std::shared_ptr<Node>> children; // vector of shared pointers to the children nodes

	// metadata attributes
	Gizmo_Type gizmo_type; // type of gizmo, if any
	glm::vec3 bounding_box_min{}, bounding_box_max{}; // bounding box of the node

	// behavioral flags
	bool is_draggable; // whether the node can be dragged in the scene graph
	bool can_be_parent; // whether the node can be a parent to other nodes

	// Static Protected Attributes
	// ---------------------------
	// default values for node attributes
	static constexpr glm::vec4 ALBEDO{ 0.8f, 0.8f, 0.8f, 1.0f };
	static constexpr glm::vec3 POSITION{ 0.0f, 0.0f, 0.0f };
	static constexpr glm::quat ROTATION{ 1.0f, 0.0f, 0.0f, 0.0f };
	static constexpr glm::vec3 SCALE{ 1.0f, 1.0f, 1.0f };
	static constexpr glm::vec3 FORWARD{ 0.0f, 0.0f, -1.0f };
};