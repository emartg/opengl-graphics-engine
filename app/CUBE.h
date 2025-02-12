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
const GLuint nVertices = 24, nIndices = 36;

// Cube vertex position data
const GLfloat positionsArr[] = {
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
const GLfloat normalsArr[] = {
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
const GLfloat texCoordsArr[] = {
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
const GLuint indicesArr[] = {
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
const GLfloat verticesArr[] = {
	// position				 // normal				// texture coordinates
	// front
	-1.0f, -1.0f,  1.0f,	 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,	 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f,	 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
	-1.0f,  1.0f,  1.0f,	 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
	// back
	-1.0f, -1.0f, -1.0f,	 0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,	 0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
	 1.0f,  1.0f, -1.0f,	 0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,	 0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
	// left
	-1.0f, -1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
	-1.0f, -1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	-1.0f,  1.0f,  1.0f,	-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	-1.0f,  1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
	// right
	1.0f, -1.0f, -1.0f,		 1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
	1.0f, -1.0f,  1.0f,		 1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
	1.0f,  1.0f,  1.0f,		 1.0f, 0.0f, 0.0f,		1.0f, 1.0f,
	1.0f,  1.0f, -1.0f,		 1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
	// top
	-1.0f, 1.0f, -1.0f,		 0.0f, 1.0f, 0.0f,		0.0f, 0.0f,
	1.0f,  1.0f, -1.0f,		 0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
	1.0f,  1.0f,  1.0f,		 0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f, 1.0f,  1.0f,		 0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
	// bottom
	-1.0f, -1.0f, -1.0f,	 0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
	 1.0f, -1.0f, -1.0f,	 0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
	 1.0f, -1.0f,  1.0f,	 0.0f, -1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f, -1.0f,  1.0f,	 0.0f, -1.0f, 0.0f,		0.0f, 1.0f
};

// Cube vertex position data in vector form
const std::vector<GLfloat> positionsVec{ std::begin(positionsArr), std::end(positionsArr) };

// Cube normal data in vector form
const std::vector<GLfloat> normalsVec{ std::begin(normalsArr), std::end(normalsArr) };

// Cube texture coordinate data in vector form
const std::vector<GLfloat> texCoordsVec{ std::begin(texCoordsArr), std::end(texCoordsArr) };

// Cube index data in vector form
const std::vector<GLuint> indicesVec{ std::begin(indicesArr), std::end(indicesArr) };

// Interleaved cube vertex data in vector form
const std::vector<GLfloat> verticesVec{ std::begin(verticesArr), std::end(verticesArr) };
