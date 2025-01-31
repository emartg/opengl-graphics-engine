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

#include "../shader/Shader.h"
#include "../texture/Texture.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Mesh
{
public:
	// Public Attributes
	// ---------------
	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	// Constructors
	// ------------
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

	// Public Functions
	// --------------
	// Renders the mesh using the provided shader
	void Draw(Shader& shader) const;

	// Deletes all the buffer objects/arrays
	void DeallocateResources();

private:
	// Private Attributes
	// ------------------
	// Render data
	GLuint VAO, VBO, EBO;

	// Private Functions
	// -----------------
	// Initializes all the buffer objects/arrays
	void setupMesh();

};
