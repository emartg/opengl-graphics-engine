/*
* Model.h
* This file implements the Model class (a derived class of Asset),
* which is an abstract base class used to load a model and draw it.
*/

#include "Model.h"

// Static Protected Attributes
// ---------------------------
GLuint Model::nModels{}; // initialize the number of models in the scene to 0

// Constructors
// ------------
Model::Model(const std::string& name,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward,
			 const ModelType modelType,
			 const GizmoType gizmoType)
	: Asset(name, AssetType::MODEL),
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward },
	modelType{ modelType },
	gizmoType{ gizmoType }
{
	nModels++; // increments the number of models
}

Model::Model(const std::string& name,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::vec3 rotationInEulerAnglesDegrees, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward,
			 const ModelType modelType,
			 const GizmoType gizmoType)
	: Model(name,
			albedo,
			position, glm::quat(glm::radians(rotationInEulerAnglesDegrees)), scale,
			forward, meshForward,
			modelType,
			gizmoType)
{}

// Public Methods
// --------------
const glm::vec3 Model::GetRotationInEulerAngles() const
{
	// convert the quaternion rotation to Euler angles in radians, then to degrees
	return glm::degrees(glm::eulerAngles(this->rotation));
}
const glm::vec3 Model::GetForward() const
{
	// return the forward vector in world space, normalized
	return glm::normalize(this->rotation * meshForward);
}

void Model::SetRotation(const glm::quat& rotation)
{
	// normalize to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation);
	// update forward vector based on the new rotation
	this->forward = glm::normalize(this->rotation * meshForward);
}
void Model::SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees)
{
	// convert degrees to radians for glm::quat constructor
	glm::vec3 eulerAnglesRadians = glm::radians(eulerAnglesDegrees);
	// create quaternion from Euler angles
	glm::quat newRotation = glm::quat(eulerAnglesRadians);
	// use the SetRotation method to set the new rotation as a quaternion
	SetRotation(newRotation);
}
void Model::SetForward(const glm::vec3& worldForward)
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

glm::mat4 Model::GetModelMatrix() const
{
	glm::mat4 model = glm::mat4{ 1.0f };
	model = glm::translate(model, position); // apply translation
	model *= glm::mat4_cast(rotation); // apply rotation
	model = glm::scale(model, scale); // apply scaling
	return model;
}
glm::mat4 Model::GetTranslationMatrix() const
{
	glm::mat4 translationMatrix = glm::mat4{ 1.0f };
	translationMatrix = glm::translate(translationMatrix, position);
	return translationMatrix;
}
glm::mat4 Model::GetRotationMatrix() const
{
	glm::mat4 rotationMatrix = glm::mat4{ 1.0f };
	rotationMatrix *= glm::mat4_cast(rotation);
	return rotationMatrix;
}
glm::mat4 Model::GetScaleMatrix() const
{
	glm::mat4 scaleMatrix = glm::mat4{ 1.0f };
	scaleMatrix = glm::scale(scaleMatrix, scale);
	return scaleMatrix;
}