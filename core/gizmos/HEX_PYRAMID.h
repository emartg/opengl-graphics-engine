/*
* HEX_PYRAMID.h
* This file defines the vertex data for a simple hexagonal pyramid, both in array and vector form.
* Vertices are replicated for each face, so that each face can have different normals and texture coordinates.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>
#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the hexagonal pyramid (6 triangles × 3 vertices)
const GLuint nHexPyramidVertices = 18, nHexPyramidIndices = 18;

// Radius and height used in shape construction
constexpr float HEX_RADIUS = 1.0f;
constexpr float HEX_HEIGHT = 1.0f;

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
	 0.0f,		0.5f,	-0.866f,		 0.0f,		0.5f,	-0.866f,			 0.0f,		0.5f,	-0.866f,
	-0.866f,	0.5f,	-0.5f,			-0.866f,	0.5f,	-0.5f,				-0.866f,	0.5f,	-0.5f,
	-1.0f,		0.5f,    0.0f,			-1.0f,		0.5f,	0.0f,				-1.0f,		0.5f,   0.0f,
	-0.866f,	0.5f,    0.5f,			-0.866f,	0.5f,	0.5f,				-0.866f,	0.5f,   0.5f,
	 0.0f,		0.5f,    0.866f,		 0.0f,		0.5f,	0.866f,				 0.0f,		0.5f,   0.866f,
	 0.866f,	0.5f,	 0.5f,			 0.866f,	0.5f,	0.5f,				 0.866f,	0.5f,   0.5f
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

// Hexagonal pyramid index data (6 triangles × 3 vertices)
const GLuint hexPyramidIndicesArr[] = {
	0,	1,	2,
	3,	4,	5,
	6,	7,	8,
	9,	10,	11,
	12,	13,	14,
	15,	16,	17
};

// Interleaved hexagonal pyramid vertex data (position, normal, texture coordinates)
const GLfloat hexPyramidVerticesArr[] = {
	// position															// normal							// tex coords
	0.0f,					HEX_HEIGHT,		 0.0f,						0.0f,		0.5f,	-0.866f,		0.5f,	1.0f,
	HEX_RADIUS,				0.0f,			 0.0f,						0.0f,		0.5f,	-0.866f,		0.0f,	0.0f,
	HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,		0.0f,		0.5f,	-0.866f,		1.0f,	0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,						-0.866f,	0.5f,	-0.5f,			0.5f,	1.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,		-0.866f,	0.5f,	-0.5f,			0.0f,	0.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,		-0.866f,	0.5f,	-0.5f,			1.0f,	0.0f,

	 0.0f,					HEX_HEIGHT,		 0.0f,						-1.0f,		0.5f,	0.0f,			0.5f,	1.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			-HEX_RADIUS * 0.866f,		-1.0f,		0.5f,	0.0f,			0.0f,	0.0f,
	-HEX_RADIUS,			0.0f,			 0.0f,						-1.0f,		0.5f,	0.0f, 			1.0f,	0.0f,

	 0.0f,					HEX_HEIGHT,		0.0f,						-0.866f,	0.5f,	0.5f,			0.5f,	1.0f,
	-HEX_RADIUS,			0.0f,			0.0f,						-0.866f,	0.5f,	0.5f,			0.0f,	0.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			HEX_RADIUS * 0.866f,		-0.866f,	0.5f,	0.5f,			1.0f,	0.0f,

	 0.0f,					HEX_HEIGHT,		0.0f,						0.0f,		0.5f,	0.866f,			0.5f,	1.0f,
	-HEX_RADIUS * 0.5f,		0.0f,			HEX_RADIUS * 0.866f,		0.0f,		0.5f,	0.866f,			0.0f,	0.0f,
	 HEX_RADIUS * 0.5f,		0.0f,			HEX_RADIUS * 0.866f,		0.0f,		0.5f,	0.866f,			1.0f,	0.0f,

	0.0f,					HEX_HEIGHT,		0.0f,						0.866f,		0.5f,	0.5f,			0.5f,	1.0f,
	HEX_RADIUS * 0.5f,		0.0f,			HEX_RADIUS * 0.866f,		0.866f,		0.5f,	0.5f,			0.0f,	0.0f,
	HEX_RADIUS,				0.0f,			0.0f,						0.866f,		0.5f,	0.5f,			1.0f,	0.0f
};

// Hexagonal pyramid data in vector form
const std::vector<GLfloat> hexPyramidPositionsVec{ std::begin(hexPyramidPositionsArr), std::end(hexPyramidPositionsArr) };
const std::vector<GLfloat> hexPyramidNormalsVec{ std::begin(hexPyramidNormalsArr), std::end(hexPyramidNormalsArr) };
const std::vector<GLfloat> hexPyramidTexCoordsVec{ std::begin(hexPyramidTexCoordsArr), std::end(hexPyramidTexCoordsArr) };
const std::vector<GLuint> hexPyramidIndicesVec{ std::begin(hexPyramidIndicesArr), std::end(hexPyramidIndicesArr) };
const std::vector<GLfloat> hexPyramidVerticesVec{ std::begin(hexPyramidVerticesArr), std::end(hexPyramidVerticesArr) };
