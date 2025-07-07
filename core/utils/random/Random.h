/*
* Random.h
* This file defines the Random class, which is a utility class
* used to generate random numbers, vectors, etc.
*/

#pragma once

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
	Random(const Random&) = default; // Copy constructor 
	Random(Random&&) = default; // Move constructor

	// Operator overloading
	// --------------------
	Random& operator=(const Random&) = default; // Copy assignment operator
	Random& operator=(Random&&) = default; // Move assignment operator

	// Destructor
	// ----------
	~Random() = default;

	// Public Methods
	// --------------
	// Generate a random color
	const glm::vec3 GenerateRandomColor() const;

	// Generate a random position within a certain range with respect to a target
	const glm::vec3 GenerateRandomPosition(glm::vec3 target,
										   float minDistanceFromTarget,
										   float maxDistanceFromTarget) const;
	// Generate a random rotation quaternion
	const glm::quat GenerateRandomRotation() const;
	// Genearate a random rotation quaternion with a specified angle range
	const glm::quat GenerateRandomRotation(float minAngle, float maxAngle) const;
	// Generate a random rotation quaternion with a specified angle range and axis
	const glm::quat GenerateRandomRotation(float minAngle, float maxAngle, glm::vec3 axis) const;
	// Generate a random direction vector (unit vector)
	const glm::vec3 GenerateRandomDirection() const;

};