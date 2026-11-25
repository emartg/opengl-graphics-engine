/*
 * Ray.h
 * This file defines the Ray struct, a half-line from an origin along a unit direction, such as the line from the camera through
 * a point of the view (Camera::get_ray), and its intersection with a horizontal plane (for example the ground, y = 0). It needs no
 * OpenGL context, so it is unit tested.
 */

#pragma once

#include <optional>

#include <glm/glm.hpp>

struct Ray
{
	// Public Attributes
	// -----------------
	glm::vec3 origin{ 0.0f };                 // the point where the ray starts, in world units
	glm::vec3 direction{ 0.0f, 0.0f, -1.0f }; // the direction of the ray (a unit vector)

	// Public Methods
	// --------------
	// Returns the point at the given distance from the origin along the ray
	glm::vec3 at(float distance) const { return origin + direction * distance; }

	// Returns the distance from the origin to the horizontal plane at the given height (y = height), or nothing when the ray is
	// parallel to the plane or points away from it (an origin on the plane is at distance 0)
	std::optional<float> intersect_horizontal_plane(float height) const;
};
