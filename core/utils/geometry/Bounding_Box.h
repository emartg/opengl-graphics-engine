/*
 * Bounding_Box.h
 * This file defines the Bounding_Box class, an axis-aligned bounding box (AABB) defined by its
 * minimum and maximum corners, used to approximate the volume of meshes and nodes (e.g., for culling).
 */

#pragma once

#include <limits>

#include <glm/glm.hpp>

class Bounding_Box
{
public:
	// Constructors
	// ------------
	Bounding_Box() = default;                                               // empty (invalid) box, which contains no point
	Bounding_Box(const glm::vec3& min_corner, const glm::vec3& max_corner); // box from its corners

	// Public Methods
	// --------------
	// Returns whether the box contains at least one point (i.e., it has been expanded or built from corners)
	bool is_valid() const;

	// Expands the box to contain the given point or box (an invalid box is ignored)
	void expand(const glm::vec3& point);
	void expand(const Bounding_Box& other);

	// Returns the axis-aligned box that contains this box transformed by the given matrix
	// (e.g., the world-space box of a local-space box and a model matrix)
	Bounding_Box transformed(const glm::mat4& transform) const;

	// Getters
	const glm::vec3& get_min() const { return min_corner; }
	const glm::vec3& get_max() const { return max_corner; }
	glm::vec3        get_center() const { return (min_corner + max_corner) * 0.5f; }
	glm::vec3        get_size() const { return max_corner - min_corner; }

private:
	// Private Attributes
	// ------------------
	// the corners of an empty box are inverted, so that expanding it with any point sets both corners to it
	glm::vec3 min_corner{ std::numeric_limits<float>::max() };
	glm::vec3 max_corner{ std::numeric_limits<float>::lowest() };
};
