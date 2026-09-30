/*
 * Line.cpp
 * This file implements the Line class, which is used to represent a line in 3D space
 * and provides methods for rendering and manipulating the line.
 */

#include "Line.h"

// Constructors
// ------------
Line::Line(const std::vector<GLfloat>& vertices)
{
	// check if the vector is empty or has the correct size (empty vector allows for deferred update)
	if (vertices.size() != 6 && !vertices.empty())
	{
		throw std::invalid_argument("Line constructor requires 6 float values for two endpoints, or be empty for deferred update.");
	}

	setup_line(vertices);
}

// Public Methods
// --------------
void Line::deallocate_resources()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Line::draw() const
{
	// bind the VAO and draw the line
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, 2);

	// unbind the the VAO
	glBindVertexArray(0);
}

void Line::update_vertices(const std::vector<GLfloat>& vertices)
{
	// check if the new vector has the correct size
	if (vertices.size() != 6)
		throw std::invalid_argument("Line::update_vertices requires 6 float values for two endpoints.");

	// bind the VBO and update the vertices with the new data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());

	// unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Private Methods
// ---------------
void Line::setup_line(const std::vector<GLfloat>& vertices)
{
	// create buffers/arrays
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// bind the VAO
	glBindVertexArray(vao);

	// bind the VBO and send the vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// use GL_DYNAMIC_DRAW to allow for dynamic updates of the vertex data
	// (this is useful for lines that may change frequently), and ensure
	// that, if the vector is empty, there is still space reserved for two points,
	// which is 6 floats (2 points * 3 coordinates each)
	if (!vertices.empty())
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
	else
		// if the vector is empty, reserve space for two points (6 floats)
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6, nullptr, GL_DYNAMIC_DRAW);

	// set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
