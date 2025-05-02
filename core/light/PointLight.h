/*
* PointLight.h
* This file defines the PointLight class (a derived class of Light),
* which is used to create a point light source.
*/

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include "Light.h"

class PointLight : public Light
{
public:
	// Constructors
	// ------------
	PointLight(const std::string& name,
			   const glm::vec3 ambient = AMBIENT, const glm::vec3 diffuse = DIFFUSE, const glm::vec3 specular = SPECULAR,
			   const glm::vec3 position = POSITION,
			   const GLfloat constant = CONSTANT, const GLfloat linear = LINEAR, const GLfloat quadratic = QUADRATIC);

	// Destructor
	// ----------
	~PointLight() { nPointLights--; } // decrements the number of point lights

	// Public Methods
	// --------------
	// Getters
	glm::vec3 GetPosition() const { return position; }
	GLfloat GetConstant() const { return constant; }
	GLfloat GetLinear() const { return linear; }
	GLfloat GetQuadratic() const { return quadratic; }

	// Setters
	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetConstant(GLfloat constant) { this->constant = constant; }
	void SetLinear(GLfloat linear) { this->linear = linear; }
	void SetQuadratic(GLfloat quadratic) { this->quadratic = quadratic; }

	// Create the gizmo for the point light
	void CreateGizmo() override;
	// Syncronize gizmo's position with the light's position
	void SyncGizmoPositionFromLight() { if (gizmo) gizmo->SetPosition(position); }

	// Static Public Functions
	// -----------------------
	static GLuint GetNPointLights() { return nPointLights; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nPointLights; // number of point lights in the scene

	// Private Attributes
	// ------------------
	glm::vec3 position;
	GLfloat constant;
	GLfloat linear;
	GLfloat quadratic;

	// Private Static Attributes (for default values)
	// ----------------------------------------------
	static constexpr glm::vec3 POSITION{ 1.0f, 2.0f, 3.0f };
	static constexpr glm::vec3 AMBIENT{ 0.1f }, DIFFUSE{ 0.8f }, SPECULAR{ 1.0f };
	static constexpr GLfloat CONSTANT{ 1.0f }, LINEAR{ 0.09f }, QUADRATIC{ 0.032f };

};