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

	// Public Methods
	// --------------
	// Deletes all the buffer objects/arrays
	void deallocate_resources();

	// Renders the mesh
	void draw() const;

	// Binds the textures of the mesh
	void bind_textures(Shader& shader) const;

	// Getters
	std::vector<Vertex> get_vertices() const { return vertices; }

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex>                   vertices;
	std::vector<GLuint>                   indices;
	std::vector<std::shared_ptr<Texture>> textures;
	GLuint                                vao, vbo, ebo;

	// Private Methods
	// ---------------
	// Initializes all the buffer objects/arrays
	void setup_mesh();
};
