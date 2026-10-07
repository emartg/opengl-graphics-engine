/*
 * Random.cpp
 * This file implements the Random class, which is a utility class
 * used to generate random numbers, vectors, etc.
 */

#include "Random.h"

#include <cmath>

#include <glm/gtc/constants.hpp>

// Constructors
// ------------
Random::Random() : engine{ std::random_device{}() } {}

Random::Random(std::uint32_t seed) : engine{ seed } {}

// Public Methods
// --------------
float Random::generate_random_float(float min_value, float max_value)
{
	std::uniform_real_distribution<float> distribution{ min_value, max_value };
	return distribution(engine);
}

glm::vec4 Random::generate_random_color()
{
	return glm::vec4{
		generate_random_float(0.0f, 1.0f), // red
		generate_random_float(0.0f, 1.0f), // green
		generate_random_float(0.0f, 1.0f), // blue
		1.0f                               // set alpha to 1.0f (fully opaque)
	};
}

glm::vec3 Random::generate_random_position(glm::vec3 target, float min_distance_from_target, float max_distance_from_target)
{
	// move from the target along a random direction, by a random distance within the range
	const float distance = generate_random_float(min_distance_from_target, max_distance_from_target);
	return target + generate_random_direction() * distance;
}

glm::quat Random::generate_random_rotation()
{
	// uniformly distributed random rotation (Shoemake's method), built from three uniform random numbers
	const float u1 = generate_random_float(0.0f, 1.0f);
	const float u2 = generate_random_float(0.0f, glm::two_pi<float>());
	const float u3 = generate_random_float(0.0f, glm::two_pi<float>());

	const float a = std::sqrt(1.0f - u1);
	const float b = std::sqrt(u1);

	return glm::quat{ a * std::cos(u2), a * std::sin(u2), b * std::sin(u3), b * std::cos(u3) }; // (w, x, y, z)
}

glm::quat Random::generate_random_rotation(float min_angle, float max_angle)
{
	// rotate around a random axis
	return generate_random_rotation(min_angle, max_angle, generate_random_direction());
}

glm::quat Random::generate_random_rotation(float min_angle, float max_angle, glm::vec3 axis)
{
	// generate a random angle between min_angle and max_angle
	const float angle = generate_random_float(min_angle, max_angle);

	// create quaternion from axis and angle
	return glm::angleAxis(glm::radians(angle), glm::normalize(axis));
}

glm::vec3 Random::generate_random_direction()
{
	// uniformly distributed point on the unit sphere: a random height (z) and a random angle around the z axis
	const float z     = generate_random_float(-1.0f, 1.0f);
	const float theta = generate_random_float(0.0f, glm::two_pi<float>());
	const float r     = std::sqrt(1.0f - z * z); // radius of the circle at height z

	return glm::vec3{ r * std::cos(theta), r * std::sin(theta), z };
}
