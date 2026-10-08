/*
 * Mesh.h
 * This file defines the Mesh class, which is used to store mesh data and render it.
 * A mesh combines a geometry (vertices and indices on the GPU), shared with every mesh
 * that has identical data, and its own material (if any).
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations

#include "Mesh_Geometry.h" // also defines the Vertex struct

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Material;

class Mesh
{
public:
	// Constructors
	// ------------
	// Creates the mesh, reusing the geometry of an existing mesh with identical vertices and indices (if any),
	// with its own material (e.g., the material of an imported model), or without one (nullptr) to use the
	// material of its node or the default material
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::shared_ptr<Material> material = nullptr);

	// Public Methods
	// --------------
	// Releases the mesh's reference to its geometry (whose buffer objects are deleted once no mesh uses it);
	// safe to call more than once
	void deallocate_resources();

	// Renders the mesh
	void draw() const;

	// Getters
	const std::shared_ptr<Mesh_Geometry>& get_geometry() const { return geometry; }
	const std::shared_ptr<Material>&      get_material() const { return material; }
	const std::vector<Vertex>&            get_vertices() const;
	// Returns the bounding box of the vertices, in the local space of the mesh
	const Bounding_Box& get_bounding_box() const;

	// Setters
	void set_material(const std::shared_ptr<Material>& material) { this->material = material; }

private:
	// Private Attributes
	// ------------------
	std::shared_ptr<Mesh_Geometry> geometry; // geometry on the GPU (shared by meshes with identical data)
	std::shared_ptr<Material>      material; // own material of the mesh (nullptr if it has none)
};
