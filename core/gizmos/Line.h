/*
* Line.h
* This file defines a Line class, which is used to represent a line segment in 3D space
* and and provides methods for rendering and manipulating the line.
*/

#pragma once

#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "TRIANGLE_FAN_PLANE.h"

class Line
{
public:
	// Contructors
	// -----------
	Line(const std::vector<GLfloat>& vertices = { triangle_fan_plane_direction_line_vector });

	// Public Methods
	// --------------
	// Deallocates all the resources of the line
	void deallocate_resources();

	// Renders the line
	void draw() const;
	// Updates the vertices of the line
	void update_vertices(const std::vector<GLfloat>& vertices);

private:
	// Private Attributes
	// ------------------
	std::vector<GLfloat> vertices;

	GLuint vao, vbo;

	// Private Methods
	// ---------------
	// Initializes the buffer objects/arrays
	void setup_line(const std::vector<GLfloat>& vertices);

};