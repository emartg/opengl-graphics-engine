/*
 * Light.h
 * This file defines the Light class (a derived class of Node),
 * which is an abstract base class used to create a light source.
 */

#pragma once

#include "../Node.h"

#include <iostream>
#include <string>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Enumeration for the different types of lights
enum class Light_Type
{
	UNDEFINED = 0,
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOTLIGHT
};

class Light : public Node
{
public:
	// Constructors
	// ------------
	Light(
		const std::string&          name,
		const glm::vec3             ambient,
		const glm::vec3             diffuse,
		const glm::vec3             specular,
		const std::shared_ptr<Node> gizmo = nullptr,
		const Light_Type            type  = Light_Type::UNDEFINED);

	// Virtual destructor
	// ------------------
	virtual ~Light() = default;

	// Public Methods
	// --------------
	// Loads the light and its resources, including its children if the light is composite
	void load() override
	{
		for (const auto& child : children)
			if (child)
				child->load();
	}

	// Deallocates all the resources of the light, including its children if the light is composite
	void deallocate_resources() override
	{
		for (const auto& child : children)
			if (child)
				child->deallocate_resources();
	}

	// Draws the meshes of the gizmos representing the light (if any),
	// as the light itself has no visual representation
	virtual void draw() const override;
	// Draws the meshes of the gizmos representing the light (if any) with the specified shader,
	// as the light itself has no visual representation
	virtual void draw(const Shader& shader) const override;

	// Getters
	glm::vec3             get_ambient() const { return ambient; }
	glm::vec3             get_diffuse() const { return diffuse; }
	glm::vec3             get_specular() const { return specular; }
	Light_Type            get_light_type() const { return light_type; }
	std::shared_ptr<Node> get_gizmo() const;
	// Returns the scale of the gizmo, in world units (its size: a scale of 1 is a gizmo about 1 unit across)
	glm::vec3 get_gizmo_scale() const { return gizmo_scale; }

	// Setters
	void set_ambient(glm::vec3 ambient) { this->ambient = ambient; }
	void set_diffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void set_specular(glm::vec3 specular) { this->specular = specular; }
	void set_light_type(Light_Type type) { light_type = type; }
	// Sets the scale of the gizmo, in world units, and returns true: the gizmo takes it if it exists, and create_gizmo uses it
	// otherwise. Returns false and keeps the current scale when a component is not finite or not greater than 0. An application
	// whose scene is very large or very small sets a size that fits it (the default is for a scene in metres)
	bool set_gizmo_scale(const glm::vec3& scale);

	// Keeps the light where it is in the world when it is given a new parent: its position and direction (which are relative to its
	// parent, like a transform) are expressed relative to the new parent, and so is its gizmo, while the transform of the light
	// itself stays as it is (the identity, unless an application set one)
	void rebase_transform(const glm::mat4& old_parent_world, const glm::mat4& new_parent_world) override;

	// create the gizmo for the light
	virtual void create_gizmo() = 0;
	// Syncronize gizmo's diffuse color with the light's diffuse color
	void sync_gizmo_color_from_light();

	// Public Static Methods
	// ---------------------
	// Returns the linear (x) and quadratic (y) terms of the attenuation of a point light or a spotlight that reaches about the
	// given distance, in world units (linear = 4.5 / range, quadratic = 75 / range squared, the formula behind the usual table of
	// attenuation terms; the constant term is 1). The range must be finite and greater than 0
	static glm::vec2 get_attenuation_for_range(GLfloat range);

protected:
	// Protected Methods
	// -----------------
	// Converts a point or a direction given relative to the parent of the light (the space of its position and direction) to
	// world space, so that a light follows its parent; the direction is normalized
	glm::vec3 to_world_point(const glm::vec3& point) const;
	glm::vec3 to_world_direction(const glm::vec3& direction) const;

	// Applies a change of space (an affine matrix) to the position and the direction of the light, if it has them
	virtual void transform_light(const glm::mat4& change) = 0;

	// Protected Attributes
	// --------------------
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	Light_Type light_type; // type of the light (e.g., DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT)

	glm::vec3 gizmo_scale{ 1.0f }; // scale of the gizmo (each kind of light sets its own default in its constructor)
};
