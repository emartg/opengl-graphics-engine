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
	Spotlight(const std::string& name,
			  const glm::vec3 ambient = AMBIENT,
			  const glm::vec3 diffuse = DIFFUSE,
			  const glm::vec3 specular = SPECULAR,
			  const glm::vec3 position = POSITION, const glm::vec3 direction = DIRECTION,
			  const GLfloat innerCutOff = INNER_CUTOFF, const GLfloat outerCutOff = OUTER_CUTOFF,
			  const GLfloat constant = CONSTANT,
			  const GLfloat linear = LINEAR,
			  const GLfloat quadratic = QUADRATIC);

	// Destructor
	// ----------
	~Spotlight() { nSpotlights--; } // decrements the number of spotlights

	// Public Methods
	// --------------
	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetDirection() const { return direction; }
	GLfloat GetInnerCutOff() const { return innerCutOff; }
	GLfloat GetOuterCutOff() const { return outerCutOff; }
	GLfloat GetConstant() const { return constant; }
	GLfloat GetLinear() const { return linear; }
	GLfloat GetQuadratic() const { return quadratic; }

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
	void SetInnerCutOff(GLfloat innerCutOff) { this->innerCutOff = innerCutOff; }
	void SetOuterCutOff(GLfloat outerCutOff) { this->outerCutOff = outerCutOff; }
	void SetConstant(GLfloat constant) { this->constant = constant; }
	void SetLinear(GLfloat linear) { this->linear = linear; }
	void SetQuadratic(GLfloat quadratic) { this->quadratic = quadratic; }

	// Creates the gizmo for the spotlight
	void CreateGizmo() override;
	// Syncronizes gizmo's position with the light's position
	void SyncGizmoPositionFromLight();

	// Static Public Functions
	// -----------------------
	static GLuint GetNSpotlights() { return nSpotlights; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nSpotlights; // number of spotlights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position;
	glm::vec3 direction;
	GLfloat innerCutOff;
	GLfloat outerCutOff;
	GLfloat constant;
	GLfloat linear;
	GLfloat quadratic;

	// Private Static Attributes
	// -------------------------
	// default values for the spotlight attributes
	static constexpr glm::vec3 POSITION{ 1.0f , 4.5f , -0.5f };
	static constexpr glm::vec3 DIRECTION{ 0.0f, -1.0f, 0.0f }; // -Y axis by default
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr GLfloat CONSTANT{ 1.0f }, LINEAR{ 0.09f }, QUADRATIC{ 0.032f };
	static const GLfloat INNER_CUTOFF, OUTER_CUTOFF;
	static constexpr glm::vec3 GIZMO_SCALE{ 0.3f }; // default scale factor for the spotlight gizmo

};

