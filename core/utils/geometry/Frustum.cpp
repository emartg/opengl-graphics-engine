/*
 * Frustum.cpp
 * This file implements the Frustum class, which represents the view volume of a camera as six planes,
 * extracted from its view-projection matrix, and tests whether bounding boxes are inside it (e.g., for culling).
 */

#include "Frustum.h"

// Constructors
// ------------
Frustum::Frustum(const glm::mat4& view_projection)
{
	// extract the planes from the rows of the matrix (Gribb-Hartmann method): a point p is inside the
	// frustum when -w <= x, y, z <= w in clip space, i.e., when (row_3 +/- row_i) . (p, 1) >= 0
	// (GLM matrices are column-major, so the rows are read across the columns)
	const glm::mat4& m = view_projection;
	const glm::vec4  row_0{ m[0][0], m[1][0], m[2][0], m[3][0] };
	const glm::vec4  row_1{ m[0][1], m[1][1], m[2][1], m[3][1] };
	const glm::vec4  row_2{ m[0][2], m[1][2], m[2][2], m[3][2] };
	const glm::vec4  row_3{ m[0][3], m[1][3], m[2][3], m[3][3] };

	planes = {
		row_3 + row_0, // left
		row_3 - row_0, // right
		row_3 + row_1, // bottom
		row_3 - row_1, // top
		row_3 + row_2, // near
		row_3 - row_2  // far
	};

	// normalize the planes, so that the distances to them are measured in world units
	for (auto& plane : planes) plane /= glm::length(glm::vec3(plane));
}

// Public Methods
// --------------
bool Frustum::intersects(const Bounding_Box& box) const
{
	if (!box.is_valid())
		return false; // an empty box contains nothing to draw

	for (const auto& plane : planes)
	{
		// the corner of the box furthest along the plane's normal (the "positive vertex"): if even that corner
		// is behind the plane, the whole box is outside the frustum
		const glm::vec3 normal{ plane };
		const glm::vec3 positive_vertex{ normal.x >= 0.0f ? box.get_max().x : box.get_min().x,
										 normal.y >= 0.0f ? box.get_max().y : box.get_min().y,
										 normal.z >= 0.0f ? box.get_max().z : box.get_min().z };
		if (glm::dot(normal, positive_vertex) + plane.w < 0.0f)
			return false;
	}
	return true;
}

bool Frustum::contains(const glm::vec3& point) const
{
	for (const auto& plane : planes)
		if (glm::dot(glm::vec3(plane), point) + plane.w < 0.0f)
			return false;
	return true;
}
