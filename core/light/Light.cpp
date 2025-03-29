/*
* Light.h
* This file implements the Light class (a derived class of Asset),
* which is an abstract base class used to create a light source.
*/

#include "Light.h"

// Static Protected Attributes
// ---------------------------
GLuint Light::nLights{}; // initialize the number of lights in the scene to 0

// Constructors
// ------------
Light::Light(const std::string& name,
			 const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular)
	: Asset(name, AssetType::LIGHT),
	ambient{ ambient }, diffuse{ diffuse }, specular{ specular }
{
	nLights++; // increments the number of lights
}