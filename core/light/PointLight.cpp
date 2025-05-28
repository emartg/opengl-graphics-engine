/*
* PointLight.cpp
* This file implements the PointLight class (a derived class of Light),
* which is used to create a point light source.
*/

#include "PointLight.h"

#include "../gizmos/DECAHEDRON.h" // the Point Light gizmo is a decahedron shape

// Static Private Attributes
// -------------------------
GLuint PointLight::nPointLights{}; // initialize the number of point lights in the scene to 0

// Constructors
// ------------
PointLight::PointLight(const std::string& name,
					   const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
					   const glm::vec3 position,
					   const GLfloat constant, const GLfloat linear, const GLfloat quadratic)
	: Light(name, ambient, diffuse, specular,
			LightType::POINT_LIGHT), // set the light type to point light
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
	// create a decahedron shape for the point light gizmo  
	gizmoShape = std::make_shared<Shape>(name + " Gizmo",
										 decahedronVerticesVec, decahedronIndicesVec,
										 diffuse, // set the color of the gizmo to the light's diffuse color   
										 position); // set the position of the gizmo to the light's position
	// set the gizmo shape's type to POINT_LIGHT (used for rendering and interaction purposes)
	gizmoShape->SetGizmoShapeType(GizmoShapeType::POINT_LIGHT);
}