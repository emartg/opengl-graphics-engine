/*
* Spotlight.cpp
* This file defines the Spotlight class (a derived class of Light),
* which is used to create a spotlight source.
*/

#include "Spotlight.h"

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
void Spotlight::CreateGizmo()
{
	// create a hexagonal pyramid shape for the spotlight gizmo
	gizmoShape = std::make_shared<Shape>(name + " Gizmo",
										 hexPyramidVerticesVec, hexPyramidIndicesVec,
										 diffuse, // set the color of the gizmo to the light's diffuse color
										 position, // set the position of the gizmo to the light's position
										 direction // set the direction of the gizmo to the light's direction
	);
	// set the default direction to the axis the mesh points to (the negative y-axis)
	gizmoShape->SetDefaultDirection(glm::vec3(DIRECTION));
	// set the gizmo type to SPOTLIGHT (used for rendering and interaction purposes)
	gizmoShape->SetGizmoShapeType(GizmoShapeType::SPOTLIGHT);
}
