/*
* ModelComponent.h
* This file defines the ModelComponent class (a derived class of Asset),
* which represents is the abstract base class for any every scene graph node (mesh-bearing or composite).
* It integrates child management directly to allow any model to become composite.
*/

#pragma once

#include <vector>
#include <string>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // enable experimental features in GLM
#include <glm/gtx/quaternion.hpp> // for quaternion operations

#include "../Asset.h"
#include "mesh/Mesh.h"

enum class ModelType
{
	UNDEFINED = 0,
	COMPOSITE_MODEL,
	COMPOSITE_ASSIMP_MODEL, ASSIMP_MODEL,
	COMPOSITE_SHAPE_MODEL, SHAPE_MODEL
};

// enumeration class that allows to disriminate between different types of gizmos 
// and also indicate if the model is not a gizmo without a boolean flag
enum class GizmoType { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

// public std::enable_shared_from_this to allow shared pointers to 'this' object
class ModelComponent : public Asset, public std::enable_shared_from_this<ModelComponent>
{
public:
	// Constructors
	// ------------
	ModelComponent(const std::string& name,
				   const glm::vec3 albedo = ALBEDO, const glm::vec3 position = POSITION,
				   const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
				   const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
				   const ModelType modelType = ModelType::UNDEFINED,
				   const GizmoType gizmoType = GizmoType::NONE);

	// Virtual destructor
	// ------------------
	virtual ~ModelComponent() {}

	// Public Functions
	// ----------------
	// Loads the model component, including its children if the model is composite
	virtual void Load() override = 0;

	// Deallocates all the resources of the model component, including its children if the model is composite
	virtual void DeallocateResources() override = 0;

	// Draws the model component, including its children if the model is composite
	virtual void Draw() const = 0;
	// Draws the model component with the specified shader, including its children if the model is composite
	virtual void Draw(const Shader& shader) const = 0;

	// Getters and Setters
	const glm::vec3& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::quat& GetRotation() const { return rotation; }
	const glm::vec3 GetRotationInEulerAngles() const; // returns the rotation as Euler angles in degrees
	const glm::vec3& GetScale() const { return scale; }
	const glm::vec3 GetForward() const; // returns the forward vector in world space and normalized
	const glm::vec3& GetMeshForward() const { return meshForward; }
	const ModelType& GetModelType() const { return modelType; }
	const GizmoType& GetGizmoType() const { return gizmoType; }

	void SetAlbedo(const glm::vec3& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	void SetRotation(const glm::quat& rotation); // sets the rotation and updates the fwd vector accordingly
	void SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees); // from Euler angles in degrees
	void SetScale(const glm::vec3& scale) { this->scale = scale; }
	void SetForward(const glm::vec3& worldForward); // aligns model fwd vector to the given world fwd vector
	void SetMeshForward(const glm::vec3& meshForward) { this->meshForward = meshForward; }
	void SetModelType(const ModelType modelType) { this->modelType = modelType; }
	void SetGizmoType(GizmoType gizmoType) { this->gizmoType = gizmoType; }

	// Gets the model matrix, the hierarchical world model matrix, or any of its components separately
	glm::mat4 GetModelMatrix() const;
	glm::mat4 GetWorldModelMatrix() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;

	// Gets the size of the model component's bounding box
	glm::vec3 GetBoundingBoxSize() const { return m_boundingBoxMax - m_boundingBoxMin; }

	// Getter and setter for the parent model component (weak pointer to avoid circular references)
	virtual std::shared_ptr<ModelComponent> GetParent() const { return parent.lock(); }
	virtual void SetParent(const std::shared_ptr<ModelComponent>& parent) { this->parent = parent; }

	// Child management functions to avoid exposing any concrete implementation to the client code
	virtual void AddChild(const std::shared_ptr<ModelComponent>& child);
	virtual void RemoveChild(const std::shared_ptr<ModelComponent>& child);
	// Getter for all children of the model component (would be empty if it is a leaf)
	virtual std::vector<std::shared_ptr<ModelComponent>> GetChildren() const { return children; }
	// Returns a boolean indicating if the model has children (i.e, does not imply whether it is composite)
	virtual bool HasChildren() const { return !children.empty(); }
	// Returns the top-level ancestor of the model component in the scene graph or this if it has no parent
	virtual std::shared_ptr<ModelComponent> GetRootParent() const;

	// Static Public Functions
	// -----------------------
	static GLuint GetNModels() { return nModels; }

protected:
	// Static Protected Attributes
	// ---------------------------
	static GLuint nModels; // number of models in the scene (leaf and composite)

	// Protected Attributes
	// --------------------
	// weak pointer to the parent model component to avoid circular references
	std::weak_ptr<ModelComponent> parent;
	// vector of shared pointers to the child model components
	std::vector<std::shared_ptr<ModelComponent>>& children;

	std::vector<Mesh> meshes;
	std::vector<Texture> loadedTextures;
	std::string directory; // directory of the model file

	glm::vec3 albedo; // current albedo (color when texture is not applied)

	glm::vec3 position; // current position vector
	glm::quat rotation; // current orientation as a quaternion
	glm::vec3 scale; // current scale vector

	glm::vec3 forward; // current forward vector in world space
	glm::vec3 meshForward; // forward vector in local (mesh) space

	ModelType modelType; // type of the model component (e.g., ASSIMP_MODEL, SHAPE)
	GizmoType gizmoType; // type of the gizmo if the model component is a gizmo, or NONE if it is not a gizmo

	glm::vec3 m_boundingBoxMin{}; // minimum point of the bounding box
	glm::vec3 m_boundingBoxMax{}; // maximum point of the bounding box

	// Private Static Attributes
	// -------------------------
	// default values for the model component attributes
	static constexpr glm::vec3 ALBEDO{ 0.8 }; // default albedo color (light gray)
	static constexpr glm::vec3 POSITION{ 0.0f }; // origin position
	static constexpr glm::quat ROTATION{ 1.0f, 0.0f, 0.0f, 0.0f }; // identity quaternion (no rotation)
	static constexpr glm::vec3 ROTATION_IN_EULER_ANGLES{ 0.0f, 0.0f, 0.0f }; // no rotation
	static constexpr glm::vec3 SCALE{ 1.0f }; // unit vector
	static constexpr glm::vec3 FORWARD{ 0.0f, 0.0f, 1.0f }; // +Z direction

};