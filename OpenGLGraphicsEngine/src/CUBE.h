#pragma once

/* CUBE.h
This file defines the vertex data for a simple cube.
Vertices are replicated for each face, so that each face can have different normals and texture coordinates */

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Cube vertex position data
const std::vector<GLfloat> cubeVertices = {
	// front
	-1.0f,	-1.0f,  1.0f,
	 1.0f,	-1.0f,  1.0f,
	 1.0f,	 1.0f,	1.0f,
	-1.0f,   1.0f,	1.0f,
	// back
	-1.0f,	-1.0f,	-1.0f,
	 1.0f,	-1.0f,	-1.0f,
	 1.0f,	 1.0f,	-1.0f,
	-1.0f,   1.0f,	-1.0f,
	// left
	-1.0f,	-1.0f,	-1.0f,
	-1.0f,	-1.0f,	 1.0f,
	-1.0f,	 1.0f,	 1.0f,
	-1.0f,	 1.0f,	-1.0f,
	// right
	1.0f,	-1.0f,	-1.0f,
	1.0f,	-1.0f,	 1.0f,
	1.0f,	 1.0f,	 1.0f,
	1.0f,	 1.0f,	-1.0f,
	// top
	-1.0f,   1.0f,	-1.0f,
	 1.0f,   1.0f,	-1.0f,
	 1.0f,   1.0f,	 1.0f,
	-1.0f,   1.0f,	 1.0f,
	// bottom
	-1.0f,	-1.0f,	-1.0f,
	 1.0f,	-1.0f,	-1.0f,
	 1.0f,	-1.0f,	 1.0f,
	-1.0f,	-1.0f,	 1.0f
};

// Cube normal data
const std::vector<GLfloat> cubeNormals = {
	// front
	0.0f,	0.0f,	1.0f,
	0.0f,	0.0f,	1.0f,
	0.0f,	0.0f,	1.0f,
	0.0f,	0.0f,	1.0f,
	// back
	0.0f,	0.0f,	-1.0f,
	0.0f,	0.0f,	-1.0f,
	0.0f,	0.0f,	-1.0f,
	0.0f,	0.0f,	-1.0f,
	// left
	-1.0f,  0.0f,	0.0f,
	-1.0f,  0.0f,	0.0f,
	-1.0f,  0.0f,	0.0f,
	-1.0f,  0.0f,	0.0f,
	// right
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	// top
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	// bottom
	0.0f,	-1.0f,  0.0f,
	0.0f,	-1.0f,  0.0f,
	0.0f,	-1.0f,  0.0f,
	0.0f,	-1.0f,  0.0f
};

// Cube texture coordinate data
const std::vector<GLfloat> cubeTexCoords = {
	// front
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	// back
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	// left
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	// right
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	// top
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f,
	// bottom
	0.0f,	0.0f,
	1.0f,	0.0f,
	1.0f,	1.0f,
	0.0f,	1.0f
};

// Cube index data
const std::vector<GLuint> cubeIndices = {
	// front
	0,	1,	2,
	2,	3,	0,
	// back
	4,	5,	6,
	6,	7,	4,
	// left
	8,	9,	10,
	10, 11, 8,
	// right
	12, 13, 14,
	14, 15, 12,
	// top
	16, 17, 18,
	18, 19, 16,
	// bottom
	20, 21, 22,
	22, 23, 20
};