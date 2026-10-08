/*
 * Example_Scene.cpp
 * This file implements the example scene of the App: a camera, lights, shapes, an imported model, and a skybox.
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

void setup_example_scene(Core* engine)
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

	// create a spotlight and add it along with its gizmo to the node manager
	auto spotlight = std::make_shared<Spotlight>(
		"Spotlight",
		glm::vec3{ 0.1f },                   // ambient color (override required
											 // although it is the default)
		glm::vec4{ 1.0f, 0.4f, 0.4f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f },                   // specular color (override required although it is the
											 // default)
		glm::vec3{ 3.0f, -0.3f, -0.9f },     // position (overridden)
		glm::vec3{ -0.8f, 0.3f, 0.6f },      // direction (overridden)
		glm::cos(glm::radians(15.0f)),       // inner cut-off (overridden)
		glm::cos(glm::radians(32.5f))        // outer cut-off (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	spotlight->set_name(String_Utils::generate_id_prefixed_name(spotlight));

	// create the gizmo child (the light has been fully constructed and placed
	// in a shared_ptr)
	spotlight->create_gizmo();
	// add the gizmo child to the node manager
	// before moving the light (renderer skips it via parent check)
	auto spotlight_gizmo = spotlight->get_gizmo();
	if (spotlight_gizmo)
		node_manager->add_node(std::move(spotlight_gizmo));
	node_manager->add_node(std::move(spotlight));

	// create a composite model hierarchy mixing shapes and an Assimp model
	auto root_group = std::make_shared<Model>("Root Group");

	// convert the model's id to string and set it as part of the model's name
	root_group->set_name(String_Utils::generate_id_prefixed_name(root_group));

	// create a shapes group to hold multiple shapes as children of the root
	// group
	auto shapes_group = std::make_shared<Model>(
		"Shapes Group",
		Node_Type::COMPOSITE_MODEL,
		glm::vec4{ 1.0f },             // albedo (overridden)
		glm::vec3{ 0.0f, 0.0f, -2.5f } // position (overridden - offset from root)
	);

	// convert the model's id to string and set it as part of the model's name
	shapes_group->set_name(String_Utils::generate_id_prefixed_name(shapes_group));

	// create three shapes with different colors and transformations as children
	// of the shapes group
	auto red_cube = std::make_shared<Shape_Model>(
		"Red Cube Shape",
		cube_vertices_vector,
		cube_indices_vector,
		glm::vec4{ 0.9f, 0.2f, 0.2f, 1.0f },                     // albedo (overridden)
		glm::vec3{ -1.0f, 0.0f, 0.0f },                          // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(15.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.8f }                                        // scale (overridden)
	);
	auto blue_cube = std::make_shared<Shape_Model>(
		"Blue Cube Shape",
		cube_vertices_vector,
		cube_indices_vector,
		glm::vec4{ 0.2f, 0.2f, 0.9f, 1.0f },                      // albedo (overridden)
		glm::vec3{ 1.0f, 0.0f, 0.0f },                            // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(-25.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.6f }                                         // scale (overridden)
	);
	auto green_plane = std::make_shared<Shape_Model>(
		"Green Plane",
		plane_vertices_vector,
		plane_indices_vector,
		glm::vec4{ 0.3f, 0.9f, 0.3f, 0.5f },                      // albedo (overridden - alpha below 1 for transparency)
		glm::vec3{ 0.0f, 0.75f, 0.0f },                           // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ glm::radians(-90.0f), 0.0f, 0.0f }), // rotation (overridden - vertical plane)
		glm::vec3{ 2.5f }                                         // scale (overridden)
	);
	green_plane->set_is_two_sided(true); // set the plane to be two-sided for transparency

	// convert the red cube's id to string and set it as part of the shape's
	// name
	red_cube->set_name(String_Utils::generate_id_prefixed_name(red_cube));

	// convert the blue cube's id to string and set it as part of the shape's
	// name
	blue_cube->set_name(String_Utils::generate_id_prefixed_name(blue_cube));

	// convert the green plane's id to string and set it as part of the shape's
	// name
	green_plane->set_name(String_Utils::generate_id_prefixed_name(green_plane));

	// attach shapes under the shapes group
	shapes_group->add_child(red_cube);
	shapes_group->add_child(blue_cube);
	shapes_group->add_child(green_plane);

	// attach the shapes group under the root group
	root_group->add_child(shapes_group);

	// create an Assimp model as a child of the root group and add it to the
	// node manager
	std::string model_file_path = engine->get_resource_path("models/gltf/teapot/teapot.gltf");
	if (std::filesystem::exists(model_file_path))
	{ // check if the file exists before loading it
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                     // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 0.5f }, // albedo (overriden)
			glm::vec3{ 0.0f, 0.0f, 3.0f },       // position (overridden - offset from root)
			glm::quat(glm::vec3{ 0.0f }),        // rotation (default)
			glm::vec3{ 0.25f }                   // scale (overridden)
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
		// attach Assimp model under the root group
		root_group->add_child(model);
		// add the Assimp model to the node manager
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_example_scene] Assimp model not found: " << model_file_path << std::endl;
	}

	// add all unregistered selectable nodes in the hierarchy
	// to the node manager to register them for selection/picking
	node_manager->add_node(root_group);
	node_manager->add_node(shapes_group);
	node_manager->add_node(red_cube);
	node_manager->add_node(blue_cube);
	node_manager->add_node(green_plane);

	// load an HDR skybox texture and set it as the skybox in the scene manager
	std::string skybox_file_path = engine->get_resource_path("textures/skyboxes/hdr/tiergarten_4k.hdr");
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_example_scene] Skybox file not found: " << skybox_file_path << std::endl;
	}

	// print a success message indicating the example scene setup is complete
	std::cout << "[SUCCESS::main::setup_example_scene] Example scene setup "
				 "completed successfully"
			  << std::endl;
}
