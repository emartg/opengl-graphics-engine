/*
 * Reflection_Scenes.cpp
 * This file implements the test scenes of the App for reflective and refractive materials, which capture
 * the dynamic environment maps of their objects.
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

void setup_test_scene_reflective_1(Core* engine)
{
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
		glm::vec3{ 2.4f, 8.0f, -3.0f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	directional_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// create a point light and add it along with its gizmo to the node manager
	auto point_light = std::make_shared<Point_Light>(
		"Point Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 0.3f, 0.9f, 1.0f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f }       // position (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	point_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto point_light_gizmo = point_light->get_gizmo();
	if (point_light_gizmo)
		node_manager->add_node(std::move(point_light_gizmo));
	node_manager->add_node(std::move(point_light));

	// create a reflective plane model, register it for dynamic environment map
	// capture, and add it to the node manager
	auto reflective_plane = std::make_shared<Shape_Model>(
		"Reflective Plane",
		plane_vertices_vector,
		plane_indices_vector,
		glm::vec4{ 1.0f },                                       // albedo (overridden - white to ensure full reflection)
		glm::vec3{ 0.0f, -0.5f, 0.0f },                          // position (overridden)
		glm::quat(glm::radians(glm::vec3{ 0.0f, 45.0f, 0.0f })), // rotation (overridden)
		glm::vec3{ 5.8f }                                        // scale (overridden)
	);
	// set the plane to be two-sided for reflection
	reflective_plane->set_is_two_sided(true);
	// convert the plane's id to string and set it as part of the plane's name
	reflective_plane->set_name(String_Utils::generate_id_prefixed_name(reflective_plane));
	// give the reflective plane a reflective material, which captures its environment map every frame
	reflective_plane->set_material(engine->get_material_library()->get("Mirror"));
	// add the reflective plane to the node manager
	node_manager->add_node(reflective_plane);

	// create an Assimp model and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/Teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },                     // albedo (default)
			glm::vec3{ -3.25f, 0.95f, -3.25f },                      // position (overridden)
			glm::quat(glm::radians(glm::vec3{ 0.0f, 85.0f, 0.0f })), // rotation (overridden)
			glm::vec3{ 0.2f }                                        // scale (overridden)
		);
		// remove the path and the extension from the file path for the model's
		// name
		std::string model_name = model_file_path.substr(
			model_file_path.find_last_of("/\\") + 1,
			model_file_path.find_last_of('.') - model_file_path.find_last_of("/\\") - 1);
		// uppercase the first letter of the model's name
		model_name[0] = std::toupper(model_name[0]);
		// convert the model's id to string and set it as part of the model's
		// name
		model->set_name(String_Utils::generate_id_prefixed_name(model));
		// add the model to the node manager
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_test_scene_reflective_1] Assimp "
					 "model not found: "
				  << model_file_path << std::endl;
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/canary_wharf_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_reflective_1] Skybox file "
					 "not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_reflective_1] Test scene "
				 "setup complete"
			  << std::endl;
}

void setup_test_scene_reflective_2(Core* engine)
{
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
		glm::vec3{ 2.4f, 8.0f, -3.0f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	directional_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// create a point light and add it along with its gizmo to the node manager
	auto point_light = std::make_shared<Point_Light>(
		"Point Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 0.3f, 0.9f, 1.0f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f }       // position (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	point_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto point_light_gizmo = point_light->get_gizmo();
	if (point_light_gizmo)
		node_manager->add_node(std::move(point_light_gizmo));
	node_manager->add_node(std::move(point_light));

	// create a reflective plane model, register it for dynamic environment map
	// capture, and add it to the node manager
	auto reflective_plane = std::make_shared<Shape_Model>(
		"Reflective Plane",
		plane_vertices_vector,
		plane_indices_vector,
		glm::vec4{ 1.0f },                                       // albedo (overridden - white to ensure full reflection)
		glm::vec3{ 0.0f, -0.5f, 0.0f },                          // position (overridden)
		glm::quat(glm::radians(glm::vec3{ 0.0f, 45.0f, 0.0f })), // rotation (overridden)
		glm::vec3{ 5.8f }                                        // scale (overridden)
	);
	// set the plane to be two-sided for reflection
	reflective_plane->set_is_two_sided(true);
	// convert the plane's id to string and set it as part of the plane's name
	reflective_plane->set_name(String_Utils::generate_id_prefixed_name(reflective_plane));
	// give the reflective plane a reflective material, which captures its environment map every frame
	reflective_plane->set_material(engine->get_material_library()->get("Mirror"));
	// add the reflective plane to the node manager
	node_manager->add_node(reflective_plane);

	// create a reflective Assimp model, register it for dynamic environment map
	// capture, and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/gltf/teapot/teapot.gltf");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Reflective Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },                     // albedo (default)
			glm::vec3{ -3.25f, 0.95f, -3.25f },                      // position (overridden)
			glm::quat(glm::radians(glm::vec3{ 0.0f, 85.0f, 0.0f })), // rotation (overridden)
			glm::vec3{ 0.2f }                                        // scale (overridden)
		);
		// remove the path and the extension from the file path for the model's
		// name
		std::string model_name = model_file_path.substr(
			model_file_path.find_last_of("/\\") + 1,
			model_file_path.find_last_of('.') - model_file_path.find_last_of("/\\") - 1);
		// uppercase the first letter of the model's name
		model_name[0] = std::toupper(model_name[0]);
		// convert the model's id to string and set it as part of the model's
		// name
		model->set_name(String_Utils::generate_id_prefixed_name(model));
		// give the Assimp model a reflective material, which captures its environment map every frame
		model->set_material(engine->get_material_library()->get("Mirror"));
		// add the model to the node manager
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_test_scene_reflective_2] Assimp "
					 "model not found: "
				  << model_file_path << std::endl;
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/canary_wharf_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_reflective_2] Skybox file "
					 "not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_reflective_2] Test scene "
				 "setup complete"
			  << std::endl;
}

void setup_test_scene_refractive_1(Core* engine)
{
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
		glm::vec3{ 2.4f, 8.0f, -3.0f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	directional_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto directional_light_gizmo = directional_light->get_gizmo();
	directional_light_gizmo->set_is_visible(false); // hide the gizmo to avoid clutter in the scene
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// create a point light and add it along with its gizmo to the node manager
	auto point_light = std::make_shared<Point_Light>(
		"Point Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 0.3f, 0.9f, 1.0f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f }       // position (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	point_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto point_light_gizmo = point_light->get_gizmo();
	point_light_gizmo->set_is_visible(false); // hide the gizmo to avoid clutter in the scene
	if (point_light_gizmo)
		node_manager->add_node(std::move(point_light_gizmo));
	node_manager->add_node(std::move(point_light));

	// create a refractive cube model, register it for dynamic environment map
	// capture, and add it to the node manager
	auto refractive_cube = std::make_shared<Shape_Model>(
		"Refractive Cube",
		cube_vertices_vector,
		cube_indices_vector,
		glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                      // albedo (default)
		glm::vec3{ 0.15f, 1.5f, 2.5f },                           // position (overridden)
		glm::quat(glm::radians(glm::vec3{ -90.0f, 0.0f, 0.0f })), // rotation (overridden)
		glm::vec3{ 5.0f, 0.1f, 5.0f }                             // scale (overridden)
	);
	// convert the cube's id to string and set it as part of the cube's name
	refractive_cube->set_name(String_Utils::generate_id_prefixed_name(refractive_cube));
	// give the refractive cube a refractive material, which captures its environment map every frame
	refractive_cube->set_material(engine->get_material_library()->get("Dynamic Glass"));
	// add the refractive cube to the node manager
	node_manager->add_node(refractive_cube);

	// create an Assimp model and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/Teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },                     // albedo (default)
			glm::vec3{ -2.75f, -1.95f, -5.75f },                     // position (overridden)
			glm::quat(glm::radians(glm::vec3{ 0.0f, 85.0f, 0.0f })), // rotation (overridden)
			glm::vec3{ 0.3f }                                        // scale (overridden)
		);
		// remove the path and the extension from the file path for the model's
		// name
		std::string model_name = model_file_path.substr(
			model_file_path.find_last_of("/\\") + 1,
			model_file_path.find_last_of('.') - model_file_path.find_last_of("/\\") - 1);
		// uppercase the first letter of the model's name
		model_name[0] = std::toupper(model_name[0]);
		// convert the model's id to string and set it as part of the model's
		// name
		model->set_name(String_Utils::generate_id_prefixed_name(model));
		// add the model to the node manager
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_test_scene_refractive_1] Assimp "
					 "model not found: "
				  << model_file_path << std::endl;
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/golden_gate_hills_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_refractive_1] Skybox file "
					 "not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_refractive_1] Test scene "
				 "setup complete"
			  << std::endl;
}

void setup_test_scene_refractive_2(Core* engine)
{
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
		glm::vec3{ 2.4f, 8.0f, -3.0f },      // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f }       // direction (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	directional_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto directional_light_gizmo = directional_light->get_gizmo();
	directional_light_gizmo->set_is_visible(false); // hide the gizmo to avoid clutter in the scene
	if (directional_light_gizmo)
		node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// create a point light and add it along with its gizmo to the node manager
	auto point_light = std::make_shared<Point_Light>(
		"Point Light",
		glm::vec3{ 0.1f },                   // ambient color (default)
		glm::vec4{ 0.3f, 0.9f, 1.0f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f }       // position (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	point_light->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto point_light_gizmo = point_light->get_gizmo();
	point_light_gizmo->set_is_visible(false); // hide the gizmo to avoid clutter in the scene
	if (point_light_gizmo)
		node_manager->add_node(std::move(point_light_gizmo));
	node_manager->add_node(std::move(point_light));

	// create a refractive cube model, register it for dynamic environment map
	// capture, and add it to the node manager
	auto refractive_cube = std::make_shared<Shape_Model>(
		"Refractive Cube",
		cube_vertices_vector,
		cube_indices_vector,
		glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                          // albedo (default)
		glm::vec3{ -8.4f, 1.5f, -3.95f },                             // position (overridden)
		glm::quat(glm::radians(glm::vec3{ 90.0f, -80.0f, -179.0f })), // rotation (overridden)
		glm::vec3{ 5.0f, 0.1f, 5.0f }                                 // scale (overridden)
	);
	// convert the cube's id to string and set it as part of the cube's name
	refractive_cube->set_name(String_Utils::generate_id_prefixed_name(refractive_cube));
	// give the refractive cube a refractive material, which captures its environment map every frame
	refractive_cube->set_material(engine->get_material_library()->get("Dynamic Glass"));
	// add the refractive cube to the node manager
	node_manager->add_node(refractive_cube);

	// create a refractive Assimp model, register it for dynamic environment map
	// capture, and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/Teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },                     // albedo (default)
			glm::vec3{ -1.4f, -0.6f, -5.6f },                        // position (overridden)
			glm::quat(glm::radians(glm::vec3{ 0.0f, 85.0f, 0.0f })), // rotation (overridden)
			glm::vec3{ 0.6f }                                        // scale (overridden)
		);
		// remove the path and the extension from the file path for the model's
		// name
		std::string model_name = model_file_path.substr(
			model_file_path.find_last_of("/\\") + 1,
			model_file_path.find_last_of('.') - model_file_path.find_last_of("/\\") - 1);
		// uppercase the first letter of the model's name
		model_name[0] = std::toupper(model_name[0]);
		// convert the model's id to string and set it as part of the model's
		// name
		model->set_name(String_Utils::generate_id_prefixed_name(model));
		// give the Assimp model a refractive material, which captures its environment map every frame
		model->set_material(engine->get_material_library()->get("Dynamic Glass"));
		// add the model to the node manager
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_test_scene_refractive_2] Assimp "
					 "model not found: "
				  << model_file_path << std::endl;
	}

	// create a skybox and set it in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/golden_gate_hills_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_test_scene_refractive_2] Skybox file "
					 "not found: "
				  << skybox_file_path << std::endl;
	}

	// print a success message indicating the test scene setup is complete
	std::cout << "[SUCCESS::main::setup_test_scene_refractive_2] Test scene "
				 "setup complete"
			  << std::endl;
}
