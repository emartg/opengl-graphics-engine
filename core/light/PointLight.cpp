/*
* PointLight.cpp
* This file implements the PointLight class (a derived clas of Light),
* which is used to create a point light source.
*/

#include "PointLight.h"

// Static Private Attributes
// -------------------------
GLuint PointLight::nPointLights{}; // initialize the number of point lights in the scene to 0

// Constructors
// ------------
PointLight::PointLight(const std::string& name,
					   const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
					   const glm::vec3 position,
					   const GLfloat constant, const GLfloat linear, const GLfloat quadratic)
	: Light(name, ambient, diffuse, specular),
	position{ position },
	constant{ constant }, linear{ linear }, quadratic{ quadratic }
{
	nPointLights++; // increments the number of point lights
}
