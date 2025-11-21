/*
* ComponentModel.h
* This file defines the ComponentModel class,
* which represents the interface for any model component in the scene graph, both leaf and composite.
* It is the abstract base class for ModelLeaf and ModelComposite, and it is derived from the Asset class:
* - Declares common attributes and methods for all model components.
* - Allows treating individual models and groups of models uniformly.
*/

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // enable experimental features in GLM
#include <glm/gtx/quaternion.hpp> // for quaternion operations

#include "../Asset.h"
#include "mesh/Mesh.h"

enum class ModelType { UNDEFINED = 0, COMPOSITE_MODEL, ASSIMP_MODEL, SHAPE_MODEL };

// enumeration class that allows to disriminate between different types of gizmos 
// and also indicate if the model is not a gizmo without a boolean flag
enum class GizmoType { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class ModelComponent : public Asset
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

	ModelComponent(const std::string& name,
				   const glm::vec3 albedo, const glm::vec3 position,
				   const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale = SCALE,
				   const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
				   const ModelType modelType = ModelType::UNDEFINED,
				   const GizmoType gizmoType = GizmoType::NONE);

	// Virtual destructor
	// ------------------
	virtual ~ModelComponent() {}

	// Public Functions
	// ----------------
	// Loads the model component, including its children if any
	virtual void Load() override = 0;

	// Deallocates all the resources of the model component, including its children if any
	virtual void DeallocateResources() override = 0;

	// Getter and setter for the parent model component (weak pointer to avoid circular references)
	virtual std::shared_ptr<ModelComponent> GetParent() const { return parent.lock(); }
	virtual void SetParent(const std::shared_ptr<ModelComponent>& parent) { this->parent = parent; }

	// Child management functions to avoid exposing any concrete implementation to the client code
	// (they will be overridden only in the ModelComposite class, as leaf cannot bear children)
	// Addition and removal of child model components (default implementation for leaf models)
	virtual void AddChild(const std::shared_ptr<ModelComponent>& child) {}
	virtual void RemoveChild(const std::shared_ptr<ModelComponent>& child) {}
	// Getter for all children of the model component (default implementation for leaf models)
	virtual std::vector<std::shared_ptr<ModelComponent>> GetChildren() const { return {}; }

	// Returns a boolean indicating whether the model is composite, i.e, can have children
	virtual bool IsComposite() const { return false; } // default implementation for leaf models

	// Draws the model component, 
	// independently of whether it is a leaf or composite model
	virtual void Draw() const = 0;
	// Draws the model component with the specified shader, 
	// independently of whether it is a leaf or composite model
	virtual void Draw(const Shader& shader) const = 0;

	// Getters
	const glm::vec3& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::quat& GetRotation() const { return rotation; }
	// Returns the rotation as Euler angles in degrees
	const glm::vec3 GetRotationInEulerAngles() const;
	const glm::vec3& GetScale() const { return scale; }
	// Returns the shape's forward vector in world space, normalized
	const glm::vec3 GetForward() const;
	const glm::vec3& GetMeshForward() const { return meshForward; }
	const ModelType& GetModelType() const { return modelType; }
	const GizmoType& GetGizmoType() const { return gizmoType; }

	// Setters
	void SetAlbedo(const glm::vec3& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	// Sets the rotation directly from a quaternion and updates the forward vector accordingly
	void SetRotation(const glm::quat& rotation);
	// Sets the rotation from Euler angles in degrees and updates the forward vector accordingly
	void SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees);
	void SetScale(const glm::vec3& scale) { this->scale = scale; }
	// Aligns the shape's forward vector to the specified world forward vector
	void SetForward(const glm::vec3& worldForward);
	void SetMeshForward(const glm::vec3& meshForward) { this->meshForward = meshForward; }
	void SetModelType(const ModelType modelType) { this->modelType = modelType; }
	void SetGizmoType(GizmoType gizmoType) { this->gizmoType = gizmoType; }

	// Methods that return the model matrix, the hierarchical world model matrix,
	// or any of its components separately
	glm::mat4 GetModelMatrix() const;
	glm::mat4 GetWorldModelMatrix() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;

	// Gets the size of the model component's bounding box
	glm::vec3 GetBoundingBoxSize() const { return m_boundingBoxMax - m_boundingBoxMin; }

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