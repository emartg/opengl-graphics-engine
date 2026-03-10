/*
* DECAHEDRON.h
* This file defines the vertex data for a simple decahedron (pentagonal bipyramid),
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

// Number of vertices and indices in the decahedron (10 triangles x 3 vertices)
const GLuint decahedron_vertex_count = 30, decahedron_index_count = 30;

// Decahedron vertex position data (pentagonal bipyramid: top, bottom, 5 base vertices)
const GLfloat decahedron_positions_array[] = {
	// Top 5 faces
	0.0f,	1.0f,	0.0f,		 0.0f,		0.0f,	 1.0f,		 0.951f,	0.0f,	 0.309f,
	0.0f,	1.0f,	0.0f,		 0.951f,	0.0f,	 0.309f,	 0.588f,	0.0f,	-0.809f,
	0.0f,	1.0f,	0.0f,		 0.588f,	0.0f,	-0.809f,	-0.588f,	0.0f,	-0.809f,
	0.0f,	1.0f,	0.0f,		-0.588f,	0.0f,	-0.809f,	-0.951f,	0.0f,	 0.309f,
	0.0f,	1.0f,	0.0f,		-0.951f,	0.0f,	 0.309f,	 0.0f,		0.0f,	 1.0f,

	// Bottom 5 faces
	0.0f,	-1.0f,	0.0f,		 0.0f,		0.0f,	 1.0f,		-0.951f,	0.0f,	 0.309f,
	0.0f,	-1.0f,  0.0f,		-0.951f,	0.0f,	 0.309f,	-0.588f,	0.0f,	-0.809f,
	0.0f,	-1.0f,  0.0f,		-0.588f,	0.0f,	-0.809f,	 0.588f,	0.0f,	-0.809f,
	0.0f,	-1.0f,  0.0f,		 0.588f,	0.0f,	-0.809f,	 0.951f,	0.0f,	 0.309f,
	0.0f,	-1.0f,  0.0f,		 0.951f,	0.0f,	 0.309f,	 0.0f,		0.0f,	 1.0f
};

// Decahedron normal data (flat normals for each triangle face)
const GLfloat decahedron_normals_array[] = {
	// simplified normals, actual per-face normals would be calculated with cross products
	 0.0f,		0.707f,		 0.707f,
	 0.0f,		0.707f,		 0.707f,
	 0.0f,		0.707f,		 0.707f,

	 0.707f,	0.707f,		 0.0f,
	 0.707f,	0.707f,		 0.0f,
	 0.707f,	0.707f,		 0.0f,

	 0.0f,		0.707f,		-0.707f,
	 0.0f,		0.707f,		-0.707f,
	 0.0f,		0.707f,		-0.707f,

	-0.707f,	0.707f,		 0.0f,
	-0.707f,	0.707f,		 0.0f,
	-0.707f,	0.707f,		 0.0f,

	 0.0f,		0.707f,		 0.707f,
	 0.0f,		0.707f,		 0.707f,
	 0.0f,		0.707f,		 0.707f,

	 0.0f,		-0.707f,	 0.707f,
	 0.0f,		-0.707f,	 0.707f,
	 0.0f,		-0.707f,	 0.707f,

	-0.707f,	-0.707f,	 0.0f,
	-0.707f,	-0.707f,	 0.0f,
	-0.707f,	-0.707f,	 0.0f,

	 0.0f,		-0.707f,	-0.707f,
	 0.0f,		-0.707f,	-0.707f,
	 0.0f,		-0.707f,	-0.707f,

	 0.707f,	-0.707f,	 0.0f,
	 0.707f,	-0.707f,	 0.0f,
	 0.707f,	-0.707f,	 0.0f,

	 0.0f,		-0.707f,	 0.707f,
	 0.0f,		-0.707f,	 0.707f,
	 0.0f,		-0.707f,	 0.707f
};

// Decahedron texture coordinate data (simple triangle layout)
const GLfloat decahedron_tex_coords_array[] = {
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,

	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f,
	0.5f,	1.0f,		0.0f,	0.0f,		1.0f, 0.0f
};

// Decahedron index data (10 triangles x 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint decahedron_index_array[] = {
	0,  1,  2,
	3,  4,  5,
	6,  7,  8,
	9,  10, 11,
	12, 13, 14,
	15, 16, 17,
	18, 19, 20,
	21, 22, 23,
	24, 25, 26,
	27, 28, 29
};

// Interleaved decahedron vertex data (position, normal, texture coordinates)
const GLfloat decahedron_vertices_array[] = {
	// position						// normal								// tex coords
	0.0f,    1.0f,   0.0f,			0.0f,		0.707f,		0.707f,			0.5f,	1.0f,
	0.0f,    0.0f,   1.0f,			0.0f,		0.707f,		0.707f,			0.0f,	0.0f,
	0.951f,  0.0f,   0.309f,		0.0f,		0.707f,		0.707f,			1.0f,	0.0f,

	0.0f,    1.0f,   0.0f,			0.707f,		0.707f,		0.0f,			0.5f,	1.0f,
	0.951f,  0.0f,   0.309f,		0.707f,		0.707f,		0.0f,			0.0f,	0.0f,
	0.588f,  0.0f,  -0.809f,		0.707f,		0.707f,		0.0f,			1.0f,	0.0f,

	 0.0f,    1.0f,   0.0f,			0.0f,		0.707f,		-0.707f,		0.5f,	1.0f,
	 0.588f,  0.0f,  -0.809f,		0.0f,		0.707f,		-0.707f,		0.0f,	0.0f,
	-0.588f,  0.0f,  -0.809f,		0.0f,		0.707f,		-0.707f,		1.0f,	0.0f,

	 0.0f,    1.0f,   0.0f,			-0.707f,	0.707f,		0.0f,			0.5f,	1.0f,
	-0.588f,  0.0f,  -0.809f,		-0.707f,	0.707f,		0.0f,			0.0f,	0.0f,
	-0.951f,  0.0f,   0.309f,		-0.707f,	0.707f,		0.0f,			1.0f,	0.0f,

	 0.0f,    1.0f,   0.0f,			0.0f,		0.707f,		0.707f,			0.5f,	1.0f,
	-0.951f,  0.0f,   0.309f,		0.0f,		0.707f,		0.707f,			0.0f,	0.0f,
	 0.0f,    0.0f,   1.0f,			0.0f,		0.707f,		0.707f,			1.0f,	0.0f,

	 0.0f,	 -1.0f,   0.0f,			0.0f,		-0.707f,	0.707f,			0.5f,	1.0f,
	 0.0f,    0.0f,   1.0f,			0.0f,		-0.707f,	0.707f,			0.0f,	0.0f,
	-0.951f,  0.0f,   0.309f,		0.0f,		-0.707f,	0.707f,			1.0f,	0.0f,

	 0.0f,	 -1.0f,   0.0f,			-0.707f,	-0.707f,	0.0f,			0.5f,	1.0f,
	-0.951f,  0.0f,   0.309f,		-0.707f,	-0.707f,	0.0f,			0.0f,	0.0f,
	-0.588f,  0.0f,  -0.809f,		-0.707f,	-0.707f,	0.0f,			1.0f,	0.0f,

	 0.0f,   -1.0f,   0.0f,			0.0f,		-0.707f,	-0.707f,		0.5f,	1.0f,
	-0.588f,  0.0f,  -0.809f,		0.0f,		-0.707f,	-0.707f,		0.0f,	0.0f,
	 0.588f,  0.0f,  -0.809f,		0.0f,		-0.707f,	-0.707f,		1.0f,	0.0f,

	0.0f,   -1.0f,   0.0f,			0.707f,		-0.707f,	0.0f,			0.5f,	1.0f,
	0.588f,  0.0f,  -0.809f,		0.707f,		-0.707f,	0.0f,			0.0f,	0.0f,
	0.951f,  0.0f,   0.309f,		0.707f,		-0.707f,	0.0f,			1.0f,	0.0f,

	0.0f,   -1.0f,   0.0f,			0.0f,		-0.707f,	0.707f,			0.5f,	1.0f,
	0.951f,  0.0f,   0.309f,		0.0f,		-0.707f,	0.707f,			0.0f,	0.0f,
	0.0f,    0.0f,   1.0f,			0.0f,		-0.707f,	0.707f,			1.0f,	0.0f
};

// Decahedron data in vector form
const std::vector<GLfloat> decahedron_positions_vector{
	std::begin(decahedron_positions_array), std::end(decahedron_positions_array)
};
const std::vector<GLfloat> decahedron_normals_vector{
	std::begin(decahedron_normals_array), std::end(decahedron_normals_array)
};
const std::vector<GLfloat> decahedron_tex_coords_vector{
	std::begin(decahedron_tex_coords_array), std::end(decahedron_tex_coords_array)
};
const std::vector<GLuint> decahedron_indices_vector{
	std::begin(decahedron_index_array), std::end(decahedron_index_array)
};
const std::vector<GLfloat> decahedron_vertices_vector{
	std::begin(decahedron_vertices_array), std::end(decahedron_vertices_array)
};
