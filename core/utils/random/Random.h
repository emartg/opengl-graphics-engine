/*
 * Random.h
 * This file defines the Random class, which is a utility class
 * used to generate random numbers, vectors, etc.
 * Each instance owns its own random number generator (a Mersenne Twister engine), seeded from
 * a non-deterministic source by default, or with a fixed seed to get reproducible sequences.
 */

#pragma once

#include <cstdint>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Random
{
public:
	// Constructors
	// ------------
	Random();                            // seeds the generator from a non-deterministic source
	explicit Random(std::uint32_t seed); // seeds the generator with a fixed value (reproducible sequences)
	Random(const Random&) = default;     // copy constructor (the copy continues the same sequence)
	Random(Random&&)      = default;     // move constructor

	// Operator overloading
	// --------------------
	Random& operator=(const Random&) = default; // copy assignment operator
	Random& operator=(Random&&)      = default; // move assignment operator

	// Destructor
	// ----------
	~Random() = default;

	// Public Methods
	// --------------
	// Generate a random float within a specified range [min_value, max_value)
	float generate_random_float(float min_value, float max_value);
	// Generate a random color with RGB components in [0, 1) and an opaque alpha component
	glm::vec4 generate_random_color();
	// Generate a random position whose distance to the target is within [min_distance_from_target, max_distance_from_target)
	glm::vec3 generate_random_position(glm::vec3 target, float min_distance_from_target, float max_distance_from_target);
	// Generate a random rotation quaternion (uniformly distributed over all rotations)
	glm::quat generate_random_rotation();
	// Generate a random rotation quaternion around a random axis, with an angle in degrees within [min_angle, max_angle)
	glm::quat generate_random_rotation(float min_angle, float max_angle);
	// Generate a random rotation quaternion around the specified axis, with an angle in degrees within [min_angle, max_angle)
	glm::quat generate_random_rotation(float min_angle, float max_angle, glm::vec3 axis);
	// Generate a random direction vector (unit vector, uniformly distributed over the sphere)
	glm::vec3 generate_random_direction();

private:
	// Private Attributes
	// ------------------
	std::mt19937 engine; // random number generator
};
