/*
* DirectionalLight.cpp
* This file implements the DirectionalLight class (a derived class of Light),
* which is used to create a directional light source.
*/

#include "DirectionalLight.h"

#include "../model/Shape.h" // to create the gizmo for the DirectionalLight
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
			nullptr, // no gizmo model is provided at this point
			LightType::DIRECTIONAL_LIGHT), // set the light type to directional light
	position{ position }, direction{ direction },
	gizmoDirectionLine{ nullptr }
{
	CreateGizmo(); // create the gizmo for the spotlight
	nDirectionalLights++; // increment the number of directional lights
}

// Public Methods
// --------------
void DirectionalLight::SetPosition(glm::vec3 position)
{
	this->position = position;
	// update gizmo position based on the light position
	gizmo->SetPosition(position);
	// update the position of the vertices of the direction line
	UpdateGizmoDirectionLine();
}
void DirectionalLight::SetDirectionOnly(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the position of the vertices of the direction line
	UpdateGizmoDirectionLine();
}
void DirectionalLight::SetDirectionAndAlignGizmo(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the gizmo's forward direction when the light's direction changes
	gizmo->SetForward(this->direction);
	// update the position of the vertices of the direction line
	UpdateGizmoDirectionLine();
}

void DirectionalLight::CreateGizmo()
{
	// compute the initial rotation of the gizmo based on the light's direction
	glm::vec3 forward = glm::normalize(direction);
	constexpr glm::vec3 meshForward = RECTANGULAR_PLANE_FORWARD;
	glm::quat meshToZ = glm::angleAxis(glm::half_pi<float>() * -1.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // -90° around X
	glm::quat lookAt = glm::quatLookAt(forward, glm::vec3(0.0f, 1.0f, 0.0f)); // look at the forward direction, with Y up
	glm::quat rotation = lookAt * meshToZ;

	// create a rectangular plane gizmo for the directional light gizmo
	gizmo = std::make_shared<Shape>(name + " Gizmo",
									rectangularPlaneVerticesVec, rectangularPlaneIndicesVec,
									diffuse, // set the color of the gizmo to the light's diffuse color
									position, // set the position of the gizmo to the light's position
									rotation, // set the rotation of the gizmo based on the light's direction
									GIZMO_SCALE, // set the scale of the gizmo to a predefined constant
									direction, // set the forward direction of the gizmo to the light's direction
									meshForward // set the mesh's forward direction to the local space forward direction
	);

	// set the gizmo's type to DIRECTIONAL_LIGHT (used for rendering and interaction purposes)
	gizmo->SetGizmoType(GizmoType::DIRECTIONAL_LIGHT);

	// create the line that represents the direction of the light,
	// initializing it with the current position and direction of the light
	glm::vec3 start = this->position;
	glm::vec3 end = start + forward * gizmoDirectionLineLength;
	std::vector<GLfloat> lineVertices = {
		start.x, start.y, start.z,
		end.x,   end.y,   end.z
	};
	// the gizmo direction line is only created once and then updated when needed
	gizmoDirectionLine = std::make_shared<Line>(lineVertices);
}

void DirectionalLight::UpdateGizmoDirectionLine()
{
	glm::vec3 start = position; // set the new start point
	glm::vec3 normDirection = glm::normalize(direction);
	glm::vec3 end = start + normDirection * gizmoDirectionLineLength; // set the new end point
	// create an updated array of vertices with these points' values
	std::vector<GLfloat> lineVertices = {
		start.x, start.y, start.z,
		end.x,   end.y,   end.z
	};
	// update the direction line's vertices without creating a new line, 
	// needlessly deallocating and allocating space in memory
	gizmoDirectionLine->UpdateVertices(lineVertices);
}