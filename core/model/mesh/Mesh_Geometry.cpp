/*
 * Mesh_Geometry.cpp
 * This file implements the Mesh_Geometry class, which stores the vertices and indices of a mesh on the GPU
 * (vertex array, vertex buffer, and element buffer objects).
 * Geometries are shared: meshes with identical vertex and index data use the same geometry,
 * so that it is only uploaded once (e.g., many cubes), and they can be drawn with instancing.
 */

#include "Mesh_Geometry.h"

#include <algorithm>
#include <cstring>

// Private Static Attributes
// -------------------------
std::size_t Mesh_Geometry::live_count{ 0 };

// Constructors
// ------------
Mesh_Geometry::Mesh_Geometry(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::size_t hash) :
	vertices{ vertices },
	indices{ indices },
	hash{ hash }
{
	// compute the bounding box of the geometry from its vertices (e.g., for frustum culling)
	for (const auto& vertex : this->vertices) bounding_box.expand(vertex.position);

	// create buffers/arrays
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	// bind the VAO
	glBindVertexArray(vao);

	// bind the VBO and send the vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);

	// bind the EBO and send the indices to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), this->indices.data(), GL_STATIC_DRAW);

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

	// unbind the VAO first (so that it keeps the EBO binding), and then the VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	++live_count;
}

// Destructor
// ----------
Mesh_Geometry::~Mesh_Geometry()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	// remove the expired entries of this geometry's hash from the cache (including this one, whose last
	// shared pointer has already been released when its destructor runs)
	auto& cache = get_cache();
	auto  it    = cache.find(hash);
	if (it != cache.end())
	{
		std::erase_if(it->second, [](const std::weak_ptr<Mesh_Geometry>& geometry) { return geometry.expired(); });
		if (it->second.empty())
			cache.erase(it);
	}

	--live_count;
}

// Public Static Methods
// ---------------------
std::shared_ptr<Mesh_Geometry> Mesh_Geometry::get_or_create(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
{
	const std::size_t data_hash = compute_hash(vertices, indices);
	auto&             bucket    = get_cache()[data_hash];

	// reuse an existing geometry with identical data (the data is compared, since different data may have the same hash)
	for (const auto& cached : bucket)
	{
		auto geometry = cached.lock();
		if (geometry && geometry->vertices.size() == vertices.size() && geometry->indices == indices &&
			std::memcmp(geometry->vertices.data(), vertices.data(), vertices.size() * sizeof(Vertex)) == 0)
			return geometry;
	}

	// otherwise, create a new geometry and add it to the cache
	// (the constructor is private, so std::make_shared cannot be used)
	std::shared_ptr<Mesh_Geometry> geometry{ new Mesh_Geometry(vertices, indices, data_hash) };
	bucket.push_back(geometry);
	return geometry;
}

// Public Methods
// --------------
void Mesh_Geometry::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, get_index_count(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// Private Static Methods
// ----------------------
std::unordered_map<std::size_t, std::vector<std::weak_ptr<Mesh_Geometry>>>& Mesh_Geometry::get_cache()
{
	// function-local static, so that the cache is initialized on its first use
	static std::unordered_map<std::size_t, std::vector<std::weak_ptr<Mesh_Geometry>>> cache;
	return cache;
}

std::size_t Mesh_Geometry::compute_hash(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
{
	// FNV-1a hash of the raw bytes of the vertices and indices
	std::size_t hash{ 14695981039346656037ull };
	auto        add_bytes = [&hash](const void* data, std::size_t size) {
		const auto* bytes = static_cast<const unsigned char*>(data);
		for (std::size_t i = 0; i < size; ++i)
		{
			hash ^= bytes[i];
			hash *= 1099511628211ull;
		}
	};
	add_bytes(vertices.data(), vertices.size() * sizeof(Vertex));
	add_bytes(indices.data(), indices.size() * sizeof(GLuint));
	return hash;
}
