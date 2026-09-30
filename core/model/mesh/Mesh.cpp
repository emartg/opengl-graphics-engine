/*
 * Mesh.cpp
 * This file implements the Mesh class, which is used to store mesh data and render it.
 */

#include "Mesh.h"

#include "../../shader/Shader.h"
#include "../../texture/Texture.h"

// Constructors
// ------------
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<std::shared_ptr<Texture>> textures) :
    vertices{ vertices },
    indices{ indices },
    textures{ textures }
{
	setup_mesh();
}

// Public methods
// --------------
void Mesh::deallocate_resources()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void Mesh::draw() const
{
	// draw mesh
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// unbind the VAO
	glBindVertexArray(0);
}

void Mesh::bind_textures(Shader& shader) const
{
	// bind every texture to a unique texture unit so they are always accessible in the shader
	// (for the current shader, set only the expected sampler uniforms once)
	const GLuint albedo_map_unit{ 0 };
	const GLuint metallic_map_unit{ 1 };
	const GLuint opacity_map_unit{ 2 };

	// fallback units in case the expected maps are not found
	GLuint next_available_unit{ 3 };

	// indices of known texture types (to bind to fixed slots - initially -1 = not found)
	GLint albedo_idx{ -1 };   // index of the albedo map in the textures vector
	GLint metallic_idx{ -1 }; // index of the metallic map in the textures vector
	GLint opacity_idx{ -1 };  // index of the opacity map in the textures vector

	// tag-based selection of known texture types before relying on legacy filename-based hints
	for (GLuint i{}; i < textures.size(); ++i)
	{
		const Texture_Type type = textures[i]->get_texture_type(); // retrieve texture type
		// assign indices based on texture type (if not already assigned)
		if (albedo_idx < 0 && (type == Texture_Type::DIFFUSE || type == Texture_Type::AMBIENT))
			albedo_idx = static_cast<GLint>(i);
		else if (metallic_idx < 0 && (type == Texture_Type::SPECULAR || type == Texture_Type::METALNESS))
			metallic_idx = static_cast<GLint>(i);
		else if (opacity_idx < 0 && type == Texture_Type::OPACITY)
			opacity_idx = static_cast<GLint>(i);
	}

	// legacy filename-based hints if still unresolved (infer texture type from name)
	for (GLuint i = 0; i < textures.size() && (albedo_idx < 0 || metallic_idx < 0); ++i)
	{
		// skip already assigned textures
		if (textures[i]->get_texture_type() != Texture_Type::UNDEFINED)
			continue;

		const std::string name = textures[i]->get_name(); // retrieve texture name

		// infer type from name substrings (if not already assigned)
		if (albedo_idx < 0 &&
		    (name.find("albedo") != std::string::npos || name.find("diffuse") != std::string::npos ||
		     name.find("basecolor") != std::string::npos || name.find("bcolor") != std::string::npos))
			albedo_idx = static_cast<GLint>(i);
		else if (
		    metallic_idx < 0 &&
		    (name.find("specular") != std::string::npos || name.find("reflective") != std::string::npos ||
			 name.find("metal") != std::string::npos || name.find("metallic") != std::string::npos))
			metallic_idx = static_cast<GLint>(i);
		else if (
		    opacity_idx < 0 &&
		    (name.find("opacity") != std::string::npos || name.find("alpha") != std::string::npos ||
			 name.find("transparent") != std::string::npos || name.find("transparency") != std::string::npos))
			opacity_idx = static_cast<GLint>(i);
	}

	// bind fixed slots first (if found)
	if (albedo_idx >= 0) // if an albedo map was found, bind it to the expected unit
		textures[albedo_idx]->bind(albedo_map_unit);
	if (metallic_idx >= 0) // if a metallic map was found, bind it to the expected unit
		textures[metallic_idx]->bind(metallic_map_unit);
	if (opacity_idx >= 0) // if an opacity map was found, bind it to the expected unit
		textures[opacity_idx]->bind(opacity_map_unit);

	// bind all remaining textures to subsequent units (always available for future shaders)
	for (GLuint i = 0; i < textures.size(); ++i)
	{
		// if this texture is already bound to a fixed slot, skip it
		if (static_cast<GLint>(i) == albedo_idx || static_cast<GLint>(i) == metallic_idx || static_cast<GLint>(i) == opacity_idx)
			continue;
		// otherwise, bind the texture to the next available unit
		textures[i]->bind(next_available_unit++);
	}

	// communicate presence of known maps to the shader (per-mesh, which is more performant)
	shader.set_int("u_material.has_albedo_map", albedo_idx >= 0 ? 1 : 0);
	shader.set_int("u_material.has_metallic_map", metallic_idx >= 0 ? 1 : 0);
	shader.set_int("u_material.has_opacity_map", opacity_idx >= 0 ? 1 : 0);

	glActiveTexture(GL_TEXTURE0); // set the active texture unit back to 0 once all textures are bound
}

// Private Methods
// ---------------
void Mesh::setup_mesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	// bind the VAO
	glBindVertexArray(vao);

	// bind the VBO and send the vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// bind the EBO and send the indices to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

	// unbind the VBO, EBO, and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}