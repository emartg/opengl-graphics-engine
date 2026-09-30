/*
 * Directional_Light.cpp
 * This file implements the Directional_Light class (a derived class of Light),
 * which is used to create a directional light source.
 */

#include "Directional_Light.h"

#include "../model/Shape_Model.h"         // to create the gizmo for the Directional_Light
#include "../gizmos/Line.h"               // to create the direction line for the Directional_Light gizmo
#include "../gizmos/TRIANGLE_FAN_PLANE.h" // the Directional_Light gizmo is a triangle fan plane

// Static Protected Attributes
// ---------------------------
GLuint Directional_Light::directional_light_count{};

// Constructors
// ------------
Directional_Light::Directional_Light(
	const std::string& name,
	const glm::vec3    ambient,
	const glm::vec3    diffuse,
	const glm::vec3    specular,
	const glm::vec3    position,
	const glm::vec3    direction) :
	Light(
		name,
		ambient,
		diffuse,
		specular,
		nullptr,                        // no gizmo model is provided at this point
		Light_Type::DIRECTIONAL_LIGHT), // set the light type to directional light
	position{ position },
	direction{ direction },
	gizmo_direction_line{ nullptr }
{
	directional_light_count++;
}

// Public Methods
// --------------
void Directional_Light::draw() const
{
	// draw the gizmo representing the light via the base class method
	Light::draw();

	// draw the direction line specific to directional lights (if it exists)
	if (gizmo_direction_line)
	{
		// Update the line's vertices before drawing to match the light's current position and direction
		// (const_cast is used here because the draw method is const,
		// but the Line class's update_vertices method is non-const)
		const_cast<Directional_Light*>(this)->update_gizmo_direction_line();

		gizmo_direction_line->draw(); // draw the direction line using its own draw method
	}
}

void Directional_Light::draw(const Shader& shader) const
{
	// draw the gizmo representing the light with the specified shader via the base class method
	Light::draw(shader);

	// draw the direction line specific to directional lights (if it exists)
	if (gizmo_direction_line)
	{
		// Update the line's vertices before drawing to match the light's current position and direction
		// (const_cast is used here because the draw method is const,
		// but the Line class's update_vertices method is non-const)
		const_cast<Directional_Light*>(this)->update_gizmo_direction_line();

		gizmo_direction_line->draw(); // draw the direction line using its own draw method
	}
}

void Directional_Light::set_position(glm::vec3 position)
{
	this->position = position;
	// update gizmo position based on the light position
	auto gizmo = get_gizmo();
	if (gizmo)
		gizmo->set_position(position);
	// update the position of the vertices of the direction line
	update_gizmo_direction_line();
}
void Directional_Light::set_direction_only(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the position of the vertices of the direction line
	update_gizmo_direction_line();
}
void Directional_Light::set_direction_and_align_gizmo(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
	// update the gizmo's forward direction when the light's direction changes
	auto gizmo = get_gizmo();
	if (gizmo)
		gizmo->set_forward(this->direction);
	// update the position of the vertices of the direction line
	update_gizmo_direction_line();
}

void Directional_Light::create_gizmo()
{
	// compute the initial rotation of the gizmo based on the light's direction
	glm::vec3           forward      = glm::normalize(direction);
	constexpr glm::vec3 mesh_forward = TF_PLANE_FORWARD;
	glm::quat           mesh_to_z    = glm::angleAxis(glm::half_pi<float>() * -1.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // -90° around X
	glm::quat           look_at      = glm::quatLookAt(forward, glm::vec3(0.0f, 1.0f, 0.0f)); // look at the forward direction, with Y up
	glm::quat           rotation     = look_at * mesh_to_z;

	// create a triangle fan plane gizmo for the directional light gizmo
	auto gizmo = std::make_shared<Shape_Model>(
		name + " Gizmo", // set the name of the gizmo based on the light's name
		triangle_fan_plane_vertices_vector,
		triangle_fan_plane_indices_vector,
		glm::vec4(diffuse, 1.0f), // set the color of the gizmo to the light's diffuse color
		position,                 // set the position of the gizmo to the light's position
		rotation,                 // set the rotation of the gizmo based on the light's direction
		GIZMO_SCALE,              // set the scale of the gizmo to a predefined constant
		direction,                // set the forward direction of the gizmo to the light's direction
		mesh_forward              // set the mesh's forward direction to the local space forward direction
	);

	// set the gizmo's type to DIRECTIONAL_LIGHT (used for rendering and interaction purposes)
	gizmo->set_gizmo_type(Gizmo_Type::DIRECTIONAL_LIGHT);

	// add the gizmo as a child node of the light
	add_child(gizmo);

	// create the line that represents the direction of the light,
	// initializing it with the current position and direction of the light
	glm::vec3            start         = this->position;
	glm::vec3            end           = start + forward * gizmo_direction_line_length;
	std::vector<GLfloat> line_vertices = { start.x, start.y, start.z, end.x, end.y, end.z };
	// the gizmo direction line is only created once and then updated when needed
	gizmo_direction_line = std::make_shared<Line>(line_vertices);
}

void Directional_Light::sync_gizmo_position_from_light()
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->set_position(position); // set the gizmo's position to the light's position
}

void Directional_Light::update_gizmo_direction_line()
{
	glm::vec3 start            = position; // set the new start point
	glm::vec3 normal_direction = glm::normalize(direction);
	glm::vec3 end              = start + normal_direction * gizmo_direction_line_length; // set the new end point
	// create an updated array of vertices with these points' values
	std::vector<GLfloat> line_vertices = { start.x, start.y, start.z, end.x, end.y, end.z };
	// update the direction line's vertices without creating a new line,
	// needlessly deallocating and allocating space in memory
	gizmo_direction_line->update_vertices(line_vertices);
}
