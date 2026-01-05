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
enum class NodeType
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
enum class GizmoType { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

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
	Node(const std::string& name, const NodeType type = NodeType::UNDEFINED,
		 const glm::vec4 albedo = ALBEDO, const glm::vec3 position = POSITION,
		 const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
		 const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
		 const GizmoType gizmoType = GizmoType::NONE)
		: id{ ++nodesCount }, name{ name }, type{ type },
		albedo{ albedo }, position{ position }, rotation{ rotation },
		scale{ scale }, forward{ forward }, meshForward{ meshForward },
		gizmoType{ gizmoType }, children{ } // initialize children vector as empty
	{
		nNodes++;
	}

	// Virtual destructor
	// ------------------
	virtual ~Node() { nNodes--; }

	// Public Functions
	// ----------------
	// Loads the node and its resources, including its children if the node is composite
	virtual void Load() { for (const auto& child : children) if (child) child->Load(); }

	// Deallocates all the resources of the node, including its children if the node is composite
	virtual void DeallocateResources()
	{
		for (const auto& child : children) if (child) child->DeallocateResources();
	}

	// Draws the node if it is a mesh-bearing node, including its children if the node is composite
	virtual void Draw() const = 0;
	// Draws the node if it is a mesh-bearing node, including its children if the node is composite,
	// using the specified shader (binds the textures before drawing)
	virtual void Draw(const Shader& shader) const = 0;

	// Getters and Setters
	std::uint32_t GetId() const { return id; }
	const std::string& GetName() const { return name; }
	const NodeType& GetNodeType() const { return type; }

	void SetId(std::uint32_t id) { this->id = id; }
	void SetName(const std::string& name) { this->name = name; }
	void SetNodeType(const NodeType& type) { this->type = type; }

	const glm::vec4& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::quat& GetRotation() const { return rotation; }
	const glm::vec3 GetRotationInEulerAngles() const; // returns the rotation as Euler angles in degrees
	const glm::vec3& GetScale() const { return scale; }
	const glm::vec3 GetForward() const; // returns the forward vector in world space and normalized
	const glm::vec3& GetMeshForward() const { return meshForward; }
	const GizmoType& GetGizmoType() const { return gizmoType; }

	void SetAlbedo(const glm::vec4& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	void SetRotation(const glm::quat& rotation); // sets the rotation and updates the fwd vector accordingly
	void SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees); // from Euler angles in degrees
	void SetScale(const glm::vec3& scale) { this->scale = scale; }
	void SetForward(const glm::vec3& worldForward); // aligns node's fwd vector to the given world fwd vector
	void SetMeshForward(const glm::vec3& meshForward) { this->meshForward = meshForward; }
	void SetGizmoType(GizmoType gizmoType) { this->gizmoType = gizmoType; }

	// Gets the model matrix, the hierarchical world model matrix, or any of its components separately
	glm::mat4 GetModelMatrix() const;
	glm::mat4 GetWorldModelMatrix() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;

	// Gets the size of the node's bounding box
	glm::vec3 GetBoundingBoxSize() const { return m_boundingBoxMax - m_boundingBoxMin; }

	// Getter and setter for the parent node (weak pointer to avoid circular references)
	virtual std::shared_ptr<Node> GetParent() const { return parent.lock(); }
	virtual void SetParent(const std::shared_ptr<Node>& parent) { this->parent = parent; }

	// Child management functions to avoid exposing any concrete implementation to the client code
	virtual void AddChild(const std::shared_ptr<Node>& child);
	virtual void RemoveChild(const std::shared_ptr<Node>& child);
	// Getter for all children of the node (would be empty if it is a leaf)
	virtual std::vector<std::shared_ptr<Node>> GetChildren() const { return children; }
	// Returns a boolean indicating if the model has children (i.e, does not imply whether it is composite)
	virtual bool IsComposite() const { return !children.empty(); }
	// Returns the top-level ancestor of the node in the scene graph or this if it has no parent
	virtual std::shared_ptr<Node> GetRootNode() const;

	// Static Public Functions
	// -----------------------
	static std::uint32_t GetNodesCount() { return nodesCount; }
	static std::uint32_t GetNNodes() { return nNodes; }

private:
	// Static Private Attributes
	// ---------------------------
	static std::uint32_t nodesCount; // total number of nodes created
	static std::uint32_t nNodes; // number of nodes in the scene (leaf and composite)

protected:
	// Protected Attributes
	// --------------------
	// fundamental node attributes
	std::uint32_t id; // unique identifier for the node in the scene
	std::string name;
	NodeType type; // type of node (e.g., COMPOSITE_MODEL, COMPOSITE_ASSIMP_MODEL, ASSIMP_MODEL, etc.)

	// weak pointer to the parent node to avoid circular references
	std::weak_ptr<Node> parent;
	// vector of shared pointers to the child nodes
	std::vector<std::shared_ptr<Node>> children;

	std::vector<std::shared_ptr<Mesh>> meshes; // meshes of the node itself (if any)
	std::vector<std::shared_ptr<Texture>> textures; // textures of the node itself (if any)
	std::string directory; // directory of the model file of this node (if any)

	glm::vec4 albedo; // current albedo (color when texture is not applied)

	glm::vec3 position; // current position vector
	glm::quat rotation; // current orientation as a quaternion
	glm::vec3 scale; // current scale vector

	glm::vec3 forward; // current forward vector in world space
	glm::vec3 meshForward; // forward vector in local (mesh) space

	GizmoType gizmoType; // type of the gizmo if the node is a gizmo, or NONE if it is not a gizmo

	glm::vec3 m_boundingBoxMin{}; // minimum point of the bounding box
	glm::vec3 m_boundingBoxMax{}; // maximum point of the bounding box

	// Private Static Attributes
	// -------------------------
	// default values for the node attributes
	static constexpr glm::vec4 ALBEDO{ 0.8, 0.8f, 0.8f, 1.0f }; // light gray color as default (RGBA)
	static constexpr glm::vec3 POSITION{ 0.0f }; // origin position
	static constexpr glm::quat ROTATION{ 1.0f, 0.0f, 0.0f, 0.0f }; // identity quaternion (no rotation)
	static constexpr glm::vec3 ROTATION_IN_EULER_ANGLES{ 0.0f, 0.0f, 0.0f }; // no rotation
	static constexpr glm::vec3 SCALE{ 1.0f }; // unit vector
	static constexpr glm::vec3 FORWARD{ 0.0f, 0.0f, 1.0f }; // +Z direction

};