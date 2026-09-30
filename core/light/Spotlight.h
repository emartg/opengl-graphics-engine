/*
 * Spotlight.h
 * This file defines the Spotlight class (a derived class of Light),
 * which is used to create a spotlight source.
 */

#pragma once

#include "Light.h"

#include <iostream>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // for glm::pi
#include <glm/gtc/quaternion.hpp>

class Spotlight : public Light
{
public:
	// Constructors
	// ------------
	Spotlight(
		const std::string& name,
		const glm::vec3    ambient      = AMBIENT,
		const glm::vec3    diffuse      = DIFFUSE,
		const glm::vec3    specular     = SPECULAR,
		const glm::vec3    position     = POSITION,
		const glm::vec3    direction    = DIRECTION,
		const GLfloat      inner_cutoff = INNER_CUTOFF,
		const GLfloat      outer_cutoff = OUTER_CUTOFF,
		const GLfloat      constant     = CONSTANT,
		const GLfloat      linear       = LINEAR,
		const GLfloat      quadratic    = QUADRATIC);

	// Destructor
	// ----------
	~Spotlight() { spotlight_count--; } // decrements the number of spotlights

	// Public Methods
	// --------------
	// Getters
	glm::vec3 get_position() const { return position; }
	glm::vec3 get_direction() const { return direction; }
	GLfloat   get_inner_cutoff() const { return inner_cutoff; }
	GLfloat   get_outer_cutoff() const { return outer_cutoff; }
	GLfloat   get_constant() const { return constant; }
	GLfloat   get_linear() const { return linear; }
	GLfloat   get_quadratic() const { return quadratic; }

	// Setters
	// Sets the position of the light and updates the gizmo's position accordingly.
	// This method also ensures the gizmo's direction line is updated if there are position changes
	void set_position(glm::vec3 position);
	// Sets the direction of the light without aligning the gizmo (only updates the light's direction).
	// This is useful when the gizmo's rotation is changed (e.g., by the GUI), and the light's direction
	// needs to be updated based on the gizmo's new orientation, without causing gizmo->set_forward()
	// to be called again, which would create a conflict.
	// This method also ensures the gizmo's direction line is updated if there are direction changes
	void set_direction_only(const glm::vec3& direction);
	// Sets the direction of the light and aligns the gizmo with the new direction.
	// This is useful when the light's direction is changed programmatically,
	// and the visual gizmo needs to update its orientation to match.
	// This method also ensures the gizmo's direction line is updated if there are direction changes
	void set_direction_and_align_gizmo(const glm::vec3& direction);
	void set_inner_cutoff(GLfloat inner_cutoff) { this->inner_cutoff = inner_cutoff; }
	void set_outer_cutoff(GLfloat outer_cutoff) { this->outer_cutoff = outer_cutoff; }
	void set_constant(GLfloat constant) { this->constant = constant; }
	void set_linear(GLfloat linear) { this->linear = linear; }
	void set_quadratic(GLfloat quadratic) { this->quadratic = quadratic; }

	// Creates the gizmo for the spotlight
	void create_gizmo() override;
	// Syncronizes gizmo's position with the light's position
	void sync_gizmo_position_from_light();

	// Static Public Functions
	// -----------------------
	static GLuint get_spotlight_count() { return spotlight_count; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint spotlight_count; // number of spotlights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position;
	glm::vec3 direction;
	GLfloat   inner_cutoff;
	GLfloat   outer_cutoff;
	GLfloat   constant;
	GLfloat   linear;
	GLfloat   quadratic;

	// Private Static Attributes
	// -------------------------
	// default values for the spotlight attributes
	static constexpr glm::vec3 POSITION{ 1.0f, 4.5f, -0.5f };
	static constexpr glm::vec3 DIRECTION{ 0.0f, -1.0f, 0.0f }; // -Y axis by default
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr GLfloat   CONSTANT{ 1.0f }, LINEAR{ 0.09f }, QUADRATIC{ 0.032f };
	static const GLfloat       INNER_CUTOFF, OUTER_CUTOFF;
	static constexpr glm::vec3 GIZMO_SCALE{ 0.3f }; // default scale factor for the spotlight gizmo
};
