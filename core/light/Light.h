/*
* Light.h
* This file defines the Light class (a derived class of Asset),
* which is an abstract base class used to create a light source.
*/

#pragma once

#include <iostream>
#include <string>
#include <memory> // for smart pointers

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include "../Asset.h"
#include "../model/Model.h" // to allow the light to have a gizmo model

// enumeration for the different types of lights
enum class LightType { UNDEFINED = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class Light : public Asset
{
public:
	// Constructors
	// ------------
	Light(const std::string& name,
		  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
		  const std::shared_ptr<Model> gizmo = nullptr,
		  const LightType type = LightType::UNDEFINED);

	// Virtual destructor
	// ------------------
	virtual ~Light() { nLights--; } // decrements the number of lights

	// Public Methods
	// --------------
	// Loads the light
	virtual void Load() override {}

	// Deallocates all the resources of the light
	virtual void DeallocateResources() override {}

	// Getters
	glm::vec3 GetAmbient() const { return ambient; }
	glm::vec3 GetDiffuse() const { return diffuse; }
	glm::vec3 GetSpecular() const { return specular; }
	LightType GetLightType() const { return lightType; }
	std::shared_ptr<Model>& GetGizmo() { return gizmo; }

	// Setters
	void SetAmbient(glm::vec3 ambient) { this->ambient = ambient; }
	void SetDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void SetSpecular(glm::vec3 specular) { this->specular = specular; }
	void SetLightType(LightType type) { this->lightType = type; }
	void SetGizmo(std::shared_ptr<Model> gizmo) { this->gizmo = gizmo; }

	// Create the gizmo for the light
	virtual void CreateGizmo() = 0;
	// Syncronize gizmo's diffuse color with the light's diffuse color
	void SyncGizmoColorFromLight() { if (gizmo) gizmo->SetAlbedo(diffuse); }

	// Static Public Functions
	// -----------------------
	static GLuint GetNLights() { return nLights; }

protected:
	// Static Protected Attributes
	// ---------------------------
	static GLuint nLights; // number of lights in the scene

	// Protected Attributes
	// --------------------
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	std::shared_ptr<Model> gizmo; // the model used to represent the light in the scene

	LightType lightType; // type of the light (e.g., DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT)

};