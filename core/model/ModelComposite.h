/*
* ModelComposite.h
* This file defines the ModelComposite class (a derived class of ModelComponent),
* which represents a composite model in the scene graph. It can bear children.
*/

#pragma once

#include <algorithm>

#include "ModelComponent.h"

// public std::enable_shared_from_this to allow shared pointers to 'this' object
class ModelComposite : public ModelComponent
{
public:
	// Constructors
	// ------------
	ModelComposite(const std::string& name,
				   const glm::vec3 albedo = ALBEDO, const glm::vec3 position = POSITION,
				   const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
				   const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
				   const ModelType modelType = ModelType::COMPOSITE_MODEL,
				   const GizmoType gizmoType = GizmoType::NONE)
		: ModelComponent(name, albedo, position, rotation, scale, forward, meshForward,
						 modelType, gizmoType)
	{}

	// Virtual destructor
	// ------------------
	virtual ~ModelComposite() { DeallocateResources(); }

	// Public Functions
	// ----------------
	// Loads all its children recursively
	void Load() override { for (const auto& child : children) if (child) child->Load(); }

	// Deallocates all the resources of all its children recursively, that is, all their meshes
	void DeallocateResources() override
	{
		for (const auto& child : children) if (child) child->DeallocateResources();
	}

	// Draws only its own meshes (children are drawn by their own Draw calls in the scene graph traversal)
	void Draw() const override;
	// Draws only its own meshes with the specified shader (binds the textures before drawing, 
	// children are drawn by their own Draw calls in the scene graph traversal)
	void Draw(const Shader& shader) const override;

	// Child management functions
	void AddChild(const std::shared_ptr<ModelComponent>& child) override;
	void RemoveChild(const std::shared_ptr<ModelComponent>& child) override;
	// Getter for all children of the model component (would be empty if it is a leaf)
	std::vector<std::shared_ptr<ModelComponent>> GetChildren() const { return children; }
	// Returns a boolean indicating if the model has children (i.e, does not imply whether it is composite)
	bool HasChildren() const { return !children.empty(); }
	// Returns the top-level ancestor of the model component in the scene graph or this if it has no parent
	std::shared_ptr<ModelComponent> GetRootParent() const override
	{
		// base class implementation is mandatory since it uses shared_from_this() and
		// derived classes cannot derive from enable_shared_from_this again, 
		// since it would lead to undefined behavior (ambiguous which base to use)
		return ModelComponent::GetRootParent();
	}
};