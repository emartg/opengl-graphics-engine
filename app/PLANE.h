/*
 * PLANE.h
 * This file defines the vertex data for a rectangular plane (a quad made of 2 triangles).
 * It uses an index buffer to share vertices, and its defined both in array and vector form.
 * The vertex data is separated into position, normal, and texture coordinate data, but there is also
 * an array and a vector with the interleaved vertex data to test the Shape_Model constructors that take
 * interleaved vertex data directly.
 */

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Number of vertices and indices in the plane (4 vertices, 2 triangles * 3 indices = 6 indices)
const GLuint plane_vertex_count = 4, plane_index_count = 6;

// Plane dimensions used in shape construction
constexpr float PLANE_WIDTH  = 2.0f;
constexpr float PLANE_HEIGHT = 2.0f;

// Forward direction of the plane defined below
constexpr glm::vec3 PLANE_FORWARD{ 0.0f, 1.0f, 0.0f }; // facing up along the Y-axis

// clang-format off

// Plane vertices
const GLfloat plane_positions_array[] = {
	-PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,	// 0: bottom-left
	 PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,	// 1: bottom-right
	 PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,	// 2: top-right
	-PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,	// 3: top-left
};

// Plane normal data (all facing the same direction)
const GLfloat plane_normals_array[] = {
	0.0f,	1.0f,	0.0f, // 0
	0.0f,	1.0f,	0.0f, // 1
	0.0f,	1.0f,	0.0f, // 2
	0.0f,	1.0f,	0.0f  // 3
};

// Plane texture coordinate data
const GLfloat plane_tex_coords_array[] = {
	0.0f,	0.0f, // 0
	1.0f,	0.0f, // 1
	1.0f,	1.0f, // 2
	0.0f,	1.0f  // 3
};

// Plane index data (2 triangles * 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint plane_indices_array[] = {
	0,  1,  2,  // first triangle (bottom-left, bottom-right, top-right)
	0,  2,  3   // second triangle (bottom-left, top-right, top-left)
};

// Interleaved plane vertex data
const GLfloat plane_vertices_array[] = {
	// position											// normal					// tex coords
	-PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	0.0f, // 0
	 PLANE_WIDTH / 2,	0.0f,	-PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	0.0f, // 1
	 PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	1.0f, // 2
	-PLANE_WIDTH / 2,	0.0f,	 PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f  // 3
};

// clang-format on

// Plane data in vector form
const std::vector<GLfloat> plane_positions_vector{ std::begin(plane_positions_array), std::end(plane_positions_array) };
const std::vector<GLfloat> plane_normals_vector{ std::begin(plane_normals_array), std::end(plane_normals_array) };
const std::vector<GLfloat> plane_tex_coords_vector{ std::begin(plane_tex_coords_array), std::end(plane_tex_coords_array) };
const std::vector<GLuint>  plane_indices_vector{ std::begin(plane_indices_array), std::end(plane_indices_array) };
const std::vector<GLfloat> plane_vertices_vector{ std::begin(plane_vertices_array), std::end(plane_vertices_array) };