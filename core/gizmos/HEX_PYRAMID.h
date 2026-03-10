/*
* HEX_PYRAMID.h
* This file defines the vertex data for a simple hexagonal pyramid,
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
#include <glm/glm.hpp>

// Number of vertices and indices in the hexagonal pyramid (6 triangles x 3 vertices)
const GLuint hex_pyramid_vertex_count = 18, hex_pyramid_index_count = 18;

// Radius and height used in shape construction
constexpr float HEX_RADIUS = 1.0f;
constexpr float HEX_HEIGHT = 1.0f;

// Forward direction of the hexagonal pyramid defined below
constexpr glm::vec3 HEX_PYRAMID_FORWARD{ 0.0f, -1.0f, 0.0f }; // facing down along the negative Y-axis

// Hexagonal base vertices (duplicated per face with apex)
const GLfloat hex_pyramid_positions_array[] = {
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
const GLfloat hex_pyramid_normals_array[] = {
	 0.0f,		0.5f,	-0.866f,		 0.0f,		0.5f,	-0.866f,		 0.0f,		0.5f,	-0.866f,
	-0.866f,	0.5f,	-0.5f,			-0.866f,	0.5f,	-0.5f,			-0.866f,	0.5f,	-0.5f,
	-1.0f,		0.5f,    0.0f,			-1.0f,		0.5f,	0.0f,			-1.0f,		0.5f,   0.0f,
	-0.866f,	0.5f,    0.5f,			-0.866f,	0.5f,	0.5f,			-0.866f,	0.5f,   0.5f,
	 0.0f,		0.5f,    0.866f,		 0.0f,		0.5f,	0.866f,			 0.0f,		0.5f,   0.866f,
	 0.866f,	0.5f,	 0.5f,			 0.866f,	0.5f,	0.5f,			 0.866f,	0.5f,   0.5f
};

// Hexagonal pyramid texture coordinate data (simple triangle layout)
const GLfloat hex_pyramid_tex_coords_array[] = {
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
	0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f
};

// Hexagonal pyramid index data (6 triangles x 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint hex_pyramid_indices_array[] = {
	0,	1,	2,
	3,	4,	5,
	6,	7,	8,
	9,	10,	11,
	12,	13,	14,
	15,	16,	17
};

// Interleaved hexagonal pyramid vertex data (position, normal, texture coordinates)
const GLfloat hex_pyramid_vertices_array[] = {
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
const std::vector<GLfloat> hex_pyramid_positions_vector{
	std::begin(hex_pyramid_positions_array), std::end(hex_pyramid_positions_array)
};
const std::vector<GLfloat> hex_pyramid_normals_vector{
	std::begin(hex_pyramid_normals_array), std::end(hex_pyramid_normals_array)
};
const std::vector<GLfloat> hex_pyramid_tex_coords_vector{
	std::begin(hex_pyramid_tex_coords_array), std::end(hex_pyramid_tex_coords_array)
};
const std::vector<GLuint> hex_pyramid_indices_vector{
	std::begin(hex_pyramid_indices_array), std::end(hex_pyramid_indices_array)
};
const std::vector<GLfloat> hex_pyramid_vertices_vector{
	std::begin(hex_pyramid_vertices_array), std::end(hex_pyramid_vertices_array)
};
