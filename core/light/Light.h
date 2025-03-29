/*
* Light.h
* This file defines the Light class (a derived class of Asset),
* which is an abstract base class used to create a light source.
*/

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include "../Asset.h"

class Light : public Asset
{
public:
	// Constructors
	// ------------
	Light(const std::string& name,
		  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular);

	// Virtual destructor (ensures that derived classes can be deleted properly - polymorphism)
	// ---------------------------------------------------------------------------------------
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
	// Setters
	void SetAmbient(glm::vec3 ambient) { this->ambient = ambient; }
	void SetDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void SetSpecular(glm::vec3 specular) { this->specular = specular; }

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

};