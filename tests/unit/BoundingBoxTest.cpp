/*
 * BoundingBoxTest.cpp
 * This file contains the unit tests of the Bounding_Box class: construction, expansion,
 * and transformation of axis-aligned bounding boxes.
 */

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gtest/gtest.h>

#include "core/utils/geometry/Bounding_Box.h"

namespace
{
	constexpr float TOLERANCE{ 1e-5f }; // tolerance of the floating-point comparisons

	// Expects two vectors to be equal within the tolerance
	void expect_vec3_near(const glm::vec3& actual, const glm::vec3& expected)
	{
		EXPECT_NEAR(actual.x, expected.x, TOLERANCE);
		EXPECT_NEAR(actual.y, expected.y, TOLERANCE);
		EXPECT_NEAR(actual.z, expected.z, TOLERANCE);
	}
} // namespace

TEST(BoundingBoxTest, DefaultBoxIsInvalid)
{
	EXPECT_FALSE(Bounding_Box{}.is_valid());
}

TEST(BoundingBoxTest, CornersAreAcceptedInAnyOrder)
{
	const Bounding_Box box{ glm::vec3{ 1.0f, -2.0f, 3.0f }, glm::vec3{ -1.0f, 2.0f, -3.0f } };

	ASSERT_TRUE(box.is_valid());
	expect_vec3_near(box.get_min(), glm::vec3{ -1.0f, -2.0f, -3.0f });
	expect_vec3_near(box.get_max(), glm::vec3{ 1.0f, 2.0f, 3.0f });
	expect_vec3_near(box.get_center(), glm::vec3{ 0.0f });
	expect_vec3_near(box.get_size(), glm::vec3{ 2.0f, 4.0f, 6.0f });
}

TEST(BoundingBoxTest, ExpandingWithPointsContainsAllOfThem)
{
	Bounding_Box box{};
	box.expand(glm::vec3{ -5.0f, -6.0f, -7.0f }); // only negative coordinates, on purpose
	box.expand(glm::vec3{ -1.0f, -9.0f, -2.0f });

	ASSERT_TRUE(box.is_valid());
	expect_vec3_near(box.get_min(), glm::vec3{ -5.0f, -9.0f, -7.0f });
	expect_vec3_near(box.get_max(), glm::vec3{ -1.0f, -6.0f, -2.0f });
}

TEST(BoundingBoxTest, ExpandingWithAnInvalidBoxChangesNothing)
{
	Bounding_Box box{ glm::vec3{ 0.0f }, glm::vec3{ 1.0f } };
	box.expand(Bounding_Box{});

	expect_vec3_near(box.get_min(), glm::vec3{ 0.0f });
	expect_vec3_near(box.get_max(), glm::vec3{ 1.0f });
}

TEST(BoundingBoxTest, ExpandingWithABoxContainsBoth)
{
	Bounding_Box box{ glm::vec3{ 0.0f }, glm::vec3{ 1.0f } };
	box.expand(Bounding_Box{ glm::vec3{ -2.0f, 0.5f, 0.5f }, glm::vec3{ -1.0f, 3.0f, 0.5f } });

	expect_vec3_near(box.get_min(), glm::vec3{ -2.0f, 0.0f, 0.0f });
	expect_vec3_near(box.get_max(), glm::vec3{ 1.0f, 3.0f, 1.0f });
}

TEST(BoundingBoxTest, TransformedBoxIsTranslatedAndScaled)
{
	const Bounding_Box box{ glm::vec3{ -1.0f }, glm::vec3{ 1.0f } };
	glm::mat4          transform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 10.0f, 0.0f, -5.0f });
	transform                    = glm::scale(transform, glm::vec3{ 2.0f, 3.0f, 4.0f });

	const Bounding_Box result = box.transformed(transform);

	expect_vec3_near(result.get_min(), glm::vec3{ 8.0f, -3.0f, -9.0f });
	expect_vec3_near(result.get_max(), glm::vec3{ 12.0f, 3.0f, -1.0f });
}

TEST(BoundingBoxTest, TransformedBoxContainsTheRotatedCorners)
{
	// a unit cube rotated 45 degrees around the y axis spans sqrt(2) along x and z
	const Bounding_Box box{ glm::vec3{ -0.5f }, glm::vec3{ 0.5f } };
	const glm::mat4    rotation = glm::rotate(glm::mat4{ 1.0f }, glm::radians(45.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });

	const Bounding_Box result = box.transformed(rotation);
	const float        half   = std::sqrt(2.0f) * 0.5f;

	expect_vec3_near(result.get_min(), glm::vec3{ -half, -0.5f, -half });
	expect_vec3_near(result.get_max(), glm::vec3{ half, 0.5f, half });
}

TEST(BoundingBoxTest, TransformedInvalidBoxIsInvalid)
{
	EXPECT_FALSE(Bounding_Box{}.transformed(glm::mat4{ 1.0f }).is_valid());
}
