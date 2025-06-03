/*
* DirectionalLight.cpp
* This file implements the DirectionalLight class (a derived class of Light),
* which is used to create a directional light source.
*/

#include "DirectionalLight.h"

#include "../gizmos/RECTANGULAR_PLANE.h"

// Static Protected Attributes
// ---------------------------
GLuint DirectionalLight::nDirectionalLights{};

// Constructors
// ------------
DirectionalLight::DirectionalLight(const std::string& name,
								   const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
								   const glm::vec3 position, const glm::vec3 direction)
	: Light(name, ambient, diffuse, specular,
			LightType::DIRECTIONAL_LIGHT), // set the light type to directional light
	position{ position }, direction{ direction },
	gizmoDirectionLine{ nullptr }
{
	CreateGizmo(); // create the gizmo for the spotlight
	nDirectionalLights++; // increment the number of directional lights
}

// Public Methods
// --------------
void DirectionalLight::CreateGizmo()
{
	// create a rectangular plane gizmo for the directional light gizmo
	gizmoShape = std::make_shared<Shape>(name + " Gizmo",
										 rectangularPlaneVerticesVec, rectangularPlaneIndicesVec,
										 diffuse, // set the color of the gizmo to the light's diffuse color
										 position, // set the position of the gizmo to the light's position
										 direction // set the direction of the gizmo to the light's direction
	);
	// set the default direction to the axis the mesh points to (the negative y-axis)
	gizmoShape->SetDefaultDirection(glm::vec3(DIRECTION));
	// set the gizmo type to DIRECTIONAL_LIGHT (used for rendering and interaction purposes)
	gizmoShape->SetGizmoShapeType(GizmoShapeType::DIRECTIONAL_LIGHT);

	// create the line that represents the direction of the light
	gizmoDirectionLine = std::make_shared<Line>(rectangularPlaneDirectionLineVec);
}

void DirectionalLight::SyncGizmoDirectionFromLight()
{
	if (gizmoShape) gizmoShape->SetDirection(direction);
	if (gizmoDirectionLine)
	{
		glm::vec3 start = position;
		glm::vec3 end = position + glm::normalize(direction) * 2.0f; // 2.0f is the line length
		std::vector<GLfloat> lineVertices = {
			start.x, start.y, start.z,
			end.x,   end.y,   end.z
		};
		gizmoDirectionLine = std::make_shared<Line>(lineVertices);
	}
}