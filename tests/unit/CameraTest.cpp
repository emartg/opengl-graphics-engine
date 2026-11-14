/*
 * CameraTest.cpp
 * This file contains the unit tests of the Camera class: the perspective projection that the renderer, the picking, and the
 * outline of the selection use (which depends on the zoom of the camera, on the aspect ratio or the size of the window, and on
 * the clip planes of the camera), the clip planes and their validation, the setters of the position, the angles, and the zoom,
 * and look_at.
 */

#include <cmath>
#include <limits>

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

	// Returns true when the three components of a vector are finite numbers
	bool is_finite(const glm::vec3& vector)
	{
		return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
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

TEST(CameraTest, ACameraStartsWithTheDefaultClipPlanes)
{
	const Camera camera{ "Camera" };

	EXPECT_EQ(camera.get_near_plane(), Camera::NEAR_PLANE);
	EXPECT_EQ(camera.get_far_plane(), Camera::FAR_PLANE);
}

TEST(CameraTest, TheProjectionUsesTheClipPlanesThatAreSet)
{
	Camera camera{ "Camera" };

	ASSERT_TRUE(camera.set_clip_planes(1.0f, 5000.0f));

	EXPECT_EQ(camera.get_near_plane(), 1.0f);
	EXPECT_EQ(camera.get_far_plane(), 5000.0f);
	const glm::mat4 expected = glm::perspective(glm::radians(camera.get_zoom()), 1.5f, 1.0f, 5000.0f);
	EXPECT_TRUE(are_equal(camera.get_projection_matrix(1.5f), expected));
}

TEST(CameraTest, RejectsClipPlanesThatAreNotValidAndKeepsTheCurrentOnes)
{
	Camera camera{ "Camera" };
	ASSERT_TRUE(camera.set_clip_planes(0.5f, 50.0f));

	const GLfloat infinity{ std::numeric_limits<GLfloat>::infinity() };
	const GLfloat not_a_number{ std::numeric_limits<GLfloat>::quiet_NaN() };
	EXPECT_FALSE(camera.set_clip_planes(0.0f, 10.0f));  // the near plane must be greater than 0
	EXPECT_FALSE(camera.set_clip_planes(-1.0f, 10.0f)); // a plane behind the camera
	EXPECT_FALSE(camera.set_clip_planes(10.0f, 10.0f)); // the far plane must be greater than the near one
	EXPECT_FALSE(camera.set_clip_planes(10.0f, 1.0f));
	EXPECT_FALSE(camera.set_clip_planes(not_a_number, 10.0f));
	EXPECT_FALSE(camera.set_clip_planes(0.1f, infinity));

	EXPECT_EQ(camera.get_near_plane(), 0.5f);
	EXPECT_EQ(camera.get_far_plane(), 50.0f);
}

TEST(CameraTest, ResettingTheCameraKeepsItsClipPlanes)
{
	Camera        camera{ "Camera" };
	const GLfloat default_zoom{ camera.get_zoom() };
	ASSERT_TRUE(camera.set_clip_planes(2.0f, 200.0f));
	camera.process_mouse_scroll(5.0f);

	camera.Reset();

	EXPECT_EQ(camera.get_zoom(), default_zoom); // the view is reset
	EXPECT_EQ(camera.get_near_plane(), 2.0f);   // the planes, which fit the scale of the scene, are kept
	EXPECT_EQ(camera.get_far_plane(), 200.0f);
}

TEST(CameraTest, TheProjectionOfAWindowSizeIsTheProjectionOfItsAspectRatio)
{
	// the renderer (with the size of the window) and the picking and the outline (with the size of their framebuffers, which is
	// the size of the window) all call get_projection_matrix(width, height), so they get exactly this matrix
	Camera camera{ "Camera" };
	ASSERT_TRUE(camera.set_clip_planes(0.01f, 1000.0f));

	EXPECT_TRUE(camera.get_projection_matrix(1600u, 900u) == camera.get_projection_matrix(1600.0f / 900.0f));
	EXPECT_TRUE(camera.get_projection_matrix(1280u, 720u) == camera.get_projection_matrix(1280.0f / 720.0f));
}

TEST(CameraTest, AWindowOfHeightZeroGivesTheProjectionOfASquareWindow)
{
	const Camera camera{ "Camera" };

	// a minimized window has a framebuffer of height 0: the aspect ratio 1 keeps the matrix finite
	EXPECT_TRUE(camera.get_projection_matrix(800u, 0u) == camera.get_projection_matrix(1.0f));
}

TEST(CameraTest, SettingThePositionMovesTheCameraWithoutTurningIt)
{
	Camera          camera{ "Camera" };
	const glm::vec3 front{ camera.get_front() };
	const glm::vec3 up{ camera.get_up() };
	const glm::vec3 position{ 1.0f, 2.0f, 3.0f };

	camera.set_position(position);

	EXPECT_EQ(camera.get_position(), position);
	EXPECT_EQ(camera.get_front(), front);
	EXPECT_EQ(camera.get_up(), up);
	EXPECT_TRUE(are_equal(camera.get_view_matrix(), glm::lookAt(position, position + front, up)));
}

TEST(CameraTest, SettingTheYawAndThePitchTurnsTheCamera)
{
	Camera camera{ "Camera" };

	camera.set_yaw(0.0f);
	camera.set_pitch(0.0f);
	// with a yaw and a pitch of 0 the camera looks along the x axis
	EXPECT_NEAR(camera.get_front().x, 1.0f, 1e-6f);
	EXPECT_NEAR(camera.get_front().y, 0.0f, 1e-6f);
	EXPECT_NEAR(camera.get_front().z, 0.0f, 1e-6f);

	camera.set_yaw(90.0f);
	// a yaw of 90 degrees turns it to the z axis
	EXPECT_NEAR(camera.get_front().x, 0.0f, 1e-6f);
	EXPECT_NEAR(camera.get_front().z, 1.0f, 1e-6f);

	camera.set_pitch(30.0f);
	// a pitch of 30 degrees raises it by the sine of 30 degrees
	EXPECT_NEAR(camera.get_front().y, 0.5f, 1e-6f);
}

TEST(CameraTest, ThePitchThatIsSetIsLimitedSoThatTheCameraNeverLooksStraightUpOrDown)
{
	Camera camera{ "Camera" };

	camera.set_pitch(120.0f);
	EXPECT_EQ(camera.get_pitch(), 89.0f);
	EXPECT_TRUE(is_finite(camera.get_right()));

	camera.set_pitch(-95.0f);
	EXPECT_EQ(camera.get_pitch(), -89.0f);
	EXPECT_TRUE(is_finite(camera.get_right()));
}

TEST(CameraTest, SettingTheZoomChangesTheFieldOfViewWithinItsLimits)
{
	Camera camera{ "Camera" };

	camera.set_zoom(30.0f);
	EXPECT_EQ(camera.get_zoom(), 30.0f);
	const glm::mat4 expected = glm::perspective(glm::radians(30.0f), 1.0f, Camera::NEAR_PLANE, Camera::FAR_PLANE);
	EXPECT_TRUE(are_equal(camera.get_projection_matrix(1.0f), expected));

	// the same limits as the mouse wheel
	camera.set_zoom(0.0f);
	EXPECT_EQ(camera.get_zoom(), 1.0f);
	camera.set_zoom(150.0f);
	EXPECT_EQ(camera.get_zoom(), 90.0f);
}

TEST(CameraTest, LookingAtAPointTurnsTheCameraTowardsIt)
{
	Camera camera{ "Camera" };
	camera.set_position(glm::vec3{ 2.0f, 3.0f, 4.0f });

	const glm::vec3 target{ -1.0f, 1.0f, 0.5f };
	ASSERT_TRUE(camera.look_at(target));

	const glm::vec3 expected{ glm::normalize(target - camera.get_position()) };
	EXPECT_NEAR(camera.get_front().x, expected.x, 1e-5f);
	EXPECT_NEAR(camera.get_front().y, expected.y, 1e-5f);
	EXPECT_NEAR(camera.get_front().z, expected.z, 1e-5f);
	EXPECT_EQ(camera.get_position(), (glm::vec3{ 2.0f, 3.0f, 4.0f })); // looking does not move the camera
}

TEST(CameraTest, LookingStraightDownIsLimitedAndLookingAtItsOwnPositionIsRefused)
{
	Camera          camera{ "Camera" };
	const glm::vec3 front_before{ camera.get_front() };

	// a point at the position of the camera gives no direction: nothing changes
	EXPECT_FALSE(camera.look_at(camera.get_position()));
	EXPECT_EQ(camera.get_front(), front_before);

	// a point straight below is limited to the pitch of -89 degrees, so the vectors stay defined
	ASSERT_TRUE(camera.look_at(camera.get_position() - glm::vec3{ 0.0f, 10.0f, 0.0f }));
	EXPECT_EQ(camera.get_pitch(), -89.0f);
	EXPECT_TRUE(is_finite(camera.get_right()));
}
