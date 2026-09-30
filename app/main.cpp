/*
 * main.cpp
 * This file is is an entry point to the App module. It serves as a simple test
 * of the Core engine. It follows these steps:
 * - It initializes the Core engine, which sets up OpenGL, window, and GUI, and
 * compiles the built-in shaders.
 * - It sets up the initial scene.
 * - It runs the main loop of the engine, which renders the scene and handles
 * events.
 * - It cleans up resources in the correct order and shuts down the engine.
 * Other important notes:
 * - GLFW_Renderer is used as the renderer implementation for the Core engine.
 * - The application uses the Core library to manage nodes, input, and scene
 * management.
 */

#include <filesystem>
#include <memory>
#include <sstream>
#include <utility>

#include "CUBE.h"
#include "PLANE.h"
#include "gui/Gui.h"

#include "core/Core.h"
#include "core/Node.h"
#include "core/camera/Camera.h"
#include "core/light/Directional_Light.h"
#include "core/light/Light.h"
#include "core/light/Point_Light.h"
#include "core/light/Spotlight.h"
#include "core/managers/Node_Manager.h"
#include "core/managers/Scene_Manager.h"
#include "core/model/Assimp_Model.h"
#include "core/model/Model.h"
#include "core/model/Shape_Model.h"
#include "core/utils/string/String_Utils.h"

#include "platform/renderer/GLFW_Renderer.h"

// Initializes the Core engine (which also compiles its built-in shaders) and
// sets up the renderer. Returns true if initialization was successful, false
// otherwise (with error messages)
bool initialize_core(Core* engine);

// Sets up an example scene with a camera, lights, a shape, an Assimp model, and
// a skybox
void setup_example_scene(Core* engine);

// Sets up a test scene (alternative to setup_example_scene) with certain
// elements to test specific functionalities of the engine
void setup_test_scene_reflective_1(Core* engine);
void setup_test_scene_reflective_2(Core* engine);
void setup_test_scene_refractive_1(Core* engine);
void setup_test_scene_refractive_2(Core* engine);
void setup_test_scene_geometric_stress_1(Core* engine, int num_shapes = 1000);
void setup_test_scene_geometric_stress_2(Core* engine, int num_shapes = 1000);
void setup_test_scene_reflective_stress(Core* engine, int num_shapes = 100);

int main(int argc, char** argv)
{
	std::cout << "[INFO::main] Starting the application..." << std::endl;

	Core* engine = Core::get_instance(); // retrieve the singleton instance of
										 // the Core class

	if (!initialize_core(engine))
	{ // if initialization failed, exit with an error code
		return -1;
	}

	// setup the initial scene (uncomment one of the following lines to choose a
	// scene, although in some cases the Renderer has to be modified in order to
	// support the materials)
	setup_example_scene(engine);
	// setup_test_scene_reflective_1(engine);
	// setup_test_scene_reflective_2(engine);
	// setup_test_scene_refractive_1(engine);
	// setup_test_scene_refractive_2(engine);
	// setup_test_scene_geometric_stress_1(engine);
	// setup_test_scene_geometric_stress_2(engine);
	// setup_test_scene_reflective_stress(engine);

	engine->run(); // run the main loop of the engine, which will render the
				   // scene and handle events

	engine->shutdown(); // clean up resources in the correct order and shut down
						// the engine

	std::cout << "[SUCCESS::main] Application finished successfully" << std::endl;

	return 0;
}

bool initialize_core(Core* engine)
{
	// create a GLFW_Renderer instance with the application's GUI, and set it as the renderer for the engine
	GLFW_Renderer* renderer = new GLFW_Renderer();
	renderer->set_gui_layer(std::make_unique<GUI>());
	engine->set_renderer(renderer);

	if (engine->init()) // initialize the engine (OpenGL, window, GUI, etc.)
	{                   // if the initialization is successful, print a success message
		std::cout << "[SUCCESS::main] Core initialized successfully" << std::endl;
	}
	else
	{
		// if the initialization fails, print an error messages and shut down
		// the engine, then prompt the user to exit
		std::cerr << "[ERROR::main] Failed to initialize the Core engine" << std::endl;
		engine->shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return false; // indicate that the program should exit with an error
					  // code
	}

	return true; // indicate that the initialization was successful
}

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
			model_file_path,                    // model file path
			glm::vec4{ 0.8, 0.8f, 0.8f, 0.5f }, // albedo (overriden)
			glm::vec3{ 0.0f, 0.0f, 3.0f },      // position (overridden - offset from root)
			glm::quat(glm::vec3{ 0.0f }),       // rotation (default)
			glm::vec3{ 0.25f }                  // scale (overridden)
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
	// register the reflective plane for dynamic environment map capture in the
	// renderer with a specified resolution (e.g., 512x512)
	engine->get_renderer()->register_model_for_dynamic_env_map_capture(reflective_plane->get_id(), 521);
	// add the reflective plane to the node manager
	node_manager->add_node(reflective_plane);

	// create an Assimp model and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                     // albedo (default)
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
	// register the reflective plane for dynamic environment map capture in the
	// renderer with a specified resolution (e.g., 512x512)
	engine->get_renderer()->register_model_for_dynamic_env_map_capture(reflective_plane->get_id(), 521);
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
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                     // albedo (default)
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
		// register the Assimp model for dynamic environment map capture in the
		// renderer with a specified resolution (e.g., 512x512)
		engine->get_renderer()->register_model_for_dynamic_env_map_capture(model->get_id(), 521);
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
	// register the refractive cube for dynamic environment map capture in the
	// renderer with a specified resolution (e.g., 512x512)
	engine->get_renderer()->register_model_for_dynamic_env_map_capture(refractive_cube->get_id(), 521);
	// add the refractive cube to the node manager
	node_manager->add_node(refractive_cube);

	// create an Assimp model and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                     // albedo (default)
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
	// register the refractive cube for dynamic environment map capture in the
	// renderer with a specified resolution (e.g., 512x512)
	engine->get_renderer()->register_model_for_dynamic_env_map_capture(refractive_cube->get_id(), 521);
	// add the refractive cube to the node manager
	node_manager->add_node(refractive_cube);

	// create a refractive Assimp model, register it for dynamic environment map
	// capture, and add it to the node manager
	std::string model_file_path = engine->get_resource_path("models/obj/teapot/teapot.obj");
	if (std::filesystem::exists(model_file_path))
	{ // if the file exists, create the model and add
	  // it to the node manager
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path,                                         // model file path
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f },                     // albedo (default)
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
		// register the Assimp model for dynamic environment map capture in the
		// renderer
		engine->get_renderer()->register_model_for_dynamic_env_map_capture(model->get_id(), 521);
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

void setup_test_scene_geometric_stress_1(Core* engine, const int num_shapes)
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
				static_cast<float>(rand()) / RAND_MAX, // random red component
				static_cast<float>(rand()) / RAND_MAX, // random green component
				static_cast<float>(rand()) / RAND_MAX, // random blue component
				1.0f                                   // uniform alpha component (fully opaque)
			},
			glm::vec3{
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random x position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random y position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random x rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random y rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f  // random z rotation
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
				static_cast<float>(rand()) / RAND_MAX, // random red component
				static_cast<float>(rand()) / RAND_MAX, // random green component
				static_cast<float>(rand()) / RAND_MAX, // random blue component
				static_cast<float>(rand()) / RAND_MAX  // random alpha component
			},
			glm::vec3{
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random x position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random y position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random x rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random y rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f  // random z rotation
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
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random x position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f, // random y position
				static_cast<float>(rand()) / RAND_MAX * 30.0f - 10.0f  // random z position
			},
			glm::quat(
				glm::radians(
					glm::vec3{
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random x rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f, // random y rotation
						static_cast<float>(rand()) / RAND_MAX * 360.0f  // random z rotation
					})),
			glm::vec3{ 0.5f } // uniform scale
		);
		// convert the reflective shape model's id to string and set it as part
		// of the model's name
		reflective_shape_model->set_name(String_Utils::generate_id_prefixed_name(reflective_shape_model));
		// register the reflective shape model for dynamic environment map
		// capture in the renderer
		engine->get_renderer()->register_model_for_dynamic_env_map_capture(reflective_shape_model->get_id(), 521);
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
