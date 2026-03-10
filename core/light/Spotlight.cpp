/*
* Spotlight.cpp
* This file defines the Spotlight class (a derived class of Light),
* which is used to create a spotlight source.
*/

#include "Spotlight.h"

#include "../model/Shape_Model.h" // to create the gizmo for the Spotlight
#include "../gizmos/HEX_PYRAMID.h" // the Spotlight gizmo is a hexagonal pyramid

// Static Private Attributes
// -------------------------
GLuint Spotlight::spotlight_count{}; // initialize the number of spotlights in the scene to 0

// initial values for the spotlight cut-off angles (they cannot be set directly in the .h file since 
// they are not constexpr - they are const instead of constexpr because they are not known at compile time)
const GLfloat Spotlight::INNER_CUTOFF = glm::cos(glm::radians(12.5f));
const GLfloat Spotlight::OUTER_CUTOFF = glm::cos(glm::radians(15.0f));

// Constructors
// ------------
Spotlight::Spotlight(const std::string& name,
					 const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
					 const glm::vec3 position, const glm::vec3 direction,
					 const GLfloat inner_cutoff, const GLfloat outer_cutoff,
					 const GLfloat constant, const GLfloat linear, const GLfloat quadratic)
	: Light(name, ambient, diffuse, specular,
			nullptr, // no gizmo model is provided at this point
			Light_Type::SPOTLIGHT), // set the light type to spotlight
	position{ position }, direction{ direction },
	inner_cutoff{ inner_cutoff }, outer_cutoff{ outer_cutoff },
	constant{ constant }, linear{ linear }, quadratic{ quadratic }
{
	spotlight_count++;
}

// Public Methods
// --------------
void Spotlight::set_position(glm::vec3 position)
{
	this->position = position;
	// update gizmo position based on the light position
	auto gizmo = get_gizmo();
	if (gizmo) gizmo->set_position(position);
}
void Spotlight::set_direction_only(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
}
void Spotlight::set_direction_and_align_gizmo(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the gizmo's forward direction when the light's direction changes
	auto gizmo = get_gizmo();
	if (gizmo) gizmo->set_forward(this->direction);
}

void Spotlight::create_gizmo()
{
	// compute the initial rotation of the gizmo based on the light's direction
	glm::vec3 forward = glm::normalize(direction);
	constexpr glm::vec3 mesh_forward = HEX_PYRAMID_FORWARD;
	glm::quat mesh_to_z = glm::angleAxis(
		glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)); // +90° around X
	glm::quat look_at = glm::quatLookAt(
		forward, glm::vec3(0.0f, 1.0f, 0.0f)); // look at the forward direction, Y is up
	glm::quat rotation = look_at * mesh_to_z;

	// create a hexagonal pyramid shape for the spotlight gizmo
	auto gizmo = std::make_shared<Shape_Model>(
		name + " Gizmo", // set the gizmo's name based on the light's name
		hex_pyramid_vertices_vector, hex_pyramid_indices_vector,
		glm::vec4(diffuse, 1.0f), // set the color of the gizmo to the light's diffuse color
		position, // set the position of the gizmo to the light's position
		rotation, // set the rotation of the gizmo based on the light's direction
		GIZMO_SCALE, // set the scale of the gizmo to a predefined constant
		direction, // set the forward direction of the gizmo to the light's direction
		mesh_forward // set the mesh's forward direction to the local space forward direction
	);

	// set the gizmo's type to SPOTLIGHT (used for rendering and interaction purposes)
	gizmo->set_gizmo_type(Gizmo_Type::SPOTLIGHT);

	// add the gizmo as a child node of the light
	add_child(gizmo);
}

void Spotlight::sync_gizmo_position_from_light()
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo) gizmo->set_position(position); // set the gizmo's position to the light's position
}
