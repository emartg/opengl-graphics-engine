/*
* DirectionalLight.h
* This file defines the DirectionalLight class (a derived class of Light),
* which is used to create a directional light source.
*/

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // for glm::pi
#include <glm/gtc/quaternion.hpp>

#include "Light.h"
#include "../gizmos/Line.h"

class DirectionalLight : public Light
{
public:
	// Constructors
	// ------------
	DirectionalLight(const std::string& name,
					 const glm::vec3 ambient = AMBIENT,
					 const glm::vec3 diffuse = DIFFUSE,
					 const glm::vec3 specular = SPECULAR,
					 const glm::vec3 position = POSITION, const glm::vec3 direction = DIRECTION);

	// Destructor
	// ----------
	~DirectionalLight() { nDirectionalLights--; } // decrements the number of directional lights

	// Public Methods
	// --------------
	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetDirection() const { return direction; }
	std::shared_ptr<Line>& GetGizmoDirectionLine() { return gizmoDirectionLine; }
	GLfloat GetGizmoDirectionLineLength(const GLfloat length) { return gizmoDirectionLineLength; }

	// Setters
	// Sets the position of the light and updates the gizmo's position accordingly.
	// This method also ensures the gizmo's direction line is updated if there are position changes
	void SetPosition(glm::vec3 position);
	// Sets the direction of the light without aligning the gizmo (only updates the light's direction).
	// This is useful when the gizmo's rotation is changed (e.g., by the GUI), and the light's direction
	// needs to be updated based on the gizmo's new orientation, without causing gizmo->SetForward() 
	// to be called again, which would create a conflict.
	// This method also ensures the gizmo's direction line is updated if there are direction changes
	void SetDirectionOnly(const glm::vec3& direction);
	// Sets the direction of the light and aligns the gizmo with the new direction.
	// This is useful when the light's direction is changed programmatically, 
	// and the visual gizmo needs to update its orientation to match.
	// This method also ensures the gizmo's direction line is updated if there are direction changes
	void SetDirectionAndAlignGizmo(const glm::vec3& direction);
	void SetGizmoDirectionLine(std::shared_ptr<Line> gizmoDirectionLine)
	{
		this->gizmoDirectionLine = gizmoDirectionLine;
	}
	void SetGizmoDirectionLineLength(const GLfloat length) { gizmoDirectionLineLength = length; }

	// Creates the gizmo for the directional light
	void CreateGizmo() override;
	// Syncronizes gizmo's position with the light's position
	void SyncGizmoPositionFromLight() { gizmo->SetPosition(position); }
	// Updates the vertices of the gizmo's direction line 
	// based on the light's current position and direction
	void UpdateGizmoDirectionLine();

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

	std::shared_ptr<Line> gizmoDirectionLine; // the gizmo representing the direction of the light
	GLfloat gizmoDirectionLineLength{ 1.5f };

	// Private Static Attributes
	// -------------------------
	// default values for the directional light attributes
	static constexpr glm::vec3 POSITION{ -3.5f, 7.0f, 0.0f };
	static constexpr glm::vec3 DIRECTION{ 0.0f, 1.0f, 0.0f }; // +Y axis by default
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr glm::vec3 GIZMO_SCALE{ 0.4f }; // default scale factor for the directional light gizmo

};

