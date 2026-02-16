/*
* Light.h
* This file defines the Light class (a derived class of Node),
* which is an abstract base class used to create a light source.
*/

#pragma once

#include "../Node.h"

#include <iostream>
#include <string>
#include <memory> 

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Enumeration for the different types of lights
enum class LightType { UNDEFINED = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class Light : public Node
{
public:
	// Constructors
	// ------------
	Light(const std::string& name,
		  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
		  const std::shared_ptr<Node> gizmo = nullptr,
		  const LightType type = LightType::UNDEFINED);

	// Virtual destructor
	// ------------------
	virtual ~Light() = default;

	// Public Methods
	// --------------
	// Loads the light and its resources, including its children if the light is composite
	void Load() override { for (const auto& child : children) if (child) child->Load(); }

	// Deallocates all the resources of the light, including its children if the light is composite
	void DeallocateResources() override
	{
		for (const auto& child : children) if (child) child->DeallocateResources();
	}

	// Draws the meshes of the gizmos representing the light (if any),
	// as the light itself has no visual representation
	virtual void Draw() const override;
	// Draws the meshes of the gizmos representing the light (if any) with the specified shader,
	// as the light itself has no visual representation
	virtual void Draw(const Shader& shader) const override;

	// Getters
	glm::vec3 GetAmbient() const { return ambient; }
	glm::vec3 GetDiffuse() const { return diffuse; }
	glm::vec3 GetSpecular() const { return specular; }
	LightType GetLightType() const { return lightType; }
	std::shared_ptr<Node> GetGizmo() const;

	// Setters
	void SetAmbient(glm::vec3 ambient) { this->ambient = ambient; }
	void SetDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void SetSpecular(glm::vec3 specular) { this->specular = specular; }
	void SetLightType(LightType type) { this->lightType = type; }

	// Create the gizmo for the light
	virtual void CreateGizmo() = 0;
	// Syncronize gizmo's diffuse color with the light's diffuse color
	void SyncGizmoColorFromLight();

protected:
	// Protected Attributes
	// --------------------
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	LightType lightType; // type of the light (e.g., DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT)

};