/*
* Spotlight.h
* This file defines the Spotlight class (a derived class of Light),
* which is used to create a spotlight source.
*/

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include "Light.h"

class Spotlight : public Light
{
public:
	// Constructors
	// ------------
	Spotlight(const std::string& name,
			  const glm::vec3 ambient = AMBIENT, const glm::vec3 diffuse = DIFFUSE, const glm::vec3 specular = SPECULAR,
			  const glm::vec3 position = POSITION, const glm::vec3 direction = DIRECTION,
			  const GLfloat cutOff = CUT_OFF, const GLfloat outerCutOff = OUTER_CUT_OFF,
			  const GLfloat constant = CONSTANT, const GLfloat linear = LINEAR, const GLfloat quadratic = QUADRATIC);
	// Destructor
	// ----------
	~Spotlight() { nSpotlights--; } // decrements the number of spotlights
	// Public Methods
	// --------------
	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetDirection() const { return direction; }
	GLfloat GetCutOff() const { return cutOff; }
	GLfloat GetOuterCutOff() const { return outerCutOff; }
	GLfloat GetConstant() const { return constant; }
	GLfloat GetLinear() const { return linear; }
	GLfloat GetQuadratic() const { return quadratic; }

	// Setters
	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetDirection(glm::vec3 direction) { this->direction = direction; }
	void SetCutOff(GLfloat cutOff) { this->cutOff = cutOff; }
	void SetOuterCutOff(GLfloat outerCutOff) { this->outerCutOff = outerCutOff; }
	void SetConstant(GLfloat constant) { this->constant = constant; }
	void SetLinear(GLfloat linear) { this->linear = linear; }
	void SetQuadratic(GLfloat quadratic) { this->quadratic = quadratic; }

	// Create the gizmo for the spotlight
	void CreateGizmo() override;
	// Syncronize gizmo's position with the light's position
	void SyncGizmoPositionFromLight() { if (gizmo) gizmo->SetPosition(position); }
	// Syncronize gizmo's direction with the light's direction
	//void SyncGizmoDirectionFromLight() { if (gizmo) gizmo->SetDirection(direction); }

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
	GLfloat cutOff;
	GLfloat outerCutOff;
	GLfloat constant;
	GLfloat linear;
	GLfloat quadratic;

	// Private Static Attributes (for default values)
	// ----------------------------------------------
	static constexpr glm::vec3 POSITION{ 1.0f, 2.0f, 3.0f }, DIRECTION{ -1.0f, -1.0f, -1.0f };
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr GLfloat CONSTANT{ 1.0f }, LINEAR{ 0.09f }, QUADRATIC{ 0.032f };
	static const GLfloat CUT_OFF, OUTER_CUT_OFF;

};

