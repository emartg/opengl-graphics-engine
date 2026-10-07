/*
 * Mesh_Geometry.h
 * This file defines the Mesh_Geometry class, which stores the vertices and indices of a mesh on the GPU
 * (vertex array, vertex buffer, and element buffer objects), and the Vertex struct.
 * Geometries are shared: meshes with identical vertex and index data use the same geometry,
 * so that it is only uploaded once (e.g., many cubes), and they can be drawn with instancing.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "../../utils/geometry/Bounding_Box.h"

// Struct that defines a single vertex of the mesh
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

class Mesh_Geometry
{
public:
	// Constructors
	// ------------
	Mesh_Geometry(const Mesh_Geometry&) = delete; // copy constructor (a geometry owns its buffer objects)

	// Operator overloads
	// ------------------
	Mesh_Geometry& operator=(const Mesh_Geometry&) = delete; // copy assignment operator

	// Destructor
	// ----------
	// Deletes the buffer objects/arrays of the geometry (requires the OpenGL context to be current)
	~Mesh_Geometry();

	// Public Static Methods
	// ---------------------
	// Returns the geometry with the given vertices and indices: an existing one if another mesh
	// already uses identical data, or a new one uploaded to the GPU otherwise
	static std::shared_ptr<Mesh_Geometry> get_or_create(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

	// Returns the number of geometries currently alive (i.e., stored on the GPU)
	static std::size_t get_live_count() { return live_count; }

	// Public Methods
	// --------------
	// Draws the geometry (with the currently bound shader program)
	void draw() const;

	// Getters
	GLuint                     get_vao() const { return vao; }
	GLsizei                    get_index_count() const { return static_cast<GLsizei>(indices.size()); }
	const std::vector<Vertex>& get_vertices() const { return vertices; }
	// Returns the bounding box of the vertices, in the local space of the geometry
	const Bounding_Box& get_bounding_box() const { return bounding_box; }

private:
	// Constructors
	// ------------
	// Uploads the vertices and indices to the GPU (use get_or_create() to share identical geometries)
	Mesh_Geometry(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::size_t hash);

	// Private Static Attributes
	// -------------------------
	static std::size_t live_count; // number of geometries currently alive

	// Private Static Methods
	// ----------------------
	// Returns the cache of the existing geometries, grouped by the hash of their data (weak pointers,
	// so that the cache does not keep them alive: a geometry is deleted when no mesh uses it anymore)
	static std::unordered_map<std::size_t, std::vector<std::weak_ptr<Mesh_Geometry>>>& get_cache();

	// Returns the hash of the given vertex and index data
	static std::size_t compute_hash(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Bounding_Box        bounding_box;                 // bounding box of the vertices (local space)
	std::size_t         hash{ 0 };                    // hash of the vertex and index data (key in the cache)
	GLuint              vao{ 0 }, vbo{ 0 }, ebo{ 0 }; // buffer objects/arrays on the GPU
};
