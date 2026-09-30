/*
 * SCREEN_QUAD.h
 * This file defines the vertex data for a simple screen quad,
 * which is used alongside FBOs for post-processing effects and rendering to textures.
 */

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Number of vertices and indices in the screen quad (2 triangles x 3 vertices)
const GLuint screen_quad_vertex_count = 6, screen_quad_index_count = 12;

// clang-format off

// Screen quad vertices
const GLfloat screen_quad_pos_array[] = {
	-1.0f,	-1.0f,	0.0f, // bottom-left
	 1.0f,	-1.0f,	0.0f, // bottom-right
	 1.0f,	 1.0f,	0.0f, // top-right
	-1.0f,	 1.0f,	0.0f  // top-left
};

// Screen quad texture coordinate data
const GLfloat screen_quad_tex_coords_array[] = {
	0.0f,	0.0f, // bottom-left
	1.0f,	0.0f, // bottom-right
	1.0f,	1.0f, // top-right
	0.0f,	1.0f  // top-left
};

// Screen quad index data (2 triangles x 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint screen_quad_indices_array[] = {
	0,  1,  2,  // bottom-left, bottom-right, top-right
	0,  2,  3   // bottom-left, top-right, top-left
};

// Interleaved vertex data for the screen quad
const GLfloat screen_quad_vertices_array[] = {
	// positions				// texture coords
	-1.0f,	-1.0f,	0.0f,		0.0f,	0.0f, // bottom-left
	 1.0f,	-1.0f,	0.0f,		1.0f,	0.0f, // bottom-right
	 1.0f,   1.0f,	0.0f,		1.0f,	1.0f, // top-right
	-1.0f,   1.0f,	0.0f,		0.0f,	1.0f  // top-left
};

// clang-format on

// Screen quad vertex data in vector form
const std::vector<GLfloat> screen_quad_positions_vector{ std::begin(screen_quad_pos_array), std::end(screen_quad_pos_array) };
const std::vector<GLfloat> screen_quad_tex_coords_vector{ std::begin(screen_quad_tex_coords_array),
														  std::end(screen_quad_tex_coords_array) };
const std::vector<GLuint>  screen_quad_indices_vector{ std::begin(screen_quad_indices_array), std::end(screen_quad_indices_array) };
const std::vector<GLfloat> screen_quad_vertices_vector{ std::begin(screen_quad_vertices_array), std::end(screen_quad_vertices_array) };
