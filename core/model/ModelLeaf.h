/*
* ModelLeaf.h
* This file defines the ModelLeaf class (a derived class of ModelComponent),
* which represents a leaf model in the scene graph. It cannot bear children.
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
	virtual ~ModelLeaf() {}

	// Public Functions
	// ----------------
	// Loads the model leaf
	void Load() override {}

	// Deallocates all the resources of the model leaf, that is, all its meshes
	void DeallocateResources() override { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Draws the model leaf, that is, all its meshes
	void Draw() const override { for (const Mesh& mesh : meshes) mesh.Draw(); }
	// Draws the model leaf, that is, all its meshes, with the specified shader 
	// (binds the textures before drawing)
	void Draw(const Shader& shader) const override
	{
		auto& nonConstShader = const_cast<Shader&>(shader);
		nonConstShader.Use(); // activate the shader program
		// set the appropiate world model matrix uniform per leaf model before drawing
		nonConstShader.SetMat4("model", GetWorldModelMatrix());

		for (const Mesh& mesh : meshes)
		{
			const_cast<Mesh&>(mesh).BindTextures(nonConstShader); // bind the textures
			mesh.Draw();
		}
	}
};