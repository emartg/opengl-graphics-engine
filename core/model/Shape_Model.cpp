/*
* Shape_Model.cpp
* This file implements the Shape_Model class (a derived class of Model),
* which is used to create and draw geometric shapes from vertex, normal, texture coordinate,
* index and texture data (if added).
*/

#include "Shape_Model.h"

// Constructors
// ------------
Shape_Model::Shape_Model(const std::string& name,
						 const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
						 const glm::vec4 albedo,
						 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 mesh_forward)
	: Model(name, Node_Type::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, mesh_forward),
	vertices{ process_vertex_data(vertices) }, indices{ indices }
{
	create_mesh();
}

Shape_Model::Shape_Model(const std::string& name,
						 const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
						 const std::vector<GLfloat> tex_coords, const std::vector<GLuint> indices,
						 const glm::vec4 albedo,
						 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 mesh_forward)
	: Model(name, Node_Type::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, mesh_forward),
	vertices{ process_vertex_data(positions, normals, tex_coords) }, indices{ indices }
{
	create_mesh();
}

Shape_Model::Shape_Model(const std::string& name,
						 const GLfloat* vertices, const GLuint n_vertices,
						 const GLuint* indices, const GLuint n_indices,
						 const glm::vec4 albedo,
						 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 mesh_forward)
	: Model(name, Node_Type::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, mesh_forward)
{
	std::vector<GLfloat> vertex_data{ vertices, vertices + n_vertices * 8 };
	std::vector<GLuint> index_data{ indices, indices + n_indices };
	this->vertices = process_vertex_data(vertex_data);
	this->indices = index_data;

	create_mesh();
}

Shape_Model::Shape_Model(const std::string& name,
						 const GLfloat* positions, const GLfloat* normals,
						 const GLfloat* tex_coords, const GLuint n_vertices,
						 const GLuint* indices, const GLuint n_indices,
						 const glm::vec4 albedo,
						 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 mesh_forward)
	: Model(name, Node_Type::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, mesh_forward)
{
	std::vector<GLfloat> position_data{ positions, positions + n_vertices * 3 };
	std::vector<GLfloat> normal_data{ normals, normals + n_vertices * 3 };
	std::vector<GLfloat> tex_coord_data{ tex_coords, tex_coords + n_vertices * 2 };
	std::vector<GLuint> index_data{ indices, indices + n_indices };
	this->vertices = process_vertex_data(position_data, normal_data, tex_coord_data);
	this->indices = index_data;

	create_mesh();
}

// Public Methods
// --------------
void Shape_Model::add_texture_data(const std::vector<std::shared_ptr<Texture>>& textures)
{
	this->textures = textures;
	meshes.clear();
	create_mesh();
}

// Private Methods
// ---------------
std::vector<Vertex> Shape_Model::process_vertex_data(std::vector<GLfloat> vertices)
{
	std::vector<Vertex> vertex_data;
	for (GLuint i{}; i < vertices.size() / 8; ++i)
	{
		Vertex vertex;
		vertex.position = glm::vec3(vertices[i * 8], vertices[i * 8 + 1], vertices[i * 8 + 2]);
		vertex.normal = glm::vec3(vertices[i * 8 + 3], vertices[i * 8 + 4], vertices[i * 8 + 5]);
		vertex.tex_coords = glm::vec2(vertices[i * 8 + 6], vertices[i * 8 + 7]);
		vertex_data.push_back(vertex);
	}
	return vertex_data;
}

std::vector<Vertex> Shape_Model::process_vertex_data(std::vector<GLfloat> vertices, std::vector<GLfloat> normals,
													 std::vector<GLfloat> tex_coords)
{
	std::vector<Vertex> vertex_data;
	for (GLuint i{}; i < vertices.size() / 3; i++)
	{
		Vertex vertex;
		vertex.position = glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		vertex.normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		vertex.tex_coords = glm::vec2(tex_coords[i * 2], tex_coords[i * 2 + 1]);
		vertex_data.push_back(vertex);
	}
	return vertex_data;
}

void Shape_Model::create_mesh() { meshes.emplace_back(std::make_shared<Mesh>(vertices, indices, textures)); }

