/*
 * CameraTest.cpp
 * This file contains the unit tests of the projection matrix of the Camera class: the perspective projection that the renderer
 * uses, which depends on the zoom of the camera and on the aspect ratio that is given.
 */

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gtest/gtest.h>

#include "core/camera/Camera.h"

namespace
{
	// Returns true when every element of the two matrices differs by less than a small tolerance
	bool are_equal(const glm::mat4& a, const glm::mat4& b)
	{
		for (int column{ 0 }; column < 4; ++column)
			for (int row{ 0 }; row < 4; ++row)
				if (std::abs(a[column][row] - b[column][row]) > 1e-6f)
					return false;
		return true;
	}
} // namespace

TEST(CameraTest, TheProjectionIsThePerspectiveOfTheZoomWithTheGivenAspectRatioAndPlanes)
{
	const Camera camera{ "Camera" };

	const glm::mat4 expected = glm::perspective(glm::radians(camera.get_zoom()), 1.6f, Camera::NEAR_PLANE, Camera::FAR_PLANE);

	EXPECT_TRUE(are_equal(camera.get_projection_matrix(1.6f), expected));
}

TEST(CameraTest, TheProjectionFollowsTheZoomOfTheCamera)
{
	Camera          camera{ "Camera" };
	const glm::mat4 before = camera.get_projection_matrix(1.0f);

	camera.process_mouse_scroll(5.0f); // changes the zoom (the field of view) of the camera

	const glm::mat4 expected = glm::perspective(glm::radians(camera.get_zoom()), 1.0f, Camera::NEAR_PLANE, Camera::FAR_PLANE);
	EXPECT_FALSE(are_equal(camera.get_projection_matrix(1.0f), before));
	EXPECT_TRUE(are_equal(camera.get_projection_matrix(1.0f), expected));
}

TEST(CameraTest, TheProjectionOfAWiderWindowIsWiderThanTheProjectionOfASquareOne)
{
	const Camera camera{ "Camera" };

	// the horizontal scale is the vertical scale divided by the aspect ratio
	EXPECT_NEAR(camera.get_projection_matrix(2.0f)[0][0] * 2.0f, camera.get_projection_matrix(1.0f)[0][0], 1e-6f);
}
