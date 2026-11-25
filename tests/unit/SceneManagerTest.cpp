/*
 * SceneManagerTest.cpp
 * This file contains the unit tests of the unit of a scene in the Scene_Manager class: the number of world units in a metre (1 by
 * default), the conversions of lengths, its validation, and how it reaches the camera of the scene (its defaults change, but the
 * camera is never moved, and a camera set afterwards keeps its own values).
 */

#include <limits>
#include <memory>

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include "core/camera/Camera.h"
#include "core/managers/Scene_Manager.h"

TEST(SceneManagerTest, AScenesUnitIsTheMetreByDefaultAndConvertsLengths)
{
	Scene_Manager scene_manager;
	EXPECT_FLOAT_EQ(scene_manager.get_units_per_metre(), 1.0f);

	EXPECT_TRUE(scene_manager.set_units_per_metre(100.0f)); // a scene in centimetres
	EXPECT_FLOAT_EQ(scene_manager.to_scene_units(5.0f), 500.0f);
	EXPECT_FLOAT_EQ(scene_manager.to_metres(250.0f), 2.5f);
}

TEST(SceneManagerTest, TheUnitReachesTheCameraOfTheSceneWithoutMovingIt)
{
	Scene_Manager scene_manager;
	auto          camera = std::make_shared<Camera>("Camera");
	camera->set_position(glm::vec3{ 0.0f, 30.0f, 80.0f }); // placed by the application, in centimetres
	scene_manager.set_camera(camera);

	EXPECT_TRUE(scene_manager.set_units_per_metre(100.0f));
	EXPECT_FLOAT_EQ(camera->get_units_per_metre(), 100.0f);
	EXPECT_FLOAT_EQ(camera->get_near_plane(), 10.0f);
	EXPECT_FLOAT_EQ(camera->get_far_plane(), 10000.0f);
	EXPECT_EQ(camera->get_position(), (glm::vec3{ 0.0f, 30.0f, 80.0f }));
}

TEST(SceneManagerTest, ACameraSetAfterTheUnitKeepsItsOwnValues)
{
	Scene_Manager scene_manager;
	EXPECT_TRUE(scene_manager.set_units_per_metre(100.0f));

	auto camera = std::make_shared<Camera>("Camera");
	scene_manager.set_camera(camera);
	EXPECT_FLOAT_EQ(camera->get_near_plane(), Camera::NEAR_PLANE);

	// the application gives it the unit of the scene explicitly
	EXPECT_TRUE(camera->apply_units(scene_manager.get_units_per_metre()));
	EXPECT_FLOAT_EQ(camera->get_near_plane(), 10.0f);
}

TEST(SceneManagerTest, AnInvalidUnitIsRefusedAndKeepsTheCurrentOne)
{
	Scene_Manager scene_manager;
	auto          camera = std::make_shared<Camera>("Camera");
	scene_manager.set_camera(camera);
	EXPECT_TRUE(scene_manager.set_units_per_metre(100.0f));

	for (const float units : { 0.0f, -1.0f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN() })
		EXPECT_FALSE(scene_manager.set_units_per_metre(units));
	EXPECT_FLOAT_EQ(scene_manager.get_units_per_metre(), 100.0f);
	EXPECT_FLOAT_EQ(camera->get_units_per_metre(), 100.0f);
}
