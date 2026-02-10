/*
* Node.cpp
* This file implements the Node class, which represents is the abstract base class
* for any every scene graph node (mesh-bearing or composite).
* It integrates child management directly to allow any node to become composite.
*/

#include "Node.h"

#include <algorithm>

#include "shader/Shader.h"
#include "model/mesh/Mesh.h"
#include "texture/Texture.h"

// Static Private Attributes
// -------------------------
std::uint32_t Node::nodesCount{}; // initialize the total number of nodes created to 0
std::uint32_t Node::nNodes{}; // initialize the number of nodes in the scene to 0

// Public Methods
// --------------
const glm::vec3 Node::GetRotationInEulerAngles() const
{
	// convert the quaternion rotation to Euler angles in radians, then to degrees
	return glm::degrees(glm::eulerAngles(this->rotation));
}
const glm::vec3 Node::GetForward() const
{
	// return the forward vector in world space, normalized
	return glm::normalize(this->rotation * meshForward);
}

void Node::SetRotation(const glm::quat& rotation)
{
	// normalize to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation);
	// update forward vector based on the new rotation
	this->forward = glm::normalize(this->rotation * meshForward);
}
void Node::SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees)
{
	// convert degrees to radians for glm::quat constructor
	glm::vec3 eulerAnglesRadians = glm::radians(eulerAnglesDegrees);
	// create quaternion from Euler angles
	glm::quat newRotation = glm::quat(eulerAnglesRadians);
	// use the SetRotation method to set the new rotation as a quaternion
	SetRotation(newRotation);
}
void Node::SetForward(const glm::vec3& worldForward)
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

glm::mat4 Node::GetModelMatrix() const
{
	glm::mat4 model = glm::mat4{ 1.0f };
	model = glm::translate(model, position); // apply translation
	model *= glm::mat4_cast(rotation); // apply rotation
	model = glm::scale(model, scale); // apply scaling
	return model;
}
glm::mat4 Node::GetWorldModelMatrix() const
{
	glm::mat4 localModelMatrix = GetModelMatrix(); // get local model matrix
	auto parentPtr = parent.lock(); // get shared pointer to parent node (if any)
	// if there is a parent, multiply its world model matrix with the local model matrix to get
	// the world model matrix of this node, otherwise return the local model matrix
	return parentPtr ? parentPtr->GetWorldModelMatrix() * localModelMatrix : localModelMatrix;
}
glm::mat4 Node::GetTranslationMatrix() const
{
	glm::mat4 translationMatrix = glm::mat4{ 1.0f };
	translationMatrix = glm::translate(translationMatrix, position);
	return translationMatrix;
}
glm::mat4 Node::GetRotationMatrix() const
{
	glm::mat4 rotationMatrix = glm::mat4{ 1.0f };
	rotationMatrix *= glm::mat4_cast(rotation);
	return rotationMatrix;
}
glm::mat4 Node::GetScaleMatrix() const
{
	glm::mat4 scaleMatrix = glm::mat4{ 1.0f };
	scaleMatrix = glm::scale(scaleMatrix, scale);
	return scaleMatrix;
}

const glm::vec3 Node::GetWorldPosition() const
{
	// get the world model matrix and extract the translation component
	glm::mat4 worldModelMatrix = GetWorldModelMatrix();
	return glm::vec3(worldModelMatrix[3]); // return the translation part
}

void Node::AddChild(const std::shared_ptr<Node>& child)
{
	if (!child) return; // check for null pointer
	// avoid re-parenting to self
	if (child.get() == this) return;
	// avoid adding duplicate children
	if (std::find(children.begin(), children.end(), child) != children.end()) return;

	children.push_back(child); // add the child to the children vector
	// use shared_from_this to get a shared_ptr to 'this' object
	child->SetParent(shared_from_this()); // set this node as the parent of the child
}
void Node::RemoveChild(const std::shared_ptr<Node>& child)
{
	if (!child) return; // check for null pointer

	// remove the child from the children vector
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	child->SetParent(nullptr); // reset the parent of the child to nullptr
}
std::shared_ptr<Node> Node::GetRootNode() const
{
	// start from this node, using const_cast to call shared_from_this (which is non-const)
	auto currentModel = const_cast<Node*>(this)->shared_from_this();

	// traverse up the parent chain until reaching the top-level ancestor (no parent)
	while (currentModel && currentModel->GetParent())
		currentModel = currentModel->GetParent(); // move up to the next parent

	return currentModel; // return the top-level ancestor or this if no parent
}