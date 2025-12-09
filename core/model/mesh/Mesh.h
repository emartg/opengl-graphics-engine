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
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh
{
public:
	// Constructors
	// ------------
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices,
		 std::vector<std::shared_ptr<Texture>> textures);

	// Public Methods
	// --------------
	// Renders the mesh
	void Draw() const;

	// Binds the textures of the mesh
	void BindTextures(Shader& shader) const;

	// Deletes all the buffer objects/arrays
	void DeallocateResources();

	// Getters
	std::vector<Vertex> GetVertices() const { return vertices; }

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<std::shared_ptr<Texture>> textures;
	GLuint VAO, VBO, EBO;

	// Private Methods
	// ---------------
	// Initializes all the buffer objects/arrays
	void setupMesh();

};
