/*
* Random.cpp
* This file implements the Random class, which is a utility class
* used to generate random numbers, vectors, etc.
*/

#include "Random.h"

// Private Methods
// ---------------
glm::vec3 Random::GenerateRandomColor() const
{
	glm::vec3 newColor{ (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };
	return newColor;
}

glm::vec3 Random::GenerateRandomPosition(glm::vec3 target, float minDistanceFromTarget, float maxDistanceFromTarget) const
{
	glm::vec3 newPos{ target.x + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget),
					  target.y + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget),
					  target.z + (rand() % 100) / 100.0f * (maxDistanceFromTarget - minDistanceFromTarget) };
	return newPos;
}