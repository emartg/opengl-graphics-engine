/*
* HEX_PYRAMID.h
* This file defines the vertex data for a simple hexagonal pyramid,
* both in array and vector form.
* Vertices are replicated for each face, so that each face can have
* different normals and texture coordinates.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>
#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the hexagonal pyramid (6 triangles x 3 vertices)
const GLuint nHexPyramidVertices = 18, nHexPyramidIndices = 18;

// Radius and height used in shape construction
constexpr float HEX_RADIUS = 1.0f;
constexpr float HEX_HEIGHT = 1.0f;

// Forward direction of the hexagonal pyramid defined below
constexpr glm::vec3 HEX_PYRAMID_FORWARD{ 0.0f, -1.0f, 0.0f }; // facing down along the negative Y-axis

// Hexagonal base vertices (duplicated per face with apex)
const GLfloat hexPyramidPositionsArr[] = {
	// 6 triangle sides (apex and base vertices)
	 0.0f,					HEX_HEIGHT,		 0.0f,
	 HEX_RADIUS,			0.0f,			 0.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,

	  0.0f,					HEX_HEIGHT,		 0.0f,
	  HEX_RADIUS * 0.5f,	0.0f,			-HEX_RADIUS * 0.866f,
	 -HEX_RADIUS * 0.5f,	0.0f,			-HEX_RADIUS * 0.866f,

	  0.0f,					HEX_HEIGHT,		0.0f,
	 -HEX_RADIUS * 0.5f,	0.0f,			-HEX_RADIUS * 0.866f,
	 -HEX_RADIUS,			0.0f,			 0.0f,

	  0.0f,					HEX_HEIGHT,		0.0f,
	 -HEX_RADIUS,			0.0f,			0.0f,
	 -HEX_RADIUS * 0.5f,	0.0f,			HEX_RADIUS * 0.866f,

	  0.0f,					HEX_HEIGHT,		0.0f,
	 -HEX_RADIUS * 0.5f,	0.0f,			HEX_RADIUS * 0.866f,
	  HEX_RADIUS * 0.5f,	0.0f,			HEX_RADIUS * 0.866f,

	 0.0f,					HEX_HEIGHT,		0.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			HEX_RADIUS * 0.866f,
	 HEX_RADIUS,			0.0f,			0.0f
};

// Hexagonal pyramid normal data (flat normals for each triangle face)
const GLfloat hexPyramidNormalsArr[] = {
	 0.0f,		0.5f,	-0.866f,		 0.0f,		0.5f,	-0.866f,		 0.0f,		0.5f,	-0.866f,
	-0.866f,	0.5f,	-0.5f,			-0.866f,	0.5f,	-0.5f,			-0.866f,	0.5f,	-0.5f,
	-1.0f,		0.5f,    0.0f,			-1.0f,		0.5f,	0.0f,			-1.0f,		0.5f,   0.0f,
	-0.866f,	0.5f,    0.5f,			-0.866f,	0.5f,	0.5f,			-0.866f,	0.5f,   0.5f,
	 0.0f,		0.5f,    0.866f,		 0.0f,		0.5f,	0.866f,			 0.0f,		0.5f,   0.866f,
	 0.866f,	0.5f,	 0.5f,			 0.866f,	0.5f,	0.5f,			 0.866f,	0.5f,   0.5f
};

// Hexagonal pyramid texture coordinate data (simple triangle layout)
const GLfloat hexPyramidTexCoordsArr[] = {
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f
};

// Hexagonal pyramid index data (6 triangles x 3 vertices)
const GLuint hexPyramidIndicesArr[] = {
	0,	2,	1,
	3,	5,	4,
	6,	8,	7,
	9,	11,	10,
	12,	14,	13,
	15,	17,	16
};

// Interleaved hexagonal pyramid vertex data (position, normal, texture coordinates)
const GLfloat hexPyramidVerticesArr[] = {
	 0.0f,					HEX_HEIGHT,		 0.0f,
	 0.0f,					0.5f,			-0.866f,
	 0.5f,					1.0f,

	 HEX_RADIUS,			0.0f,			 0.0f,
	 0.0f,					0.5f,			-0.866f,
	 0.0f,					0.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,
	 0.0f,					0.5f,			-0.866f,
	 1.0f,					0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,
	-0.866f,				0.5f,			-0.5f,
	 0.5f,	1.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,
	-0.866f,				0.5f,			-0.5f,
	 0.0f,					0.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,
	-0.866f,				0.5f,			-0.5f,
	 1.0f,					0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,
	-1.0f,					0.5f,			 0.0f,
	 0.5f,					1.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,
	-1.0f,					0.5f,			 0.0f,
	 0.0f,					0.0f,
	-HEX_RADIUS,			0.0f,			 0.0f,
	-1.0f,					0.5f,			 0.0f,
	 1.0f,					0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,
	-0.866f,				0.5f,			 0.5f,
	 0.5f,					1.0f,
	-HEX_RADIUS,			0.0f,			 0.0f,
	-0.866f,				0.5f,			 0.5f,
	 0.0f,					0.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			 HEX_RADIUS * 0.866f,
	-0.866f,				0.5f,			 0.5f,
	 1.0f,					0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,
	 0.0f,					0.5f,			 0.866f,
	 0.5f,					1.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			 HEX_RADIUS * 0.866f,
	 0.0f,					0.5f,			 0.866f,
	 0.0f,					0.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			 HEX_RADIUS * 0.866f,
	 0.0f,					0.5f,			 0.866f,
	 1.0f,					0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,
	 0.866f,				0.5f,			 0.5f,
	 0.5f,					1.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			 HEX_RADIUS * 0.866f,
	 0.866f,				0.5f,			 0.5f,
	 0.0f,					0.0f,
	 HEX_RADIUS,			0.0f,			 0.0f,
	 0.866f,				0.5f,			 0.5f,
	 1.0f,					0.0f
};

// Hexagonal pyramid data in vector form
const std::vector<GLfloat> hexPyramidPositionsVec{
	std::begin(hexPyramidPositionsArr), std::end(hexPyramidPositionsArr)
};
const std::vector<GLfloat> hexPyramidNormalsVec{
	std::begin(hexPyramidNormalsArr), std::end(hexPyramidNormalsArr)
};
const std::vector<GLfloat> hexPyramidTexCoordsVec{
	std::begin(hexPyramidTexCoordsArr), std::end(hexPyramidTexCoordsArr)
};
const std::vector<GLuint> hexPyramidIndicesVec{
	std::begin(hexPyramidIndicesArr), std::end(hexPyramidIndicesArr)
};
const std::vector<GLfloat> hexPyramidVerticesVec{
	std::begin(hexPyramidVerticesArr), std::end(hexPyramidVerticesArr)
};
