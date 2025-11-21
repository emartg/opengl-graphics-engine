/*
* ModelComponent.cpp
* This file defines the ComponentModel class,
* which represents the interface for any model component in the scene graph, both leaf and composite.
* It is the abstract base class for ModelLeaf and ModelComposite, and it is derived from the Asset class:
* - Declares common attributes and methods for all model components.
* - Allows treating individual models and groups of models uniformly.
*/

#include "ModelComponent.h"

// Static Protected Attributes
// ---------------------------
GLuint ModelComponent::nModels{}; // initialize the number of models in the scene to 0

// Constructors
// ------------
ModelComponent::ModelComponent(const std::string& name,
							   const glm::vec3 albedo, const glm::vec3 position,
							   const glm::quat rotation, const glm::vec3 scale,
							   const glm::vec3 forward, const glm::vec3 meshForward,
							   const ModelType modelType, const GizmoType gizmoType)
	: Asset(name, AssetType::MODEL),
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward }, modelType{ modelType }, gizmoType{ gizmoType }
{
	nModels++;
}

ModelComponent::ModelComponent(const std::string& name,
							   const glm::vec3 albedo, const glm::vec3 position,
							   const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale,
							   const glm::vec3 forward, const glm::vec3 meshForward,
							   const ModelType modelType, const GizmoType gizmoType)
	: ModelComponent(name, albedo, position, glm::quat(glm::radians(rotationInEulerAnglesDegrees)), scale,
					 forward, meshForward, modelType, gizmoType)
{}

// Public Methods
// --------------
const glm::vec3 ModelComponent::GetRotationInEulerAngles() const
{
	// convert the quaternion rotation to Euler angles in radians, then to degrees
	return glm::degrees(glm::eulerAngles(this->rotation));
}
const glm::vec3 ModelComponent::GetForward() const
{
	// return the forward vector in world space, normalized
	return glm::normalize(this->rotation * meshForward);
}

void ModelComponent::SetRotation(const glm::quat& rotation)
{
	// normalize to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation);
	// update forward vector based on the new rotation
	this->forward = glm::normalize(this->rotation * meshForward);
}
void ModelComponent::SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees)
{
	// convert degrees to radians for glm::quat constructor
	glm::vec3 eulerAnglesRadians = glm::radians(eulerAnglesDegrees);
	// create quaternion from Euler angles
	glm::quat newRotation = glm::quat(eulerAnglesRadians);
	// use the SetRotation method to set the new rotation as a quaternion
	SetRotation(newRotation);
}
void ModelComponent::SetForward(const glm::vec3& worldForward)
{
	// ensure the worldForward vector is normalized
	glm::vec3 normWorldForward = glm::normalize(worldForward);
	// rotate the meshForward vector to align with the worldForward vector
	glm::quat rotationQuat = glm::rotation(meshForward, normWorldForward);
	// normalize the rotation quaternion to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotationQuat);
	// update the forward vector based on the new rotation
	this->forward = normWorldForward;
}

glm::mat4 ModelComponent::GetModelMatrix() const
{
	glm::mat4 model = glm::mat4{ 1.0f };
	model = glm::translate(model, position); // apply translation
	model *= glm::mat4_cast(rotation); // apply rotation
	model = glm::scale(model, scale); // apply scaling
	return model;
}
glm::mat4 ModelComponent::GetWorldModelMatrix() const
{
	glm::mat4 localModelMatrix = GetModelMatrix(); // get local model matrix
	auto parentPtr = parent.lock(); // get shared pointer to parent model component (if any)
	// if there is a parent, multiply its world model matrix with the local model matrix to get
	// the world model matrix of this model component, otherwise return the local model matrix
	return parentPtr ? parentPtr->GetWorldModelMatrix() * localModelMatrix : localModelMatrix;
}
glm::mat4 ModelComponent::GetTranslationMatrix() const
{
	glm::mat4 translationMatrix = glm::mat4{ 1.0f };
	translationMatrix = glm::translate(translationMatrix, position);
	return translationMatrix;
}
glm::mat4 ModelComponent::GetRotationMatrix() const
{
	glm::mat4 rotationMatrix = glm::mat4{ 1.0f };
	rotationMatrix *= glm::mat4_cast(rotation);
	return rotationMatrix;
}
glm::mat4 ModelComponent::GetScaleMatrix() const
{
	glm::mat4 scaleMatrix = glm::mat4{ 1.0f };
	scaleMatrix = glm::scale(scaleMatrix, scale);
	return scaleMatrix;
}