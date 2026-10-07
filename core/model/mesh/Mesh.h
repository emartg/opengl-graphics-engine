/*
 * Mesh.h
 * This file defines the Mesh class, which is used to store mesh data and render it.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../utils/geometry/Bounding_Box.h"

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Shader;
class Texture;

// Struct that defines a single vertex of the mesh
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

class Mesh
{
public:
	// Constructors
	// ------------
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<std::shared_ptr<Texture>> textures);
	Mesh(const Mesh&) = delete; // copy constructor (a mesh owns its buffer objects, which cannot be shared)

	// Operator overloads
	// ------------------
	Mesh& operator=(const Mesh&) = delete; // copy assignment operator (a mesh owns its buffer objects)

	// Destructor
	// ----------
	// Deletes the buffer objects/arrays of the mesh (requires the OpenGL context to be current)
	~Mesh();

	// Public Methods
	// --------------
	// Deletes all the buffer objects/arrays (safe to call more than once)
	void deallocate_resources();

	// Renders the mesh
	void draw() const;

	// Binds the textures of the mesh
	void bind_textures(Shader& shader) const;

	// Getters
	const std::vector<Vertex>& get_vertices() const { return vertices; }
	// Returns the bounding box of the vertices, in the local space of the mesh
	const Bounding_Box& get_bounding_box() const { return bounding_box; }

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex>                   vertices;
	std::vector<GLuint>                   indices;
	std::vector<std::shared_ptr<Texture>> textures;
	GLuint                                vao{ 0 }, vbo{ 0 }, ebo{ 0 };
	Bounding_Box                          bounding_box; // bounding box of the vertices (local space)

	// Private Methods
	// ---------------
	// Initializes all the buffer objects/arrays
	void setup_mesh();
};
