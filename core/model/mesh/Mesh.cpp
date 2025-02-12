/*
* Mesh.cpp
* This file implements the Mesh class, which is used to store mesh data and render it.
*/

#include "Mesh.h"

// Constructors
// ------------
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
	: vertices(vertices), indices(indices), textures(textures)
{
	setupMesh();
}

// Public methods
// --------------
void Mesh::Draw() const
{
	// draw mesh
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// unbind the VAO
	glBindVertexArray(0);
}

void Mesh::BindTextures(Shader& shader) const
{
	// bind appropriate textures
	GLuint diffuseTextureMapIdx{ 1 }, specularTextureMapIdx{ 1 };
	for (GLuint i{}; i < textures.size(); i++)
	{
		std::string textureNumber, textureName;
		TextureType type = textures[i].GetTextureType();
		if (type == TextureType::DIFFUSE)
		{
			textureName = "diffuse";
			textureNumber = std::to_string(diffuseTextureMapIdx++);
		}
		else if (type == TextureType::SPECULAR)
		{
			textureName = "specular";
			textureNumber = std::to_string(specularTextureMapIdx++);
		}

		// set the sampler to the correct texture unit and bind the texture to it
		shader.SetInt("material." + textureName + textureNumber, i);
		textures[i].Bind(i);
	}
	glActiveTexture(GL_TEXTURE0); // set the active texture unit back to 0 once all textures are bound
}

void Mesh::DeallocateResources()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

// Private Methods
// ---------------
void Mesh::setupMesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind the VAO
	glBindVertexArray(VAO);

	// bind the VBO and send the vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// bind the EBO and send the indices to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// ---------------------------------
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// unbind the VAO
	glBindVertexArray(0);
}