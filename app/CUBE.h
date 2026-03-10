/*
* CUBE.h
* This file defines the vertex data for a simple cube,
* both in array and vector form.
* Vertices are replicated for each face, so that each face can have
* different normals and texture coordinates.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape_Model constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the cube
const GLuint cube_vertex_count = 24, cube_index_count = 36;

// Cube vertex position data
const GLfloat cube_positions_array[] = {
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
const GLfloat cube_normals_array[] = {
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
const GLfloat cube_tex_coords_array[] = {
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
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint cube_indices_array[] = {
	// front (face toward +Z)
	0, 1, 2,  // bottom-left, bottom-right, top-right (CCW)
	0, 2, 3,  // bottom-left, top-right, top-left (CCW)

	// back (face toward -Z)
	4, 6, 5,  // bottom-left, top-right, bottom-right (CCW when viewed from -Z)
	4, 7, 6,  // bottom-left, top-left, top-right (CCW)

	// left (face toward -X)
	8, 9, 10, // bottom-left, bottom-right, top-right (CCW when viewed from -X)
	8, 10, 11,// bottom-left, top-right, top-left (CCW)

	// right (face toward +X)
	12, 14, 13, // bottom-left, top-right, bottom-right (CCW when viewed from +X)
	12, 15, 14, // bottom-left, top-left, top-right (CCW)

	// top (face toward +Y)
	16, 19, 18,  // front-left, back-left, back-right (CCW when viewed from +Y)
	16, 18, 17,  // front-left, back-right, front-right (CCW)

	// bottom (face toward -Y)
	20, 21, 22,  // front-left, front-right, back-right (CCW when viewed from -Y)
	20, 22, 23,  // front-left, back-right, back-left (CCW)
};

// Interleaved cube vertex data (position, normal, texture coordinates)
const GLfloat cube_vertices_array[] = {
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
const std::vector<GLfloat> cube_positions_vector{
	std::begin(cube_positions_array), std::end(cube_positions_array)
};
const std::vector<GLfloat> cube_normals_vector{
	std::begin(cube_normals_array), std::end(cube_normals_array)
};
const std::vector<GLfloat> cube_tex_coords_vector{
	std::begin(cube_tex_coords_array), std::end(cube_tex_coords_array)
};
const std::vector<GLuint> cube_indices_vector{
	std::begin(cube_indices_array), std::end(cube_indices_array)
};
const std::vector<GLfloat> cube_vertices_vector{
	std::begin(cube_vertices_array), std::end(cube_vertices_array)
};
