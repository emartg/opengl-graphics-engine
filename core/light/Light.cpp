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
	// a light can be dragged into a hierarchy (it follows its parent), and it cannot be a parent itself (its only child is its gizmo)
	Node(name, Node_Type::LIGHT, ALBEDO, POSITION, ROTATION, SCALE, FORWARD, FORWARD, Gizmo_Type::NONE, true, false),
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

void Light::rebase_transform(const glm::mat4& old_parent_world, const glm::mat4& new_parent_world)
{
	// the space of the position and the direction of the light (and of its gizmo) under the old and the new parent
	const glm::mat4 old_space{ old_parent_world * get_model_matrix() };
	const glm::mat4 new_space{ new_parent_world * get_model_matrix() };

	// the gizmo first, since setting the position of the light moves the gizmo to it
	if (auto gizmo = get_gizmo())
		gizmo->rebase_transform(old_space, new_space);
	transform_light(glm::inverse(new_space) * old_space);
}

void Light::sync_gizmo_color_from_light()
{
	auto gizmo = get_gizmo(); // get the gizmo representing the light
	if (gizmo)
		gizmo->set_albedo(glm::vec4(diffuse, 1.0f)); // set gizmo's albedo to the light's diffuse color
}

// Protected Methods
// -----------------
glm::vec3 Light::to_world_point(const glm::vec3& point) const
{
	// the world model matrix of the light is that of its parent, unless an application gave the light a transform of its own
	return glm::vec3{ get_world_model_matrix() * glm::vec4{ point, 1.0f } };
}

glm::vec3 Light::to_world_direction(const glm::vec3& direction) const
{
	return glm::normalize(glm::mat3{ get_world_model_matrix() } * direction);
}
