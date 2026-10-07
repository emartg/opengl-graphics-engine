/*
 * Bounding_Box.cpp
 * This file implements the Bounding_Box class, an axis-aligned bounding box (AABB) defined by its
 * minimum and maximum corners, used to approximate the volume of meshes and nodes (e.g., for culling).
 */

#include "Bounding_Box.h"

// Constructors
// ------------
Bounding_Box::Bounding_Box(const glm::vec3& min_corner, const glm::vec3& max_corner) :
	min_corner{ glm::min(min_corner, max_corner) }, // accept the corners in any order
	max_corner{ glm::max(min_corner, max_corner) }
{}

// Public Methods
// --------------
bool Bounding_Box::is_valid() const
{
	return min_corner.x <= max_corner.x && min_corner.y <= max_corner.y && min_corner.z <= max_corner.z;
}

void Bounding_Box::expand(const glm::vec3& point)
{
	min_corner = glm::min(min_corner, point);
	max_corner = glm::max(max_corner, point);
}

void Bounding_Box::expand(const Bounding_Box& other)
{
	if (!other.is_valid())
		return; // an empty box adds no point

	min_corner = glm::min(min_corner, other.min_corner);
	max_corner = glm::max(max_corner, other.max_corner);
}

Bounding_Box Bounding_Box::transformed(const glm::mat4& transform) const
{
	if (!is_valid())
		return {}; // an empty box remains empty

	// transform the center, and compute the extents of the transformed box from the absolute values of the
	// linear part of the matrix (Arvo's method), which is equivalent to transforming the 8 corners
	const glm::vec3 center  = glm::vec3(transform * glm::vec4(get_center(), 1.0f));
	const glm::vec3 extents = get_size() * 0.5f;

	glm::vec3 new_extents{ 0.0f };
	for (int column = 0; column < 3; ++column) new_extents += glm::abs(glm::vec3(transform[column])) * extents[column];

	return Bounding_Box{ center - new_extents, center + new_extents };
}
