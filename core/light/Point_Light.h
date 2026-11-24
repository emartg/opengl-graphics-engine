/*
 * Point_Light.h
 * This file defines the Point_Light class (a derived class of Light),
 * which is used to create a point light source.
 */

#pragma once

#include "Light.h"

#include <iostream>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

class Point_Light : public Light
{
public:
	// Constructors
	// ------------
	Point_Light(
		const std::string& name,
		const glm::vec3    ambient   = AMBIENT,
		const glm::vec3    diffuse   = DIFFUSE,
		const glm::vec3    specular  = SPECULAR,
		const glm::vec3    position  = POSITION,
		const GLfloat      constant  = CONSTANT,
		const GLfloat      linear    = LINEAR,
		const GLfloat      quadratic = QUADRATIC);

	// Destructor
	// ----------
	~Point_Light() { point_light_count--; }

	// Public Methods
	// --------------
	// Getters
	// The position is relative to the parent of the light (like the position of a node), and the position in world space
	// follows the parent
	glm::vec3 get_position() const { return position; }
	glm::vec3 get_position_in_world() const { return to_world_point(position); }
	GLfloat   get_constant() const { return constant; }
	GLfloat   get_linear() const { return linear; }
	GLfloat   get_quadratic() const { return quadratic; }
	// Returns the distance, in world units, that the light reaches by its linear term (4.5 / linear, the inverse of set_range), or
	// infinity when the linear term is 0
	GLfloat get_range() const;

	// Setters
	void set_position(glm::vec3 position);
	void set_constant(GLfloat constant) { this->constant = constant; }
	void set_linear(GLfloat linear) { this->linear = linear; }
	void set_quadratic(GLfloat quadratic) { this->quadratic = quadratic; }
	// Sets the linear and quadratic terms of the attenuation for a light that reaches about the given distance, in world units
	// (Light::get_attenuation_for_range; the constant term is 1), and returns true. Returns false and keeps the current terms when
	// the range is not finite or not greater than 0. The default terms are those of a range of about 50, for a scene in metres
	bool set_range(GLfloat range);

	// Creates the gizmo for the point light
	void create_gizmo() override;
	// Syncronizes gizmo's position with the light's position
	void sync_gizmo_position_from_light();

	// Static Public Functions
	// -----------------------
	static GLuint get_point_light_count() { return point_light_count; }

protected:
	// Protected Methods
	// -----------------
	// Applies a change of space to the position of the light
	void transform_light(const glm::mat4& change) override;

private:
	// Static Private Attributes
	// -------------------------
	static GLuint point_light_count; // number of point lights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position;
	GLfloat   constant;
	GLfloat   linear;
	GLfloat   quadratic;

	// Private Static Attributes
	// -------------------------
	// default values for the point light attributes
	static constexpr glm::vec3 POSITION{ 1.0f, 2.0f, 3.0f };
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr GLfloat   CONSTANT{ 1.0f }, LINEAR{ 0.09f }, QUADRATIC{ 0.032f };
	static constexpr glm::vec3 GIZMO_SCALE{ 0.3f }; // default scale factor for the point light gizmo
};
