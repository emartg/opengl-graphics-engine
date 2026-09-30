/*
 * Shape_Model.h
 * This file defines the Shape_Model class (a derived class of Model),
 * which is used to create and draw geometric shapes from vertex, normal, texture coordinate,
 * index and texture data (if added).
 */

#pragma once

#include "Model.h"

#include <iostream>
#include <string>
#include <vector>

#include "mesh/Mesh.h"
#include "../texture/Texture.h"

class Shape_Model : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved position, normal and texture coordinate data,
	// and index data
	Shape_Model(
		const std::string&         name,
		const std::vector<GLfloat> vertices,
		const std::vector<GLuint>  indices,
		const glm::vec4            albedo       = ALBEDO,
		const glm::vec3            position     = POSITION,
		const glm::quat            rotation     = ROTATION,
		const glm::vec3            scale        = SCALE,
		const glm::vec3            forward      = FORWARD,
		const glm::vec3            mesh_forward = FORWARD);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data
	Shape_Model(
		const std::string&         name,
		const std::vector<GLfloat> positions,
		const std::vector<GLfloat> normals,
		const std::vector<GLfloat> tex_coords,
		const std::vector<GLuint>  indices,
		const glm::vec4            albedo       = ALBEDO,
		const glm::vec3            position     = POSITION,
		const glm::quat            rotation     = ROTATION,
		const glm::vec3            scale        = SCALE,
		const glm::vec3            forward      = FORWARD,
		const glm::vec3            mesh_forward = FORWARD);

	// Constructor that creates a shape from interleaved position, normal and texture coordinate data,
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape_Model(
		const std::string& name,
		const GLfloat*     vertices,
		const GLuint       n_vertices,
		const GLuint*      indices,
		const GLuint       n_indices,
		const glm::vec4    albedo       = ALBEDO,
		const glm::vec3    position     = POSITION,
		const glm::quat    rotation     = ROTATION,
		const glm::vec3    scale        = SCALE,
		const glm::vec3    forward      = FORWARD,
		const glm::vec3    mesh_forward = FORWARD);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape_Model(
		const std::string& name,
		const GLfloat*     positions,
		const GLfloat*     normals,
		const GLfloat*     tex_coords,
		const GLuint       n_vertices,
		const GLuint*      indices,
		const GLuint       n_indices,
		const glm::vec4    albedo       = ALBEDO,
		const glm::vec3    position     = POSITION,
		const glm::quat    rotation     = ROTATION,
		const glm::vec3    scale        = SCALE,
		const glm::vec3    forward      = FORWARD,
		const glm::vec3    mesh_forward = FORWARD);

	// Destructor
	// ----------
	~Shape_Model() = default;

	// Public Methods
	// --------------
	// Adds texture data to the shape
	void add_texture_data(const std::vector<std::shared_ptr<Texture>>& textures);

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex>                   vertices; // vertex, normal and texture coordinate data
	std::vector<GLuint>                   indices;  // indices that define the order in which the vertices are drawn
	std::vector<std::shared_ptr<Texture>> textures; // texture data (if any) associated with the shape

	// Private Methods
	// ---------------
	// Creates a vector of Vertex objects from interleaved vertex data
	std::vector<Vertex> process_vertex_data(std::vector<GLfloat> vertices);
	// Interleaves vertex, normal, and texture coordinate data into a single vector of Vertex objects
	std::vector<Vertex> process_vertex_data(std::vector<GLfloat> position, std::vector<GLfloat> normals, std::vector<GLfloat> tex_coords);

	// Creates the mesh from the vertex, index and texture data
	void create_mesh();
};
