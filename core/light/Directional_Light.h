/*
 * Directional_Light.h
 * This file defines the Directional_Light class (a derived class of Light),
 * which is used to create a directional light source.
 */

#pragma once

#include <iostream>

#include "Light.h"

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // for glm::pi
#include <glm/gtc/quaternion.hpp>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Line;

class Directional_Light : public Light
{
public:
	// Constructors
	// ------------
	Directional_Light(
		const std::string& name,
		const glm::vec3    ambient   = AMBIENT,
		const glm::vec3    diffuse   = DIFFUSE,
		const glm::vec3    specular  = SPECULAR,
		const glm::vec3    position  = POSITION,
		const glm::vec3    direction = DIRECTION);

	// Destructor
	// ----------
	~Directional_Light() { directional_light_count--; }

	// Public Methods
	// --------------
	// Draws the gizmo representing the directional light (if any) and its direction line (if any)
	void draw() const override;
	// Draws the gizmo representing the directional light (if any) and its direction line (if any),
	// with the specified shader
	void draw(const Shader& shader) const override;

	// Getters
	glm::vec3              get_position() const { return position; }
	glm::vec3              get_direction() const { return direction; }
	std::shared_ptr<Line>& get_gizmo_direction_line() { return gizmo_direction_line; }
	GLfloat                get_gizmo_direction_line_length() { return gizmo_direction_line_length; }

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
	void set_gizmo_direction_line(std::shared_ptr<Line> gizmo_direction_line) { this->gizmo_direction_line = gizmo_direction_line; }
	void set_gizmo_direction_line_length(const GLfloat length) { gizmo_direction_line_length = length; }

	// Creates the gizmo for the directional light
	void create_gizmo() override;
	// Syncronizes gizmo's position with the light's position
	void sync_gizmo_position_from_light();
	// Updates the vertices of the gizmo's direction line
	// based on the light's current position and direction
	void update_gizmo_direction_line();

	// Static Public Functions
	// -----------------------
	static GLuint get_directional_light_count() { return directional_light_count; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint directional_light_count; // number of directional lights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position; // for directional lights, position is used just for the gizmo (no lighting impact)
	glm::vec3 direction;

	std::shared_ptr<Line> gizmo_direction_line; // the gizmo representing the direction of the light
	GLfloat               gizmo_direction_line_length{ 1.5f };

	// Private Static Attributes
	// -------------------------
	// default values for the directional light attributes
	static constexpr glm::vec3 POSITION{ -3.5f, 7.0f, 0.0f };
	static constexpr glm::vec3 DIRECTION{ 0.0f, 1.0f, 0.0f }; // +Y axis by default
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr glm::vec3 GIZMO_SCALE{ 0.4f }; // default scale factor for the directional light gizmo
};
