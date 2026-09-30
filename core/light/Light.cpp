/*
 * Light.h
 * This file implements the Light class (a derived class of Node),
 * which is an abstract base class used to create a light source.
 */

#include "Light.h"

#include "../texture/Texture.h"

// Constructors
// ------------
Light::Light(
	const std::string&          name,
	const glm::vec3             ambient,
	const glm::vec3             diffuse,
	const glm::vec3             specular,
	const std::shared_ptr<Node> gizmo,
	const Light_Type            light_type) :
	Node(name, Node_Type::LIGHT, ALBEDO, POSITION, ROTATION, SCALE, FORWARD, FORWARD, Gizmo_Type::NONE, false, false),
	ambient{ ambient },
	diffuse{ diffuse },
	specular{ specular },
	light_type{ light_type }
{}

// Public Methods
// --------------
void Light::draw() const
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->draw(); // draw the gizmo if it exists
}

void Light::draw(const Shader& shader) const
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->draw(shader); // draw the gizmo with the specified shader if it exists
}

std::shared_ptr<Node> Light::get_gizmo() const
{
	// return the first child node thta has a non-NONE gizmo type (i.e., the gizmo representing the light)
	for (auto& child : children)
		if (child && child->get_gizmo_type() != Gizmo_Type::NONE)
			return child;
	// otherwise, return nullptr
	return nullptr;
}

void Light::sync_gizmo_color_from_light()
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->set_albedo(glm::vec4(diffuse, 1.0f)); // set gizmo's albedo to the light's diffuse color
}