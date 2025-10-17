/*
* Mesh.cpp
* This file implements the Mesh class, which is used to store mesh data and render it.
*/

#include "Mesh.h"

// Constructors
// ------------
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
	: vertices{ vertices }, indices{ indices }, textures{ textures }
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
	// bind every texture to a unique texture unit so they are always accessible in the shader
	// (for the current shader, set only the expected sampler uniforms once)
	const GLuint albedoMapUnit{ 0 };
	const GLuint metallicMapUnit{ 1 };

	// fallback units in case the expected maps are not found
	GLuint nextAvailableUnit{ 2 };

	// indices of known texture types (to bind to fixed slots - initially -1 = not found)
	GLint albedoIdx{ -1 };		// index of the albedo map in the textures vector
	GLint metallicIdx{ -1 };	// index of the metallic map in the textures vector

	// tag-based selection of known texture types before relying on legacy filename-based hints
	for (GLuint i{}; i < textures.size(); ++i)
	{
		const TextureType type = textures[i].GetTextureType(); // retrieve texture type
		// assign indices based on texture type (if not already assigned)
		if (albedoIdx < 0 && (type == TextureType::DIFFUSE || type == TextureType::AMBIENT))
			albedoIdx = static_cast<GLint>(i);
		else if (metallicIdx < 0 && (type == TextureType::SPECULAR || type == TextureType::METALNESS))
			metallicIdx = static_cast<GLint>(i);
	}

	// legacy filename-based hints if still unresolved (infer texture type from name)
	for (GLuint i = 0; i < textures.size() && (albedoIdx < 0 || metallicIdx < 0); ++i)
	{
		// skip already assigned textures
		if (textures[i].GetTextureType() != TextureType::UNDEFINED) continue;

		const std::string& n = textures[i].GetName(); // retrieve texture name

		// infer type from name substrings (if not already assigned)
		if (albedoIdx < 0 && (n.find("albedo") != std::string::npos ||
							  n.find("diffuse") != std::string::npos ||
							  n.find("bcolor") != std::string::npos ||
							  n.find("basecolor") != std::string::npos))
			albedoIdx = static_cast<GLint>(i);
		else if (metallicIdx < 0 && (n.find("specular") != std::string::npos ||
									 n.find("reflective") != std::string::npos ||
									 n.find("metallic") != std::string::npos ||
									 n.find("metal") != std::string::npos))
			metallicIdx = static_cast<GLint>(i);
	}

	// bind fixed slots first (if found)
	if (albedoIdx >= 0) // if an albedo map was found, bind it to the expected unit 
		textures[albedoIdx].Bind(albedoMapUnit);
	if (metallicIdx >= 0) // if a metallic map was found, bind it to the expected unit
		textures[metallicIdx].Bind(metallicMapUnit);

	// bind all remaining textures to subsequent units (always available for future shaders)
	for (GLuint i = 0; i < textures.size(); ++i)
	{
		// if this texture is already bound to a fixed slot, skip it
		if (static_cast<GLint>(i) == albedoIdx || static_cast<GLint>(i) == metallicIdx) continue;
		// otherwise, bind the texture to the next available unit
		textures[i].Bind(nextAvailableUnit++);
	}

	// communicate presence of known maps to the shader (per-mesh, which is more performant)
	shader.SetInt("material.hasAlbedoMap", albedoIdx >= 0 ? 1 : 0);
	shader.SetInt("material.hasMetallicMap", metallicIdx >= 0 ? 1 : 0);

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
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// unbind the VBO, EBO, and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}