/*
 * Frustum.h
 * This file defines the Frustum class, which represents the view volume of a camera as six planes,
 * extracted from its view-projection matrix, and tests whether bounding boxes are inside it (e.g., for culling).
 */

#pragma once

#include <array>

#include <glm/glm.hpp>

#include "Bounding_Box.h"

class Frustum
{
public:
	// Constructors
	// ------------
	// Builds the frustum from a view-projection matrix (projection * view) with OpenGL's clip space conventions,
	// whose planes are expressed in world space (or in the space the view matrix transforms from)
	explicit Frustum(const glm::mat4& view_projection);

	// Public Methods
	// --------------
	// Returns whether the box is at least partially inside the frustum. The test is conservative:
	// boxes outside the frustum but close to its corners may be reported as inside, but never the opposite
	bool intersects(const Bounding_Box& box) const;

	// Returns whether the point is inside the frustum (or on its boundary)
	bool contains(const glm::vec3& point) const;

private:
	// Private Attributes
	// ------------------
	// planes as (normal.x, normal.y, normal.z, distance), with normalized normals pointing inside the frustum,
	// in the order: left, right, bottom, top, near, far
	std::array<glm::vec4, 6> planes;
};
