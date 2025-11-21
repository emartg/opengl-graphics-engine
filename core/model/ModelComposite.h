/*
* ModelComposite.h
* This file defines the ModelComposite class (a derived class of ModelComponent),
* which represents a composite model in the scene graph. It can bear children.
*/

#pragma once

#include "ModelComponent.h"

// public std::enable_shared_from_this to allow shared pointers to 'this' object
class ModelComposite : public ModelComponent, public std::enable_shared_from_this<ModelComposite>
{
public:
	// Constructors
	// ------------
	// Forwarding constructors for the different types of model composite initialization
	ModelComposite(const std::string& name,
				   const glm::vec3 albedo = ALBEDO, const glm::vec3 position = POSITION,
				   const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
				   const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
				   const ModelType modelType = ModelType::COMPOSITE_MODEL,
				   const GizmoType gizmoType = GizmoType::NONE);

	ModelComposite(const std::string& name,
				   const glm::vec3 albedo, const glm::vec3 position,
				   const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale = SCALE,
				   const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
				   const ModelType modelType = ModelType::COMPOSITE_MODEL,
				   const GizmoType gizmoType = GizmoType::NONE);

	// Virtual destructor
	// ------------------
	virtual ~ModelComposite() {}

	// Public Functions
	// ----------------
	// Loads the composite model and all its children recursively
	void Load() override;

	// Deallocates all the resources of the composite model and all its children recursively
	void DeallocateResources() override;

	// Child management functions
	// Addition and removal of child model components
	void AddChild(const std::shared_ptr<ModelComponent>& child) override;
	void RemoveChild(const std::shared_ptr<ModelComponent>& child) override;
	// Getter for all children of the composite model
	std::vector<std::shared_ptr<ModelComponent>> GetChildren() const override { return children; }

	// Returns true indicating that this model is composite
	bool IsComposite() const override { return true; }

	// Draws the composite model and all its children recursively
	void Draw() const override;
	// Draws the composite model and all its children recursively with the specified shader
	void Draw(const Shader& shader) const override;

protected:
	// Protected Attributes
	// --------------------
	std::vector<std::shared_ptr<ModelComponent>> children; // vector of child model components

};