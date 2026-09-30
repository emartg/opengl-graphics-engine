/*
 * Model.h
 * This file defines the Model class (a derived class of Node),
 * which is used as a base class for all mesh-bearing nodes in the scene graph,
 * i.e., models that contain meshes to be drawn.
 */

#pragma once

#include "../Node.h"

#include <iostream>
#include <vector>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Mesh;
class Shader;

class Model : public Node
{
public:
	// Constructors
	// ------------
	Model(
		const std::string& name,
		const Node_Type    type         = Node_Type::COMPOSITE_MODEL,
		const glm::vec4    albedo       = ALBEDO,
		const glm::vec3    position     = POSITION,
		const glm::quat    rotation     = ROTATION,
		const glm::vec3    scale        = SCALE,
		const glm::vec3    forward      = FORWARD,
		const glm::vec3    mesh_forward = FORWARD,
		const Gizmo_Type   gizmo_type   = Gizmo_Type::NONE);

	// Virtual destructor
	// ------------------
	virtual ~Model() {}

	// Public Functions
	// ----------------
	// Loads the model and its resources, including its children if the model is composite
	void load() override
	{
		for (const auto& child : children)
			if (child)
				child->load();
	}

	// Deallocates all the resources of the model, including its children if the model is composite
	void deallocate_resources() override
	{
		for (const auto& child : children)
			if (child)
				child->deallocate_resources();
	}

	// Draws only its own meshes
	void draw() const override;
	// Draws only its own meshes with the specified shader (binds the textures before drawing)
	void draw(const Shader& shader) const override;
};