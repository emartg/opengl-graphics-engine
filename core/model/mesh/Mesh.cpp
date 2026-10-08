/*
 * Mesh.cpp
 * This file implements the Mesh class, which is used to store mesh data and render it.
 * A mesh combines a geometry (vertices and indices on the GPU), shared with every mesh
 * that has identical data, and its own material (if any).
 */

#include "Mesh.h"

#include "../../material/Material.h"

#include <utility>

// Constructors
// ------------
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::shared_ptr<Material> material) :
	geometry{ Mesh_Geometry::get_or_create(vertices, indices) },
	material{ std::move(material) }
{}

// Public methods
// --------------
void Mesh::deallocate_resources()
{
	geometry.reset();
}

void Mesh::draw() const
{
	if (geometry)
		geometry->draw();
}

const std::vector<Vertex>& Mesh::get_vertices() const
{
	static const std::vector<Vertex> no_vertices; // returned once the geometry has been released
	return geometry ? geometry->get_vertices() : no_vertices;
}

const Bounding_Box& Mesh::get_bounding_box() const
{
	static const Bounding_Box no_bounding_box; // invalid box, returned once the geometry has been released
	return geometry ? geometry->get_bounding_box() : no_bounding_box;
}
