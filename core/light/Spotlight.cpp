/*
* Spotlight.cpp
* This file defines the Spotlight class (a derived class of Light),
* which is used to create a spotlight source.
*/

#include "Spotlight.h"

#include "../model/Shape.h" // to create the gizmo for the Spotlight
#include "../gizmos/HEX_PYRAMID.h" // the Spotlight gizmo is a hexagonal pyramid shape

// Static Private Attributes
// -------------------------
GLuint Spotlight::nSpotlights{}; // initialize the number of spotlights in the scene to 0

// initial values for the spotlight cut-off angles (they cannot be set directly in the .h file since 
// they are not constexpr - they are const instead of constexpr because they are not known at compile time)
const GLfloat Spotlight::INNER_CUTOFF = glm::cos(glm::radians(12.5f));
const GLfloat Spotlight::OUTER_CUTOFF = glm::cos(glm::radians(15.0f));

// Constructors
// ------------
Spotlight::Spotlight(const std::string& name,
					 const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
					 const glm::vec3 position, const glm::vec3 direction,
					 const GLfloat innerCutOff, const GLfloat outerCutOff,
					 const GLfloat constant, const GLfloat linear, const GLfloat quadratic)
	: Light(name, ambient, diffuse, specular,
			nullptr, // no gizmo model is provided at this point
			LightType::SPOTLIGHT), // set the light type to spotlight
	position{ position }, direction{ direction },
	innerCutOff{ innerCutOff }, outerCutOff{ outerCutOff },
	constant{ constant }, linear{ linear }, quadratic{ quadratic }
{
	CreateGizmo(); // create the gizmo for the spotlight
	nSpotlights++; // increment the number of spotlights
}

// Public Methods
// --------------
void Spotlight::SetPosition(glm::vec3 position)
{
	this->position = position;
	// update gizmo position based on the light position
	gizmo->SetPosition(position);
}
void Spotlight::SetDirectionOnly(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
}
void Spotlight::SetDirectionAndAlignGizmo(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the gizmo's forward direction when the light's direction changes
	gizmo->SetForward(this->direction);
}

void Spotlight::CreateGizmo()
{
	// compute the initial rotation of the gizmo based on the light's direction
	glm::vec3 forward = glm::normalize(direction);
	constexpr glm::vec3 meshForward = HEX_PYRAMID_FORWARD;
	glm::quat meshToZ = glm::angleAxis(
		glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)); // +90° around X
	glm::quat lookAt = glm::quatLookAt(
		forward, glm::vec3(0.0f, 1.0f, 0.0f)); // look at the forward direction, Y is up
	glm::quat rotation = lookAt * meshToZ;

	// create a hexagonal pyramid shape for the spotlight gizmo
	gizmo = std::make_shared<Shape>(
		name + " Gizmo",
		hexPyramidVerticesVec, hexPyramidIndicesVec,
		diffuse, // set the color of the gizmo to the light's diffuse color
		position, // set the position of the gizmo to the light's position
		rotation, // set the rotation of the gizmo based on the light's direction
		GIZMO_SCALE, // set the scale of the gizmo to a predefined constant
		direction, // set the forward direction of the gizmo to the light's direction
		meshForward // set the mesh's forward direction to the local space forward direction
	);

	// set the gizmo's type to SPOTLIGHT (used for rendering and interaction purposes)
	gizmo->SetGizmoType(GizmoType::SPOTLIGHT);
}
