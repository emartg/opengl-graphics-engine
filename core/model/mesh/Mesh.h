/*
* Mesh.h
* This file defines the Mesh class, which is used to store mesh data and render it.
*/

#pragma once

#include <string>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../shader/Shader.h"
#include "../../texture/Texture.h"

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
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

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
	std::vector<Texture> textures;
	GLuint VAO, VBO, EBO;

	// Private Methods
	// ---------------
	// Initializes all the buffer objects/arrays
	void setupMesh();

};
