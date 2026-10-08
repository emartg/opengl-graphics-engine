/*
 * Scenes.h
 * This file declares the scenes of the App (an example scene and several test scenes) and the registry that
 * names them, so that the scene to load is chosen when the App starts (see the --scene command-line option)
 * instead of by editing the code.
 */

#pragma once

#include <span>
#include <string_view>

class Core;

// Scene of the registry: its name (used by the --scene option), a description, the default number of objects of the
// scenes whose number of objects can be chosen (0 for the others), and the function that sets it up
struct Scene_Info
{
	std::string_view name;
	std::string_view description;
	int              default_object_count;
	void (*setup)(Core* engine, int object_count);
};

// Returns the scenes of the registry, with the default scene ("example") first
std::span<const Scene_Info> get_scenes();

// Returns the scene with the given name, or nullptr if there is none
const Scene_Info* find_scene(std::string_view name);

// Sets up an example scene with a camera, lights, shapes, an imported model, and a skybox
void setup_example_scene(Core* engine);

// Set up test scenes for reflective and refractive materials (with dynamic environment maps)
void setup_test_scene_reflective_1(Core* engine);
void setup_test_scene_reflective_2(Core* engine);
void setup_test_scene_refractive_1(Core* engine);
void setup_test_scene_refractive_2(Core* engine);

// Set up stress test scenes with the given number of randomly placed shapes: opaque (1), opaque and transparent (2),
// or reflective, reflecting each other (reflective stress)
void setup_test_scene_geometric_stress_1(Core* engine, int num_shapes);
void setup_test_scene_geometric_stress_2(Core* engine, int num_shapes);
void setup_test_scene_reflective_stress(Core* engine, int num_shapes);
