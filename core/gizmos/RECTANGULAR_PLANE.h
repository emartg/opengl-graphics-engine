/*
* RECTANGULAR_PLANE.h
* This file defines the vertex data for a simple rectangular plane,
* both in array and vector form.
* Vertices are replicated for each triangle, so that each triangle can have
* different normals and texture coordinates.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>
#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the rectangular plane (4 triangles × 3 vertices)
const GLuint nRectangularPlaneVertices = 6, nRectangularPlaneIndices = 12;

// Plane dimensions used in shape construction
constexpr float PLANE_WIDTH = 2.0f;
constexpr float PLANE_HEIGHT = 2.0f;

// Forward direction of the rectangular plane defined below
constexpr glm::vec3 RECTANGULAR_PLANE_FORWARD{ 0.0f, 1.0f, 0.0f }; // facing up along the Y-axis

// Rectangular plane vertices
const GLfloat rectangularPlanePositionsArr[] = {
	-PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,	// bottom-left
	 PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,	// bottom-right
	 PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,	// top-right
	-PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,	// top-left
	 0.0f,				0.0f,	 0.0f               // center
};

// Rectangular plane normal data (all facing the same direction)
const GLfloat rectangularPlaneNormalsArr[] = {
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f	// center
};

// Rectangular plane texture coordinate data
const GLfloat rectangularPlaneTexCoordsArr[] = {
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	0.5f,	0.5f	// center
};

// Rectangular plane index data
const GLuint rectangularPlaneIndicesArr[] = {
	0,	1,	4,	// bottom
	1,	2,	4,	// right
	2,	3,	4,	// top
	3,	0,	4	// left
};

// Interleaved rectangular plane vertex data
const GLfloat rectangularPlaneVerticesArr[] = {
	// position											// normal					// tex coords
	-PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	0.0f,
	 PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	0.0f,
	 PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	1.0f,
	-PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f,

	// center vertex (used for normal direction line)
	 0.0f,				0.0f,	 0.0f,					0.0f,	1.0f,	0.0f,		0.5f,	0.5f
};

// Rectangular plane data in vector form
const std::vector<GLfloat> rectangularPlanePositionsVec{
	std::begin(rectangularPlanePositionsArr), std::end(rectangularPlanePositionsArr)
};
const std::vector<GLfloat> rectangularPlaneNormalsVec{
	std::begin(rectangularPlaneNormalsArr), std::end(rectangularPlaneNormalsArr)
};
const std::vector<GLfloat> rectangularPlaneTexCoordsVec{
	std::begin(rectangularPlaneTexCoordsArr), std::end(rectangularPlaneTexCoordsArr)
};
const std::vector<GLuint> rectangularPlaneIndicesVec{
	std::begin(rectangularPlaneIndicesArr), std::end(rectangularPlaneIndicesArr)
};
const std::vector<GLfloat> rectangularPlaneVerticesVec{
	std::begin(rectangularPlaneVerticesArr), std::end(rectangularPlaneVerticesArr)
};

// Rectangular plane direction line data (the normal direction)
const GLfloat rectangularPlaneDirectionLineArr[] = {
	// start: center
	rectangularPlaneVerticesArr[32],
	rectangularPlaneVerticesArr[33],
	rectangularPlaneVerticesArr[34],

	// end: center + normal direction
	rectangularPlaneVerticesArr[32] + rectangularPlaneVerticesArr[35],
	rectangularPlaneVerticesArr[33] + rectangularPlaneVerticesArr[36],
	rectangularPlaneVerticesArr[34] + rectangularPlaneVerticesArr[37]
};

// Rectangular plane direction line data in vector form
const std::vector<GLfloat> rectangularPlaneDirectionLineVec{
	std::begin(rectangularPlaneDirectionLineArr), std::end(rectangularPlaneDirectionLineArr)
};