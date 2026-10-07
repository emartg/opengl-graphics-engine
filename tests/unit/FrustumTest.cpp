/*
 * FrustumTest.cpp
 * This file contains the unit tests of the Frustum class: the planes extracted from a camera's
 * view-projection matrix, and the visibility tests of points and bounding boxes.
 */

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gtest/gtest.h>

#include "core/utils/geometry/Bounding_Box.h"
#include "core/utils/geometry/Frustum.h"

// Test fixture with the frustum of a camera at the origin looking down the -z axis
// (90 degrees of vertical field of view, square aspect ratio, near plane at 1 and far plane at 100)
class FrustumTest : public ::testing::Test
{
protected:
	const glm::mat4 view       = glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
	const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 100.0f);
	const Frustum   frustum{ projection * view };

	// Returns a cube of the given half size centered at the given point
	static Bounding_Box cube(const glm::vec3& center, float half_size)
	{
		return Bounding_Box{ center - glm::vec3{ half_size }, center + glm::vec3{ half_size } };
	}
};

TEST_F(FrustumTest, ContainsPointsInFrontOfTheCamera)
{
	EXPECT_TRUE(frustum.contains(glm::vec3{ 0.0f, 0.0f, -10.0f }));
	EXPECT_TRUE(frustum.contains(glm::vec3{ 4.0f, -4.0f, -5.0f })); // inside the 45-degree half angle
}

TEST_F(FrustumTest, DoesNotContainPointsOutsideTheView)
{
	EXPECT_FALSE(frustum.contains(glm::vec3{ 0.0f, 0.0f, 10.0f }));   // behind the camera
	EXPECT_FALSE(frustum.contains(glm::vec3{ 0.0f, 0.0f, -0.5f }));   // before the near plane
	EXPECT_FALSE(frustum.contains(glm::vec3{ 0.0f, 0.0f, -150.0f })); // beyond the far plane
	EXPECT_FALSE(frustum.contains(glm::vec3{ 20.0f, 0.0f, -10.0f })); // to the right of the view
	EXPECT_FALSE(frustum.contains(glm::vec3{ 0.0f, 20.0f, -10.0f })); // above the view
}

TEST_F(FrustumTest, IntersectsBoxesInsideTheView)
{
	EXPECT_TRUE(frustum.intersects(cube(glm::vec3{ 0.0f, 0.0f, -10.0f }, 1.0f)));
}

TEST_F(FrustumTest, IntersectsBoxesCrossingItsBoundary)
{
	// centered outside the view, but large enough to reach inside it
	EXPECT_TRUE(frustum.intersects(cube(glm::vec3{ 14.0f, 0.0f, -10.0f }, 5.0f)));
	// containing the camera
	EXPECT_TRUE(frustum.intersects(cube(glm::vec3{ 0.0f }, 2.0f)));
}

TEST_F(FrustumTest, DoesNotIntersectBoxesOutsideTheView)
{
	EXPECT_FALSE(frustum.intersects(cube(glm::vec3{ 0.0f, 0.0f, 10.0f }, 1.0f)));    // behind the camera
	EXPECT_FALSE(frustum.intersects(cube(glm::vec3{ 30.0f, 0.0f, -10.0f }, 1.0f)));  // to the right
	EXPECT_FALSE(frustum.intersects(cube(glm::vec3{ 0.0f, -30.0f, -10.0f }, 1.0f))); // below
	EXPECT_FALSE(frustum.intersects(cube(glm::vec3{ 0.0f, 0.0f, -200.0f }, 1.0f)));  // beyond the far plane
}

TEST_F(FrustumTest, DoesNotIntersectInvalidBoxes)
{
	EXPECT_FALSE(frustum.intersects(Bounding_Box{}));
}

TEST_F(FrustumTest, FollowsTheCameraTransform)
{
	// the same camera moved to (100, 0, 0) and looking down the +x axis
	const glm::mat4 moved_view =
		glm::lookAt(glm::vec3{ 100.0f, 0.0f, 0.0f }, glm::vec3{ 101.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
	const Frustum moved_frustum{ projection * moved_view };

	EXPECT_TRUE(moved_frustum.intersects(cube(glm::vec3{ 110.0f, 0.0f, 0.0f }, 1.0f)));
	EXPECT_FALSE(moved_frustum.intersects(cube(glm::vec3{ 0.0f, 0.0f, -10.0f }, 1.0f))); // visible from the origin only
}
