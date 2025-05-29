/*
* DirectionalLight.h
* This file defines the DirectionalLight class (a derived class of Light),
* which is used to create a directional light source.
*/

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include "Light.h"

class DirectionalLight : public Light
{
public:
	// Constructors
	// ------------
	DirectionalLight(const std::string& name,
					 const glm::vec3 ambient = AMBIENT, const glm::vec3 diffuse = DIFFUSE, const glm::vec3 specular = SPECULAR,
					 const glm::vec3 position = POSITION, const glm::vec3 direction = DIRECTION);

	// Destructor
	// ----------
	~DirectionalLight() { nDirectionalLights--; } // decrements the number of directional lights

	// Public Methods
	// --------------
	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetDirection() const { return direction; }

	// Setters
	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetDirection(glm::vec3 direction) { this->direction = direction; }

	// Create the gizmo for the directional light
	void CreateGizmo() override;
	// Syncronize gizmo's position with the light's position
	void SyncGizmoPositionFromLight() { if (gizmoShape) gizmoShape->SetPosition(position); }
	// Syncronize gizmo's direction with the light's direction
	void SyncGizmoDirectionFromLight() { if (gizmoShape) gizmoShape->SetDirection(direction); }

	// Static Public Functions
	// -----------------------
	static GLuint GetNDirectionalLights() { return nDirectionalLights; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nDirectionalLights; // number of directional lights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position; // for directional lights, position is used just for the gizmo (no lighting impact)
	glm::vec3 direction;

	// Private Static Attributes (for default values)
	// ----------------------------------------------
	static constexpr glm::vec3 POSITION{ -3.5f, 7.0f, 0.0f };
	static constexpr glm::vec3 DIRECTION{ 0.0f, -1.0f, 0.0f }; // default direction is -Y axis
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };

};

