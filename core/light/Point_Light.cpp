/*
 * Point_Light.cpp
 * This file implements the Point_Light class (a derived class of Light),
 * which is used to create a point light source.
 */

#include "Point_Light.h"

#include "../model/Shape_Model.h" // to create the gizmo for the Point_Light
#include "../gizmos/DECAHEDRON.h" // the Point Light gizmo is a decahedron

// Static Private Attributes
// -------------------------
GLuint Point_Light::point_light_count{}; // initialize the number of point lights in the scene to 0

// Constructors
// ------------
Point_Light::Point_Light(
    const std::string& name,
    const glm::vec3    ambient,
    const glm::vec3    diffuse,
    const glm::vec3    specular,
    const glm::vec3    position,
    const GLfloat      constant,
    const GLfloat      linear,
    const GLfloat      quadratic) :
    Light(
        name,
        ambient,
        diffuse,
        specular,
        nullptr,                  // no gizmo model is provided at this point
        Light_Type::POINT_LIGHT), // set the light type to point light
    position{ position },
    constant{ constant },
    linear{ linear },
    quadratic{ quadratic }
{
	point_light_count++;
}

// Public Methods
// --------------
void Point_Light::set_position(glm::vec3 position)
{
	this->position = position;
	// update gizmo position based on the light position
	auto gizmo = get_gizmo();
	if (gizmo)
		gizmo->set_position(position);
}

void Point_Light::create_gizmo()
{
	// create a decahedron shape for the point light gizmo
	auto gizmo = std::make_shared<Shape_Model>(
	    name + " Gizmo", // set the gizmo's name based on the light's name
	    decahedron_vertices_vector,
	    decahedron_indices_vector,
	    glm::vec4(diffuse, 1.0f),            // set the color of the gizmo to the light's diffuse color
	    position,                            // set the position of the gizmo to the light's position
	    glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, // set the orientation of the gizmo to identity quaternion
	    GIZMO_SCALE                          // set the scale of the gizmo to a predefined constant
	);

	// set the gizmo's type to POINT_LIGHT (used for rendering and interaction purposes)
	gizmo->set_gizmo_type(Gizmo_Type::POINT_LIGHT);

	// add the gizmo as a child node of the light
	add_child(gizmo);
}

void Point_Light::sync_gizmo_position_from_light()
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->set_position(position); // set the gizmo's position to the light's position
}