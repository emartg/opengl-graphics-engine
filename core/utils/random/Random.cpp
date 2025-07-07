/*
* Random.cpp
* This file implements the Random class, which is a utility class
* used to generate random numbers, vectors, etc.
*/

#include "Random.h"

// Private Methods
// ---------------
const glm::vec3 Random::GenerateRandomColor() const
{
	glm::vec3 newColor{ (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };
	return newColor;
}

const glm::vec3 Random::GenerateRandomPosition(glm::vec3 target,
											   float minDistanceFromTarget,
											   float maxDistanceFromTarget) const
{
	glm::vec3 newPos{ target.x + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget),
					  target.y + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget),
					  target.z + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget)
	};

	return newPos;
}

const glm::quat Random::GenerateRandomRotation() const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();

	// convert spherical coordinates to Cartesian coordinates
	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return glm::quat(glm::vec3(x, y, z)); // create quaternion from vector of Cartesian coordinates
}

const glm::quat Random::GenerateRandomRotation(float minAngle, float maxAngle) const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();
	// convert spherical coordinates to Cartesian coordinates
	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);
	glm::vec3 randomAxis = glm::normalize(glm::vec3(x, y, z)); // normalize the vector to get a random axis

	// generate a random angle between minAngle and maxAngle
	float angle = minAngle
		+ static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (maxAngle - minAngle);

	return glm::angleAxis(glm::radians(angle), randomAxis); // create quaternion from axis and angle
}

const glm::quat Random::GenerateRandomRotation(float minAngle, float maxAngle, glm::vec3 axis) const
{
	// generate a random angle between minAngle and maxAngle
	float angle = minAngle
		+ static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (maxAngle - minAngle);

	return glm::angleAxis(glm::radians(angle), glm::normalize(axis)); // create quaternion from axis and angle
}

const glm::vec3 Random::GenerateRandomDirection() const
{
	// generate random spherical coordinates (angles theta and phi in radians)
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>() * 2.0f;
	float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * glm::pi<float>();

	// convert spherical coordinates to Cartesian coordinates
	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return glm::normalize(glm::vec3(x, y, z)); // return normalized vector
}