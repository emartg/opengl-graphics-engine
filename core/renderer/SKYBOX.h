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
	-1.0f, -1.0f, -1.0f,	 1.0f, -1.0f, -1.0f,	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,	-1.0f, -1.0f,  1.0f,	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,	-1.0f,  1.0f,  1.0f
};

// Skybox cube index data (6 faces x 2 triangles x 3 vertices)
// Winding order is clockwise (CW) when viewed from inside the cube
const GLuint skyboxIndicesArr[] = {
	0, 1, 2,	2, 3, 0,	// front face	(facing -Z)
	4, 7, 6,	6, 5, 4,	// back face	(facing +Z)
	4, 0, 3,	3, 7, 4,	// left face	(facing -X)
	1, 5, 6,	6, 2, 1,	// right face	(facing +X)
	3, 2, 6,	6, 7, 3,	// top face		(facing +Y)
	4, 5, 1,	1, 0, 4		// bottom face	(facing -Y)
};

// Skybox cube data in vector form
const std::vector<GLfloat> skyboxPositionsVec{
	std::begin(skyboxPositionsArr), std::end(skyboxPositionsArr)
};
const std::vector<GLuint> skyboxIndicesVec{
	std::begin(skyboxIndicesArr), std::end(skyboxIndicesArr)
};