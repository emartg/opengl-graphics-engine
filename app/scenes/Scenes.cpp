/*
 * Scenes.cpp
 * This file implements the registry of the scenes of the App (an example scene and several test scenes), which
 * names them, so that the scene to load is chosen when the App starts (see the --scene command-line option)
 * instead of by editing the code.
 */

#include "Scenes.h"

#include <array>

namespace
{
	// Scenes of the App, with the default scene first. The scenes without a number of objects ignore it
	constexpr std::array<Scene_Info, 8> SCENES{ {
		{ "example", "Camera, lights, shapes, an imported model, and a skybox", 0, [](Core* engine, int) { setup_example_scene(engine); } },
		{ "reflective-1",
		  "Mirror plane reflecting the scene (dynamic environment map)",
		  0,
		  [](Core* engine, int) { setup_test_scene_reflective_1(engine); } },
		{ "reflective-2",
		  "Mirror plane and mirror model reflecting each other",
		  0,
		  [](Core* engine, int) { setup_test_scene_reflective_2(engine); } },
		{ "refractive-1",
		  "Glass cube refracting the scene (dynamic environment map)",
		  0,
		  [](Core* engine, int) { setup_test_scene_refractive_1(engine); } },
		{ "refractive-2",
		  "Glass cube and glass model refracting each other",
		  0,
		  [](Core* engine, int) { setup_test_scene_refractive_2(engine); } },
		{ "geometric-stress-1",
		  "Many opaque shapes (frustum culling and instancing)",
		  1000,
		  [](Core* engine, int object_count) { setup_test_scene_geometric_stress_1(engine, object_count); } },
		{ "geometric-stress-2",
		  "Many opaque and transparent shapes (blending)",
		  1000,
		  [](Core* engine, int object_count) { setup_test_scene_geometric_stress_2(engine, object_count); } },
		{ "reflective-stress",
		  "Many mirror shapes reflecting each other (dynamic environment maps)",
		  100,
		  [](Core* engine, int object_count) { setup_test_scene_reflective_stress(engine, object_count); } },
	} };
}

std::span<const Scene_Info> get_scenes()
{
	return SCENES;
}

const Scene_Info* find_scene(std::string_view name)
{
	for (const auto& scene : SCENES)
		if (scene.name == name)
			return &scene;
	return nullptr;
}
