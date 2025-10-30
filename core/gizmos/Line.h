/*
* Line.h
* This file defines a Line class, which is used to represent a line segment in 3D space
* and and provides methods for rendering and manipulating the line.
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "TRIANGLE_FAN_PLANE.h"

class Line
{
public:
	// Contructors
	// -----------
	Line(const std::vector<GLfloat>& vertices = { triangleFanPlaneDirectionLineVec });

	// Public Methods
	// --------------
	// Renders the line
	void Draw() const;
	// Updates the vertices of the line
	void UpdateVertices(const std::vector<GLfloat>& vertices);

	// Deallocates all the resources of the line
	void DeallocateResources();

private:
	// Private Attributes
	// ------------------
	std::vector<GLfloat> vertices;

	GLuint VAO, VBO;

	// Private Methods
	// ---------------
	// Initializes the buffer objects/arrays
	void setupLine(const std::vector<GLfloat>& vertices);

};