/*
* SKYBOX.h
* This file defines the vertex data for a simple skybox cube,
* which is used to create a skybox in the scene.
*/

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Number of unique vertices and indices in the skybox cube (6 faces x 2 triangles x 3 vertices)
const GLuint nSkyboxVertices = 8, nSkyboxIndices = 36;

// Skybox cube vertices (unique corners)
const GLfloat skyboxPositionsArr[] = {
	// positions
	-1.0f, -1.0f, -1.0f,	 1.0f, -1.0f, -1.0f,	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,	-1.0f, -1.0f,  1.0f,	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,	-1.0f,  1.0f,  1.0f
};

// Skybox cube inde	x data (6 faces x 2 triangles x 3 vertices)
const GLuint skyboxIndicesArr[] = {
	0,  1,  2,		2,  3,  0,		// front face  (z = -1)
	4,  5,  6,		6,  7,  4,		// back face   (z = +1)
	0,  4,  7,		7,  3,  0,		// left face   (x = -1)
	1,  5,  6,		6,  2,  1,		// right face  (x = +1)
	3,  2,  6,		6,  7,  3,		// top face    (y = +1)
	0,  1,  5,		5,  4,  0		// bottom face (y = -1)
};

// Skybox cube data in vector form
const std::vector<GLfloat> skyboxPositionsVec{
	std::begin(skyboxPositionsArr), std::end(skyboxPositionsArr)
};
const std::vector<GLuint> skyboxIndicesVec{
	std::begin(skyboxIndicesArr), std::end(skyboxIndicesArr)
};