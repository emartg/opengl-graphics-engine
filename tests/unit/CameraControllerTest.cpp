/*
 * CameraControllerTest.cpp
 * This file contains the unit tests of the Camera_Controller class: the orbit of a camera about a pivot (rigid, in the directions
 * of the mouse, and with the pitch limited), the pan (a point at the depth of the pivot follows the mouse), and the dolly (the
 * distance to the pivot multiplied by a factor, between limits that follow the clip planes).
 */

#include <cmath>

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include "core/camera/Camera.h"
#include "core/camera/Camera_Controller.h"

namespace
{
	// Returns a camera at the given position that looks at the given point
	Camera make_camera(const glm::vec3& position, const glm::vec3& target)
	{
		Camera camera{ "Camera" };
		camera.set_position(position);
		camera.look_at(target);
		return camera;
	}

	// Returns the position of a point in the space of the view of the camera
	glm::vec3 in_view(const Camera& camera, const glm::vec3& point)
	{
		return glm::vec3{ camera.get_view_matrix() * glm::vec4{ point, 1.0f } };
	}

	// Returns the position in pixels of a point in a view of the given size (x to the right and y down, as the mouse)
	glm::vec2 on_screen(const Camera& camera, const glm::vec3& point, GLuint width, GLuint height)
	{
		const glm::vec4 clip{ camera.get_projection_matrix(width, height) * camera.get_view_matrix() * glm::vec4{ point, 1.0f } };
		const glm::vec2 ndc{ glm::vec2{ clip } / clip.w };
		return glm::vec2{ (ndc.x + 1.0f) * 0.5f * static_cast<GLfloat>(width), (1.0f - ndc.y) * 0.5f * static_cast<GLfloat>(height) };
	}
} // namespace

TEST(CameraControllerTest, AnOrbitKeepsThePivotInTheSamePlaceOfTheViewAndAtTheSameDistance)
{
	const glm::vec3 pivot{ 1.0f, 0.5f, -2.0f };
	Camera          camera{ make_camera(glm::vec3{ 4.0f, 3.0f, 6.0f }, pivot) };
	const glm::vec3 before{ in_view(camera, pivot) };

	Camera_Controller::orbit(camera, pivot, 40.0f, -30.0f, Camera_Control_Params{});

	const glm::vec3 after{ in_view(camera, pivot) };
	EXPECT_NEAR(after.x, before.x, 1e-3f);
	EXPECT_NEAR(after.y, before.y, 1e-3f);
	EXPECT_NEAR(after.z, before.z, 1e-3f);
	EXPECT_NE(camera.get_position(), (glm::vec3{ 4.0f, 3.0f, 6.0f })); // the camera did move
}

TEST(CameraControllerTest, AnOrbitToTheRightTurnsTheSceneToTheRight)
{
	Camera camera{ make_camera(glm::vec3{ 0.0f, 0.0f, 10.0f }, glm::vec3{ 0.0f }) };

	Camera_Controller::orbit(camera, glm::vec3{ 0.0f }, 40.0f, 0.0f, Camera_Control_Params{});

	// the point of the scene in front of the pivot (towards the camera) moves to the right of the view
	EXPECT_GT(in_view(camera, glm::vec3{ 0.0f, 0.0f, 1.0f }).x, 0.0f);
}

TEST(CameraControllerTest, AnOrbitDownRaisesTheCameraAndMakesItLookDown)
{
	Camera camera{ make_camera(glm::vec3{ 0.0f, 0.0f, 10.0f }, glm::vec3{ 0.0f }) };

	Camera_Controller::orbit(camera, glm::vec3{ 0.0f }, 0.0f, 40.0f, Camera_Control_Params{});

	EXPECT_GT(camera.get_position().y, 0.0f);
	EXPECT_LT(camera.get_front().y, 0.0f);
	EXPECT_NEAR(glm::length(camera.get_position()), 10.0f, 1e-3f);
}

TEST(CameraControllerTest, TheOrbitStopsAtThePitchLimitOfTheCameraAndStaysRigid)
{
	const glm::vec3 pivot{ 0.0f };
	Camera          camera{ make_camera(glm::vec3{ 0.0f, 0.0f, 10.0f }, pivot) };
	const glm::vec3 before{ in_view(camera, pivot) };

	Camera_Controller::orbit(camera, pivot, 0.0f, 10000.0f, Camera_Control_Params{}); // far more than 90 degrees

	EXPECT_NEAR(camera.get_pitch(), -89.0f, 1e-3f);
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 10.0f, 1e-3f);
	const glm::vec3 after{ in_view(camera, pivot) };
	EXPECT_NEAR(after.x, before.x, 1e-3f);
	EXPECT_NEAR(after.y, before.y, 1e-3f);
}

TEST(CameraControllerTest, APanMovesAPointAtTheDepthOfThePivotWithTheMouse)
{
	const glm::vec3 pivot{ 0.5f, 1.0f, -1.0f };
	Camera          camera{ make_camera(glm::vec3{ 3.0f, 4.0f, 8.0f }, pivot) };
	const glm::vec2 before{ on_screen(camera, pivot, 1600u, 1000u) };

	Camera_Controller::pan(camera, pivot, 30.0f, 20.0f, 1000u);

	const glm::vec2 after{ on_screen(camera, pivot, 1600u, 1000u) };
	EXPECT_NEAR(after.x - before.x, 30.0f, 1e-2f);
	EXPECT_NEAR(after.y - before.y, 20.0f, 1e-2f);
}

TEST(CameraControllerTest, ADollyMultipliesTheDistanceToThePivotByItsFactor)
{
	const glm::vec3 pivot{ 0.0f };
	Camera          camera{ make_camera(glm::vec3{ 0.0f, 0.0f, 10.0f }, pivot) };
	const GLfloat   zoom{ camera.get_zoom() };

	Camera_Controller::dolly(camera, pivot, 1.0f, Camera_Control_Params{});
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 8.5f, 1e-4f);

	Camera_Controller::dolly(camera, pivot, -1.0f, Camera_Control_Params{});
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 10.0f, 1e-4f);
	EXPECT_EQ(camera.get_zoom(), zoom); // the field of view does not change
}

TEST(CameraControllerTest, TheDollyStopsAtLimitsThatFollowTheClipPlanes)
{
	const glm::vec3 pivot{ 0.0f };
	Camera          camera{ make_camera(glm::vec3{ 0.0f, 0.0f, 10.0f }, pivot) };

	// the defaults: 5 times the near plane (0.1) and half the far plane (100)
	Camera_Controller::dolly(camera, pivot, 100.0f, Camera_Control_Params{});
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 0.5f, 1e-4f);
	Camera_Controller::dolly(camera, pivot, -100.0f, Camera_Control_Params{});
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 50.0f, 1e-3f);

	// with planes for a scene a hundred times larger, the limits are a hundred times larger
	ASSERT_TRUE(camera.set_clip_planes(10.0f, 10000.0f));
	Camera_Controller::dolly(camera, pivot, -100.0f, Camera_Control_Params{});
	EXPECT_NEAR(glm::length(camera.get_position() - pivot), 5000.0f, 0.5f);
}
