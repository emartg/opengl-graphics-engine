/*
* ModelComposite.cpp
* This file implements the ModelComposite class (a derived class of ModelComponent),
* which represents a composite model in the scene graph. It can bear children.
*/

#include "ModelComposite.h"

// Constructors
// ------------
ModelComposite::ModelComposite(const std::string& name,
							   const glm::vec3 albedo, const glm::vec3 position,
							   const glm::quat rotation, const glm::vec3 scale,
							   const glm::vec3 forward, const glm::vec3 meshForward,
							   const ModelType modelType, const GizmoType gizmoType)
	: ModelComponent(name, albedo, position, rotation, scale, forward, meshForward,
					 modelType, gizmoType)
{}

ModelComposite::ModelComposite(const std::string& name,
							   const glm::vec3 albedo, const glm::vec3 position,
							   const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale,
							   const glm::vec3 forward, const glm::vec3 meshForward,
							   const ModelType modelType, const GizmoType gizmoType)
	: ModelComponent(name, albedo, position, rotationInEulerAnglesDegrees, scale,
					 forward, meshForward, modelType, gizmoType)
{}

// Public Methods
// --------------
void ModelComposite::Load()
{
	// load all children recursively
	for (const auto& child : children) child->Load();
}

void ModelComposite::DeallocateResources()
{
	// deallocate resources of all children recursively
	for (const auto& child : children) child->DeallocateResources();
}

void ModelComposite::AddChild(const std::shared_ptr<ModelComponent>& child)
{
	// add the child to the children vector
	children.push_back(child);
	// set this composite as the parent of the child
	child->SetParent(shared_from_this()); // use shared_from_this to get a shared_ptr to 'this' object
}

void ModelComposite::RemoveChild(const std::shared_ptr<ModelComponent>& child)
{
	// remove the child from the children vector
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	// reset the parent of the child
	child->SetParent(nullptr);
}

void ModelComposite::Draw() const
{
	// draw all children recursively
	for (const auto& child : children) child->Draw();
}

void ModelComposite::Draw(const Shader& shader) const
{
	// draw all children recursively with the specified shader
	for (const auto& child : children) child->Draw(shader);
}