/*
* ModelLeaf.h
* This file defines the ModelLeaf class (a derived class of ModelComponent),
* which represents an intentionally non-composite model node in the scene graph (semantic leaf).
*/

#pragma once

#include "ModelComponent.h"

class ModelLeaf : public ModelComponent
{
public:
	// Constructors
	// ------------
	// Forwarding constructors for the different types of model leaf initialization
	ModelLeaf(const std::string& name,
			  const glm::vec3 albedo = ALBEDO, const glm::vec3 position = POSITION,
			  const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
			  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
			  const ModelType modelType = ModelType::UNDEFINED,
			  const GizmoType gizmoType = GizmoType::NONE)
		: ModelComponent(name, albedo, position, rotation, scale, forward, meshForward,
						 modelType, gizmoType)
	{}

	ModelLeaf(const std::string& name,
			  const glm::vec3 albedo, const glm::vec3 position,
			  const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale = SCALE,
			  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
			  const ModelType modelType = ModelType::UNDEFINED,
			  const GizmoType gizmoType = GizmoType::NONE)
		: ModelComponent(name, albedo, position, rotationInEulerAnglesDegrees, scale,
						 forward, meshForward, modelType, gizmoType)
	{}

	ModelLeaf(const std::string& name, std::string const& path,
			  const glm::vec3 albedo = ALBEDO, const glm::vec3 position = POSITION,
			  const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
			  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD)
		: ModelComponent(name, albedo, position, rotation, scale, forward, meshForward,
						 ModelType::ASSIMP_MODEL) // set the model type to ASSIMP_MODEL
	{}

	// Virtual destructor 
	// ------------------
	virtual ~ModelLeaf() { DeallocateResources(); }

	// Public Functions
	// ----------------
	// Loads the model leaf
	void Load() override {}

	// Deallocates all the resources of the model leaf, that is, all its meshes
	void DeallocateResources() override { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Draws only its own meshes as it is a leaf (no children, enforced by not using AddChild())
	void Draw() const override;
	// Draws only its own meshes as it is a leaf (no children, enforced by not using AddChild()),
	// with the specified shader (binds the textures before drawing)
	void Draw(const Shader& shader) const override;

	// Child management functions to enforce leaf semantics
	void AddChild(const std::shared_ptr<ModelComponent>& child) override {}
	void RemoveChild(const std::shared_ptr<ModelComponent>& child) override {}
	// Returns an empty vector as this model is a leaf and thus cannot have children
	std::vector<std::shared_ptr<ModelComponent>> GetChildren() const override { return {}; }
	// Returns false indicating that this model is a leaf and thus cannot have children
	bool HasChildren() const override { return false; }
};