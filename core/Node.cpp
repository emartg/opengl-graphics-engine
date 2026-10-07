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
std::uint32_t Node::node_count{}; // initialize the total number of nodes created to 0

// Public Methods
// --------------
const glm::vec3 Node::get_rotation_in_euler_angles() const
{
	// convert the quaternion rotation to Euler angles in radians, then to degrees
	return glm::degrees(glm::eulerAngles(this->rotation));
}
const glm::vec3 Node::get_forward() const
{
	// return the forward vector in world space, normalized
	return glm::normalize(this->rotation * mesh_forward);
}

void Node::set_rotation(const glm::quat& rotation)
{
	// normalize to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation);
	// update forward vector based on the new rotation
	this->forward = glm::normalize(this->rotation * mesh_forward);
}
void Node::set_rotation_in_euler_angles(const glm::vec3 euler_angles_degrees)
{
	// convert degrees to radians for glm::quat constructor
	glm::vec3 euler_angles_radians = glm::radians(euler_angles_degrees);
	// create quaternion from Euler angles
	glm::quat new_rotation = glm::quat(euler_angles_radians);
	// use the set_rotation method to set the new rotation as a quaternion
	set_rotation(new_rotation);
}
void Node::set_forward(const glm::vec3& world_forward)
{
	// ensure the world_forward vector is normalized
	glm::vec3 normalized_world_forward = glm::normalize(world_forward);
	// rotate the mesh_forward vector to align with the world_forward vector
	glm::quat rotation_quat = glm::rotation(mesh_forward, normalized_world_forward);
	// normalize the rotation quaternion to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation_quat);
	// update the forward vector based on the new rotation
	this->forward = normalized_world_forward;
}

glm::mat4 Node::get_model_matrix() const
{
	glm::mat4 model = glm::mat4{ 1.0f };
	model           = glm::translate(model, position); // apply translation
	model *= glm::mat4_cast(rotation);                 // apply rotation
	model = glm::scale(model, scale);                  // apply scaling
	return model;
}
glm::mat4 Node::get_world_model_matrix() const
{
	glm::mat4 local_model_matrix = get_model_matrix(); // get local model matrix
	auto      parent_ptr         = parent.lock();      // get shared pointer to parent node (if any)
	// if there is a parent, multiply its world model matrix with the local model matrix to get
	// the world model matrix of this node, otherwise return the local model matrix
	return parent_ptr ? parent_ptr->get_world_model_matrix() * local_model_matrix : local_model_matrix;
}
glm::mat4 Node::get_translation_matrix() const
{
	glm::mat4 translation_matrix = glm::mat4{ 1.0f };
	translation_matrix           = glm::translate(translation_matrix, position);
	return translation_matrix;
}
glm::mat4 Node::get_rotation_matrix() const
{
	glm::mat4 rotation_matrix = glm::mat4{ 1.0f };
	rotation_matrix *= glm::mat4_cast(rotation);
	return rotation_matrix;
}
glm::mat4 Node::get_scale_matrix() const
{
	glm::mat4 scale_matrix = glm::mat4{ 1.0f };
	scale_matrix           = glm::scale(scale_matrix, scale);
	return scale_matrix;
}

const glm::vec3 Node::get_world_position() const
{
	// get the world model matrix and extract the translation component
	glm::mat4 world_model_matrix = get_world_model_matrix();
	return glm::vec3(world_model_matrix[3]); // return the translation part
}

Bounding_Box Node::get_local_bounding_box() const
{
	// combine the bounding boxes of the node's own meshes (children are not included)
	Bounding_Box bounding_box;
	for (const auto& mesh : meshes)
		if (mesh)
			bounding_box.expand(mesh->get_bounding_box());
	return bounding_box;
}

Bounding_Box Node::get_world_bounding_box() const
{
	return get_local_bounding_box().transformed(get_world_model_matrix());
}

void Node::add_child(const std::shared_ptr<Node>& child)
{
	if (!child)
		return; // check for null pointer
	// avoid re-parenting to self
	if (child.get() == this)
		return;
	// avoid adding duplicate children
	if (std::find(children.begin(), children.end(), child) != children.end())
		return;

	children.push_back(child); // add the child to the children vector
	// use shared_from_this to get a shared_ptr to 'this' object
	child->set_parent(shared_from_this()); // set this node as the parent of the child
}
void Node::remove_child(const std::shared_ptr<Node>& child)
{
	if (!child)
		return; // check for null pointer

	// remove the child from the children vector
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	child->set_parent(nullptr); // reset the parent of the child to nullptr
}
std::shared_ptr<Node> Node::get_root_node() const
{
	// start from this node, using const_cast to call shared_from_this (which is non-const)
	auto current_node = const_cast<Node*>(this)->shared_from_this();

	// traverse up the parent chain until reaching the top-level ancestor (no parent)
	while (current_node && current_node->get_parent()) current_node = current_node->get_parent(); // move up to the next parent

	return current_node; // return the top-level ancestor or this if no parent
}
