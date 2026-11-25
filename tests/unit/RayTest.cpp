/*
 * RayTest.cpp
 * This file contains the unit tests of the Ray struct: the points along a ray and its intersection with a horizontal plane (a ray
 * that goes down to the plane, one that goes away from it, one parallel to it, and one that starts on it).
 */

#include <cmath>
#include <optional>

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include "core/utils/geometry/Ray.h"

TEST(RayTest, APointAlongTheRayIsTheOriginPlusTheDirectionTimesTheDistance)
{
	const Ray ray{ glm::vec3{ 1.0f, 2.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f } };
	EXPECT_EQ(ray.at(4.0f), (glm::vec3{ 1.0f, 2.0f, -1.0f }));
}

TEST(RayTest, ARayGoingDownMeetsTheGroundAtTheRightDistance)
{
	const Ray                  ray{ glm::vec3{ 0.0f, 10.0f, 0.0f }, glm::normalize(glm::vec3{ 1.0f, -1.0f, 0.0f }) };
	const std::optional<float> distance{ ray.intersect_horizontal_plane(0.0f) };
	ASSERT_TRUE(distance.has_value());
	EXPECT_NEAR(*distance, 10.0f * std::sqrt(2.0f), 1e-4f);
	const glm::vec3 point{ ray.at(*distance) };
	EXPECT_NEAR(point.x, 10.0f, 1e-4f);
	EXPECT_NEAR(point.y, 0.0f, 1e-4f);
}

TEST(RayTest, ARayGoingAwayFromThePlaneOrParallelToItMissesIt)
{
	EXPECT_FALSE((Ray{ glm::vec3{ 0.0f, 10.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } }.intersect_horizontal_plane(0.0f)).has_value());
	EXPECT_FALSE((Ray{ glm::vec3{ 0.0f, 10.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f } }.intersect_horizontal_plane(0.0f)).has_value());
	// the plane can be at any height
	EXPECT_TRUE((Ray{ glm::vec3{ 0.0f, 10.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } }.intersect_horizontal_plane(15.0f)).has_value());
}

TEST(RayTest, ARayStartingOnThePlaneMeetsItAtDistanceZero)
{
	const std::optional<float> distance{ Ray{ glm::vec3{ 3.0f, 0.0f, 3.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f } }.intersect_horizontal_plane(
		0.0f) };
	ASSERT_TRUE(distance.has_value());
	EXPECT_FLOAT_EQ(*distance, 0.0f);
}
