/*
 * Random.cpp
 * This file implements the Random class, which is a utility class
 * used to generate random numbers, vectors, etc.
 */

#include "Random.h"

// Private Methods
// ---------------
const float Random::generate_random_float(float min_value, float max_value) const
{
	return min_value + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max_value - min_value);
}

const glm::vec4 Random::generate_random_color() const
{
	glm::vec4 newColor{
		(rand() % 100) / 100.0f,
		(rand() % 100) / 100.0f,
		(rand() % 100) / 100.0f,
		1.0f // set alpha to 1.0f (fully opaque)
	};
	return newColor;
}

const glm::vec3 Random::generate_random_position(glm::vec3 target, float min_distance_from_target, float max_distance_from_target) const
{
	glm::vec3 new_position{ target.x + (rand() % 100) / 100.0f * (max_distance_from_target - min_distance_from_target),
							target.y + (rand() % 100) / 100.0f * (max_distance_from_target - min_distance_from_target),
							target.z + (rand() % 100) / 100.0f * (max_distance_from_target - min_distance_from_target) };

	return new_position;
}

const glm::quat Random::generate_random_rotation() const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi   = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();

	// convert spherical coordinates to Cartesian coordinates
	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return glm::quat(glm::vec3(x, y, z)); // create quaternion from vector of Cartesian coordinates
}

const glm::quat Random::generate_random_rotation(float min_angle, float max_angle) const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi   = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();
	// convert spherical coordinates to Cartesian coordinates
	float     x           = sin(phi) * cos(theta);
	float     y           = sin(phi) * sin(theta);
	float     z           = cos(phi);
	glm::vec3 random_axis = glm::normalize(glm::vec3(x, y, z)); // normalize the vector to get a random axis

	// generate a random angle between min_angle and max_angle
	float angle = min_angle + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max_angle - min_angle);

	return glm::angleAxis(glm::radians(angle), random_axis); // create quaternion from axis and angle
}

const glm::quat Random::generate_random_rotation(float min_angle, float max_angle, glm::vec3 axis) const
{
	// generate a random angle between min_angle and max_angle
	float angle = min_angle + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max_angle - min_angle);

	// create quaternion from axis and angle
	return glm::angleAxis(glm::radians(angle), glm::normalize(axis));
}

const glm::vec3 Random::generate_random_direction() const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi   = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();

	// convert spherical coordinates to Cartesian coordinates
	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return glm::normalize(glm::vec3(x, y, z)); // return normalized vector
}
