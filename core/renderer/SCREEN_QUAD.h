/*
* SCREEN_QUAD.h
* This file defines the vertex data for a simple screen quad,
* which is used alongside FBOs for post-processing effects and rendering to textures.
*/

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the screen quad (2 triangles x 3 vertices)
const GLuint nScreenQuadVertices = 6, nScreenQuadIndices = 12;

// Screen quad vertices
const GLfloat screenQuadPositionsArr[] = {
	-1.0f,	-1.0f,	0.0f, // bottom-left
	 1.0f,	-1.0f,	0.0f, // bottom-right
	 1.0f,	 1.0f,	0.0f, // top-right
	-1.0f,	 1.0f,	0.0f  // top-left
};

// Screen quad texture coordinate data
const GLfloat screenQuadTexCoordsArr[] = {
	0.0f,	0.0f, // bottom-left
	1.0f,	0.0f, // bottom-right
	1.0f,	1.0f, // top-right
	0.0f,	1.0f  // top-left
};

// Screen quad index data (2 triangles x 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint screenQuadIndicesArr[] = {
	0,  1,  2,  // bottom-left, bottom-right, top-right
	0,  2,  3   // bottom-left, top-right, top-left
};

// Interleaved vertex data for the screen quad
const GLfloat screenQuadVerticesArr[] = {
	// positions				// texture coords
	-1.0f,	-1.0f,	0.0f,		0.0f,	0.0f, // bottom-left
	 1.0f,	-1.0f,	0.0f,		1.0f,	0.0f, // bottom-right
	 1.0f,   1.0f,	0.0f,		1.0f,	1.0f, // top-right
	-1.0f,   1.0f,	0.0f,		0.0f,	1.0f  // top-left
};

// Screen quad vertex data in vector form
const std::vector<GLfloat> screenQuadPositionsVec{
	std::begin(screenQuadPositionsArr), std::end(screenQuadPositionsArr)
};
const std::vector<GLfloat> screenQuadTexCoordsVec{
	std::begin(screenQuadTexCoordsArr), std::end(screenQuadTexCoordsArr)
};
const std::vector<GLuint> screenQuadIndicesVec{
	std::begin(screenQuadIndicesArr), std::end(screenQuadIndicesArr)
};
const std::vector<GLfloat> screenQuadVerticesVec{
	std::begin(screenQuadVerticesArr), std::end(screenQuadVerticesArr)
};