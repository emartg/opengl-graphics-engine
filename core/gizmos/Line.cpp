/*
* Line.cpp
* This file implements the Line class, which is used to represent a line in 3D space
* and provides methods for rendering and manipulating the line.
*/

#include <stdexcept>

#include "Line.h"

// Constructors
// ------------
Line::Line(const std::vector<GLfloat>& vertices)
{
	if (vertices.size() != 6) // check if the vector has exactly 6 elements (3 floats for each endpoint)
		throw std::invalid_argument("Line constructor requires exactly 6 float values for two endpoints.");

	this->vertices = vertices;
	setupLine();
}

Line::Line(const GLfloat* vertices)
{
	if (vertices == nullptr) // check if the pointer is valid
		throw std::invalid_argument("Line constructor requires a valid pointer to an array of 6 float values.");

	// assume the pointer points to an array of 6 floats (3 for each endpoint),
	// i.e., that the caller has ensured that the array is of the correct size,
	// this way we can avoid explicitly checking the size here

	this->vertices.assign(vertices, vertices + 6);
	setupLine();
}

// Public Methods
// --------------
void Line::Draw() const
{
	// bind the VAO and draw the line
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	// unbind the the VAO
	glBindVertexArray(0);
}

void Line::DeallocateResources()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

// Private Methods
// ---------------
void Line::setupLine()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// bind the VAO
	glBindVertexArray(VAO);

	// bind the VBO and send the vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}