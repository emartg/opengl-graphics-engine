/*
* PointLight.cpp
* This file implements the PointLight class (a derived class of Light),
* which is used to create a point light source.
*/

#include "PointLight.h"

#include "../gizmos/HEX_PYRAMID.h" // the Point Light gizmo is a hex pyramid

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
	CreateGizmo(); // create the gizmo for the point light
	nPointLights++; // increment the number of point lights
}

// Public Methods  
// --------------
void PointLight::CreateGizmo()
{
	// create a hexagonal pyramid shape for the point light gizmo  
	gizmo = std::make_shared<Shape>(name + " Gizmo",
									hexPyramidVerticesVec, hexPyramidIndicesVec,
									diffuse, // set the color of the gizmo to the light's diffuse color   
									position); // set the position of the gizmo to the light's position  
}