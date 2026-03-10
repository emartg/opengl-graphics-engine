/*
* Random.h
* This file defines the Random class, which is a utility class
* used to generate random numbers, vectors, etc.
*/

#pragma once

#include <iostream>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Random
{
public:
	// Constructors
	// ------------
	Random() = default;
	Random(const Random&) = default; // copy constructor 
	Random(Random&&) = default; // move constructor

	// Operator overloading
	// --------------------
	Random& operator=(const Random&) = default; // copy assignment operator
	Random& operator=(Random&&) = default; // move assignment operator

	// Destructor
	// ----------
	~Random() = default;

	// Public Methods
	// --------------
	// Generate a random float within a specified range [min_value, max_value]
	const float generate_random_float(float min_value, float max_value) const;
	// Generate a random color with RGBA components
	const glm::vec4 generate_random_color() const;
	// Generate a random position within a certain range with respect to a target
	const glm::vec3 generate_random_position(glm::vec3 target,
											 float min_distance_from_target,
											 float max_distance_from_target) const;
	// Generate a random rotation quaternion
	const glm::quat generate_random_rotation() const;
	// Genearate a random rotation quaternion with a specified angle range
	const glm::quat generate_random_rotation(float min_angle, float max_angle) const;
	// Generate a random rotation quaternion with a specified angle range and axis
	const glm::quat generate_random_rotation(float min_angle, float max_angle, glm::vec3 axis) const;
	// Generate a random direction vector (unit vector)
	const glm::vec3 generate_random_direction() const;

};