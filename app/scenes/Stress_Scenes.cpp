/*
 * Stress_Scenes.cpp
 * This file implements the stress test scenes of the App, with many randomly placed shapes (opaque,
 * transparent, or reflective) to measure the performance of the renderer.
 */

#include "Scenes.h"

#include "../CUBE.h"
#include "../PLANE.h"

#include "core/Core.h"
#include "core/Node.h"
#include "core/camera/Camera.h"
#include "core/light/Directional_Light.h"
#include "core/light/Light.h"
#include "core/light/Point_Light.h"
#include "core/light/Spotlight.h"
#include "core/managers/Node_Manager.h"
#include "core/managers/Scene_Manager.h"
#include "core/material/Material_Library.h"
#include "core/model/Assimp_Model.h"
#include "core/model/Model.h"
#include "core/model/Shape_Model.h"
#include "core/utils/random/Random.h"
#include "core/utils/string/String_Utils.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

void setup_test_scene_geometric_stress_1(Core* engine, const int num_shapes)
{
	Random random{}; // random number generator for the colors, positions, and rotations of the shapes

	// retrieve the node manager and scene manager from the engine
	auto& node_manager  = engine->get_node_manager();
	auto& scene_manager = engine->get_scene_manager();

	// create a camera with a placeholder name and default parameters
	auto camera = std::make_shared<Camera>("Camera");

	// convert the camera's id to string and set it as part of the camera's name
	camera->set_name(String_Utils::generate_id_prefixed_name(camera));

	// set the camera as the active camera in the scene manager and add it to
	// the node manager
	scene_manager->set_camera(camera);
	node_manager->add_node(std::move(camera));

	// create a directional light and add it along with its gizmo to the node
	// manager
	auto directional_light = std::make_shared<Directional_Light>(
		"Directional Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 1.0f, 1.0f, 0.7f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (override required although it is the
											 // default)
		glm::vec3{ -3.0f, 8.0f, 2.4f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// add a large number of shape models to the scene (with random RGB albedos,
	// positions, and rotations) to stress test the engine's rendering
	// capabilities
	for (int i{}; i < num_shapes; ++i)
	{
		auto shape_model = std::make_shared<Shape_Model>(
			"Shape Model " + std::to_string(i),
			cube_vertices_vector,
			cube_indices_vector,
			glm::vec4{
				random.generate_random_float(0.0f, 1.0f), // random red component
				random.generate_random_float(0.0f, 1.0f), // random green component
				random.generate_random_float(0.0f, 1.0f), // random blue component
				1.0f                                      // uniform alpha component (fully opaque)
			},
			glm::vec3{
				random.generate_random_float(-10.0f, 20.0f), // random x position
				random.generate_random_float(-10.0f, 20.0f), // random y position
				random.generate_random_float(-10.0f, 20.0f)  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						random.generate_random_float(0.0f, 360.0f), // random x rotation
						random.generate_random_float(0.0f, 360.0f), // random y rotation
						random.generate_random_float(0.0f, 360.0f)  // random z rotation
					})),
			glm::vec3{ 0.5f } // uniform scale
		);
		// convert the shape model's id to string and set it as part of the
		// shape model's name
		shape_model->set_name(String_Utils::generate_id_prefixed_name(shape_model));
		node_manager->add_node(shape_model);
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/puresky_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_geometric_stress_1] "
					 "Skybox file not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_geometric_stress_1] Test "
				 "scene setup complete"
			  << std::endl;
}

void setup_test_scene_geometric_stress_2(Core* engine, const int num_shapes)
{
	Random random{}; // random number generator for the colors, positions, and rotations of the shapes

	// retrieve the node manager and scene manager from the engine
	auto& node_manager  = engine->get_node_manager();
	auto& scene_manager = engine->get_scene_manager();

	// create a camera with a placeholder name and default parameters
	auto camera = std::make_shared<Camera>("Camera");

	// convert the camera's id to string and set it as part of the camera's name
	camera->set_name(String_Utils::generate_id_prefixed_name(camera));

	// set the camera as the active camera in the scene manager and add it to
	// the node manager
	scene_manager->set_camera(camera);
	node_manager->add_node(std::move(camera));

	// create a directional light and add it along with its gizmo to the node
	// manager
	auto directional_light = std::make_shared<Directional_Light>(
		"Directional Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 1.0f, 1.0f, 0.7f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (override required although it is the
											 // default)
		glm::vec3{ -3.0f, 8.0f, 2.4f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// add a large number of shape models to the scene (with random RGBA
	// albedos, positions, and rotations) to stress test the engine's rendering
	// capabilities
	for (int i{}; i < num_shapes; ++i)
	{
		auto shape_model = std::make_shared<Shape_Model>(
			"Shape Model " + std::to_string(i),
			cube_vertices_vector,
			cube_indices_vector,
			glm::vec4{
				random.generate_random_float(0.0f, 1.0f), // random red component
				random.generate_random_float(0.0f, 1.0f), // random green component
				random.generate_random_float(0.0f, 1.0f), // random blue component
				random.generate_random_float(0.0f, 1.0f)  // random alpha component
			},
			glm::vec3{
				random.generate_random_float(-10.0f, 20.0f), // random x position
				random.generate_random_float(-10.0f, 20.0f), // random y position
				random.generate_random_float(-10.0f, 20.0f)  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						random.generate_random_float(0.0f, 360.0f), // random x rotation
						random.generate_random_float(0.0f, 360.0f), // random y rotation
						random.generate_random_float(0.0f, 360.0f)  // random z rotation
					})),
			glm::vec3{ 0.5f } // uniform scale
		);
		// convert the shape model's id to string and set it as part of the
		// shape model's name
		shape_model->set_name(String_Utils::generate_id_prefixed_name(shape_model));
		node_manager->add_node(shape_model);
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/puresky_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_geometric_stress_2] "
					 "Skybox file not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_geometric_stress_2] Test "
				 "scene setup complete"
			  << std::endl;
}

void setup_test_scene_reflective_stress(Core* engine, const int num_shapes)
{
	Random random{}; // random number generator for the colors, positions, and rotations of the shapes

	// retrieve the node manager and scene manager from the engine
	auto& node_manager  = engine->get_node_manager();
	auto& scene_manager = engine->get_scene_manager();

	// create a camera with a placeholder name and default parameters
	auto camera = std::make_shared<Camera>("Camera");

	// convert the camera's id to string and set it as part of the camera's name
	camera->set_name(String_Utils::generate_id_prefixed_name(camera));

	// set the camera as the active camera in the scene manager and add it to
	// the node manager
	scene_manager->set_camera(camera);
	node_manager->add_node(std::move(camera));

	// create a directional light and add it along with its gizmo to the node
	// manager
	auto directional_light = std::make_shared<Directional_Light>(
		"Directional Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 1.0f, 1.0f, 0.7f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (override required although it is the
											 // default)
		glm::vec3{ -3.0f, 8.0f, 2.4f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// add a large number of reflective shape models to the scene (with
	// positions and rotations) to stress test the engine's rendering
	// capabilities
	for (int i{}; i < num_shapes; ++i)
	{
		auto reflective_shape_model = std::make_shared<Shape_Model>(
			"Reflective Shape Model " + std::to_string(i),
			cube_vertices_vector,
			cube_indices_vector,
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f }, // albedo (default)
			glm::vec3{
				random.generate_random_float(-10.0f, 20.0f), // random x position
				random.generate_random_float(-10.0f, 20.0f), // random y position
				random.generate_random_float(-10.0f, 20.0f)  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						random.generate_random_float(0.0f, 360.0f), // random x rotation
						random.generate_random_float(0.0f, 360.0f), // random y rotation
						random.generate_random_float(0.0f, 360.0f)  // random z rotation
					})),
			glm::vec3{ 0.5f } // uniform scale
		);
		// convert the reflective shape model's id to string and set it as part
		// of the model's name
		reflective_shape_model->set_name(String_Utils::generate_id_prefixed_name(reflective_shape_model));
		// give the reflective shape model a reflective material, which captures its environment map every frame
		reflective_shape_model->set_material(engine->get_material_library()->get("Mirror"));
		// add the reflective shape model to the node manager
		node_manager->add_node(reflective_shape_model);
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/puresky_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_reflective_stress] Skybox "
					 "file not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_reflective_stress] Test "
				 "scene setup complete"
			  << std::endl;
}
