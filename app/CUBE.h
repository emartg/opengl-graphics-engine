/*
* CUBE.h
* This file defines the vertex data for a simple cube, both in array and vector form.
* Vertices are replicated for each face, so that each face can have different normals and texture coordinates.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the cube
const GLuint nCubeVertices = 24, nCubeIndices = 36;

// Cube vertex position data
const GLfloat cubePositionsArr[] = {
	// front
	-1.0f,	-1.0f,	1.0f,
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
const GLfloat cubeNormalsArr[] = {
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
const GLfloat cubeTexCoordsArr[] = {
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
const GLuint cubeIndicesArr[] = {
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

// Interleaved cube vertex data (position, normal, texture coordinates)
const GLfloat cubeVerticesArr[] = {
	// position				 // normal				// texture coordinates
	// front
	-1.0f,	-1.0f,	1.0f,		0.0f,	0.0f,	1.0f,		0.0f,	0.0f,
	 1.0f,	-1.0f,  1.0f,		0.0f,	0.0f,	1.0f,		1.0f,	0.0f,
	 1.0f,   1.0f,  1.0f,		0.0f,	0.0f,	1.0f,		1.0f,	1.0f,
	-1.0f,   1.0f,  1.0f,		0.0f,	0.0f,	1.0f,		0.0f,	1.0f,
	// back
	-1.0f,	-1.0f, -1.0f,		0.0f,	0.0f,	-1.0f,		0.0f,	0.0f,
	 1.0f,	-1.0f, -1.0f,		0.0f,	0.0f,	-1.0f,		1.0f,	0.0f,
	 1.0f,   1.0f, -1.0f,		0.0f,	0.0f,	-1.0f,		1.0f,	1.0f,
	-1.0f,   1.0f, -1.0f,		0.0f,	0.0f,	-1.0f,		0.0f,	1.0f,
	// left
	-1.0f,	-1.0f, -1.0f,		-1.0f,	0.0f,	0.0f,		0.0f,	0.0f,
	-1.0f,	-1.0f,  1.0f,		-1.0f,	0.0f,	0.0f,		1.0f,	0.0f,
	-1.0f,   1.0f,  1.0f,		-1.0f,	0.0f,	0.0f,		1.0f,	1.0f,
	-1.0f,   1.0f, -1.0f,		-1.0f,	0.0f,	0.0f,		0.0f,	1.0f,
	// right
	1.0f,	-1.0f, -1.0f,		1.0f,	0.0f,	0.0f,		0.0f,	0.0f,
	1.0f,	-1.0f,  1.0f,		1.0f,	0.0f,	0.0f,		1.0f,	0.0f,
	1.0f,	 1.0f,  1.0f,		1.0f,	0.0f,	0.0f,		1.0f,	1.0f,
	1.0f,    1.0f, -1.0f,		1.0f,	0.0f,	0.0f,		0.0f,	1.0f,
	// top
	-1.0f,	1.0f, -1.0f,		0.0f,	1.0f,	0.0f,		0.0f,	0.0f,
	 1.0f,	1.0f, -1.0f,		0.0f,	1.0f,	0.0f,		1.0f,	0.0f,
	 1.0f,	1.0f,  1.0f,		0.0f,	1.0f,	0.0f,		1.0f,	1.0f,
	-1.0f,	1.0f,  1.0f,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f,
	// bottom
	-1.0f,	-1.0f, -1.0f,		0.0f,	-1.0f,	0.0f,		0.0f,	0.0f,
	 1.0f,	-1.0f, -1.0f,		0.0f,	-1.0f,	0.0f,		1.0f,	0.0f,
	 1.0f,	-1.0f,  1.0f,		0.0f,	-1.0f,	0.0f,		1.0f,	1.0f,
	-1.0f,	-1.0f,  1.0f,		0.0f,	-1.0f,	0.0f,		0.0f,	1.0f
};

// Cube data in vector form
const std::vector<GLfloat> cubePositionsVec{ std::begin(cubePositionsArr), std::end(cubePositionsArr) };
const std::vector<GLfloat> cubeNormalsVec{ std::begin(cubeNormalsArr), std::end(cubeNormalsArr) };
const std::vector<GLfloat> cubeTexCoordsVec{ std::begin(cubeTexCoordsArr), std::end(cubeTexCoordsArr) };
const std::vector<GLuint> cubeIndicesVec{ std::begin(cubeIndicesArr), std::end(cubeIndicesArr) };
const std::vector<GLfloat> cubeVerticesVec{ std::begin(cubeVerticesArr), std::end(cubeVerticesArr) };
