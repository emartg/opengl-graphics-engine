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
	Model(const std::string& name, const NodeType type = NodeType::COMPOSITE_MODEL,
		  const glm::vec4 albedo = ALBEDO, const glm::vec3 position = POSITION,
		  const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
		  const GizmoType gizmoType = GizmoType::NONE);

	// Virtual destructor
	// ------------------
	virtual ~Model() {}

	// Public Functions
	// ----------------
	// Loads the model and its resources, including its children if the model is composite
	void Load() override { for (const auto& child : children) if (child) child->Load(); }

	// Deallocates all the resources of the model, including its children if the model is composite
	void DeallocateResources() override
	{
		for (const auto& child : children) if (child) child->DeallocateResources();
	}

	// Draws only its own meshes
	void Draw() const override;
	// Draws only its own meshes with the specified shader (binds the textures before drawing)
	void Draw(const Shader& shader) const override;

	// Static Public Methods
	// ---------------------
	static GLuint GetNModels() { return nModels; }

protected:
	// Static Protected Attributes
	// ---------------------------
	static GLuint nModels; // number of models in the scene

};