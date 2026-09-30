/*
 * TF_PLANE.h
 * This file defines the vertex data for a rectangular plane made of 4 triangles
 * fanning out from a central vertex.
 * It uses an index buffer to share vertices, and its defined both in array and vector form.
 * The vertex data is separated into position, normal, and texture coordinate data, but there is also
 * an array and a vector with the interleaved vertex data to test the Shape_Model constructors that take
 * interleaved vertex data directly.
 */

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Number of vertices and indices in the triangle fan plane
// (5 vertices, 4 triangles * 3 indices = 12 indices)
const GLuint triangle_fan_plane_vertex_count = 5, triangle_fan_plane_index_count = 12;

// Plane dimensions used in shape construction
constexpr float TF_PLANE_WIDTH  = 2.0f;
constexpr float TF_PLANE_HEIGHT = 2.0f;

// Forward direction of the plane defined below
constexpr glm::vec3 TF_PLANE_FORWARD{ 0.0f, 1.0f, 0.0f }; // facing up along the Y-axis

// clang-format off

// Triangle fan plane vertices
const GLfloat triangle_fan_plane_positions_array[] = {
	-TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,	// 0: bottom-left
	 TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,	// 1: bottom-right
	 TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,	// 2: top-right
	-TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,	// 3: top-left
	 0.0f,					0.0f,	 0.0f					// 4: center
};

// Triangle fan plane normal data (all facing the same direction)
const GLfloat triangle_fan_plane_normals_array[] = {
	0.0f,	1.0f,	0.0f,	// 0
	0.0f,	1.0f,	0.0f,	// 1
	0.0f,	1.0f,	0.0f,	// 2
	0.0f,	1.0f,	0.0f,	// 3
	0.0f,	1.0f,	0.0f	// 4: center
};

// Triangle fan plane texture coordinate data
const GLfloat triangle_fan_plane_tex_coords_array[] = {
	0.0f,	0.0f,	// 0
	1.0f,	0.0f,	// 1
	1.0f,	1.0f,	// 2
	0.0f,	1.0f,	// 3
	0.5f,	0.5f	// 4: center
};

// Triangle fan plane index data (4 triangles * 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint triangle_fan_plane_indices_array[] = {
	0,  1,  4,  // bottom triangle
	1,  2,  4,  // right triangle
	2,  3,  4,  // top triangle
	3,  0,  4   // left triangle
};

// Interleaved triangle fan plane vertex data
const GLfloat triangle_fan_plane_vertices_array[] = {
	// position													// normal					// tex coords
	-TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	0.0f, // 0
	 TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	0.0f, // 1
	 TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	1.0f, // 2
	-TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f, // 3
	 0.0f,					0.0f,	 0.0f,						0.0f,	1.0f,	0.0f,		0.5f,	0.5f  // 4
};

// clang-format on

// Triangle fan plane data in vector form
const std::vector<GLfloat> triangle_fan_plane_positions_vector{ std::begin(triangle_fan_plane_positions_array),
                                                                std::end(triangle_fan_plane_positions_array) };
const std::vector<GLfloat> triangle_fan_plane_normals_vector{ std::begin(triangle_fan_plane_normals_array),
                                                              std::end(triangle_fan_plane_normals_array) };
const std::vector<GLfloat> triangle_fan_plane_tex_coords_vector{ std::begin(triangle_fan_plane_tex_coords_array),
                                                                 std::end(triangle_fan_plane_tex_coords_array) };
const std::vector<GLuint>  triangle_fan_plane_indices_vector{ std::begin(triangle_fan_plane_indices_array),
                                                              std::end(triangle_fan_plane_indices_array) };
const std::vector<GLfloat> triangle_fan_plane_vertices_vector{ std::begin(triangle_fan_plane_vertices_array),
                                                               std::end(triangle_fan_plane_vertices_array) };

// clang-format off

// Triangle fan plane direction line data (the normal direction from the center)
const GLfloat triangle_fan_plane_direction_line_array[] = {
	// start: center (vertex 4, offset 4*8 = 32)
	triangle_fan_plane_vertices_array[32],
	triangle_fan_plane_vertices_array[33],
	triangle_fan_plane_vertices_array[34],

	// end: center + normal direction
	triangle_fan_plane_vertices_array[32] + triangle_fan_plane_vertices_array[35],
	triangle_fan_plane_vertices_array[33] + triangle_fan_plane_vertices_array[36],
	triangle_fan_plane_vertices_array[34] + triangle_fan_plane_vertices_array[37]
};

// clang-format on

// Triangle fan plane direction line data in vector form
const std::vector<GLfloat> triangle_fan_plane_direction_line_vector{ std::begin(triangle_fan_plane_direction_line_array),
                                                                     std::end(triangle_fan_plane_direction_line_array) };
