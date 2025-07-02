/*
* Model.h
* This file defines the Model class (a derived class of Asset),
* which is an abstract base class used to load a model and draw it.
*/

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // enable experimental features in GLM
#include <glm/gtx/quaternion.hpp> // for quaternion operations

#include "../Asset.h"
#include "mesh/Mesh.h"

enum class ModelType { UNDEFINED = 0, ASSIMP_MODEL, SHAPE };

// enumeration class that allows to disriminate between different types of gizmos 
// and also indicate if the model is not a gizmo without a boolean flag
enum class GizmoType { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class Model : public Asset
{
public:
	// Constructors
	// ------------
	Model(const std::string& name,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
		  const ModelType modelType = ModelType::UNDEFINED,
		  const GizmoType gizmoType = GizmoType::NONE);

	Model(const std::string& name,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::vec3 rotationInEulerAnglesDegrees = ROTATION_IN_EULER_ANGLES,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD,
		  const ModelType modelType = ModelType::UNDEFINED,
		  const GizmoType gizmoType = GizmoType::NONE);

	// Virtual destructor 
	// (ensures that derived classes can be deleted properly - polymorphism)
	// ----------------------------------------------------------------------------------------
	virtual ~Model() { nModels--; } // decrements the number of models

	// Public Functions
	// ----------------
	// Loads the model
	virtual void Load() override {}

	// Deallocates all the resources of the model
	virtual void DeallocateResources() override { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Draws the model, that is, all its meshes
	virtual void Draw() const { for (const Mesh& mesh : meshes) mesh.Draw(); }

	// Draws the model, that is, all its meshes, with the specified shader (binds the textures before drawing)
	virtual void Draw(Shader& shader) const
	{
		for (const Mesh& mesh : meshes)
		{
			mesh.BindTextures(shader);
			mesh.Draw();
		}
	}

	// Binds the textures of the model
	virtual void BindTextures(Shader& shader) const { for (const Mesh& mesh : meshes) mesh.BindTextures(shader); }

	// Getters
	const glm::vec3& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::quat& GetRotation() const { return rotation; }
	// Returns the rotation as Euler angles in degrees
	const glm::vec3 GetRotationInEulerAngles() const;
	const glm::vec3& GetScale() const { return scale; }
	// Returns the shape's forward vector in world space, normalized
	const glm::vec3 GetForward() const;
	const glm::vec3& GetMeshForward() const { return meshForward; }
	const ModelType& GetModelType() const { return modelType; }
	const GizmoType& GetGizmoType() const { return gizmoType; }

	// Setters
	void SetAlbedo(const glm::vec3& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	// Sets the rotation directly from a quaternion and updates the forward vector accordingly
	void SetRotation(const glm::quat& rotation);
	// Sets the rotation from Euler angles in degrees and updates the forward vector accordingly
	void SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees);
	void SetScale(const glm::vec3& scale) { this->scale = scale; }
	// Aligns the shape's forward vector to the specified world forward vector
	void SetForward(const glm::vec3& worldForward);
	void SetMeshForward(const glm::vec3& meshForward) { this->meshForward = meshForward; }
	void SetModelType(const ModelType modelType) { this->modelType = modelType; }
	void SetGizmoType(GizmoType gizmoType) { this->gizmoType = gizmoType; }

	// Methods that return the full model matrix, or any of its components separately
	glm::mat4 GetModelMatrix() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;

	// Static Public Functions
	// -----------------------
	static GLuint GetNModels() { return nModels; }

protected:
	// Static Protected Attributes
	// ---------------------------
	static GLuint nModels; // number of models in the scene

	// Protected Attributes
	// --------------------
	std::vector<Mesh> meshes;
	std::vector<Texture> loadedTextures;
	std::string directory; // directory of the model file

	glm::vec3 albedo; // current albedo (color when texture is not applied)

	glm::vec3 position; // current position vector
	glm::quat rotation; // current orientation as a quaternion
	glm::vec3 scale; // current scale vector

	glm::vec3 forward; // shape's current forward vector in world space
	glm::vec3 meshForward; // shape's forward vector in local (mesh) space

	ModelType modelType; // type of the model (e.g., ASSIMP_MODEL, SHAPE)
	GizmoType gizmoType; // type of the gizmo if the model is a gizmo, or NONE if it is not a gizmo

	// Private Static Attributes (for default values)
	// ----------------------------------------------
	static constexpr glm::vec3 ALBEDO{ 0.5f }; // gray (when no texture is applied)
	static constexpr glm::vec3 POSITION{ 0.0f }; // origin position
	static constexpr glm::quat ROTATION{ 1.0f, 0.0f, 0.0f, 0.0f }; // identity quaternion (no rotation)
	static constexpr glm::vec3 ROTATION_IN_EULER_ANGLES{ 0.0f, 0.0f, 0.0f }; // no rotation
	static constexpr glm::vec3 SCALE{ 1.0f }; // unit vector
	static constexpr glm::vec3 FORWARD{ 0.0f, 0.0f, 1.0f }; // +Z direction
};