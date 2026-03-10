/*
* main.cpp
* This file is is an entry point to the App module. It serves as a simple test of the Core engine.
* It follows these steps:
* - It initializes the Core engine, which sets up OpenGL, window, and GUI.
* - It compiles shaders and sets up the initial scene.
* - It runs the main loop of the engine, which renders the scene and handles events.
* - It cleans up resources in the correct order and shuts down the engine.
* Other important notes:
* - GLFW_Renderer is used as the renderer implementation for the Core engine.
* - The application uses the Core library to manage nodes, input, and scene management.
*/

#include <memory>
#include <filesystem>
#include <sstream>
#include <utility>

#include "CUBE.h"
#include "PLANE.h"
#include "renderer/GLFWRenderer.h"

#include "../core/Core.h"
#include "../core/Node.h"
#include "../core/camera/Camera.h"
#include "../core/light/Light.h"
#include "../core/light/Directional_Light.h"
#include "../core/light/Point_Light.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Model.h"
#include "../core/model/Shape_Model.h"
#include "../core/model/Assimp_Model.h"
#include "../core/managers/Scene_Manager.h"
#include "../core/managers/Node_Manager.h"
#include "../core/utils/string/String_Utils.h"

// Defines hardcoded shader names and paths for the initial scene in a tuple
std::tuple<
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>> define_shaders_info();
// Sets up the initial scene with a camera, lights, a shape, an Assimp model, and a skybox
void setup_initial_scene(Core* engine);
// Sets up a simple test scene (alternative to setup_initial_scene) with basic elements to test a
// specific functionality of the engine that is currently being developed
void setup_initial_test_scene(Core* engine);

int main(int argc, char** argv)
{
	std::cout << "[INFO::main] Starting the application..." << std::endl;

	Core* engine = Core::get_instance(); // retrieve the singleton instance of the Core class

	// create a GLFW_Renderer instance and set it as the renderer for the engine
	Renderer* renderer = new GLFW_Renderer();
	engine->set_renderer(renderer);

	if (engine->init()) // initialize the engine (OpenGL, window, GUI, etc.)
	{ // if the initialization is successful, print a success message
		std::cout << "[SUCCESS::main] Core initialized successfully" << std::endl;
	}
	else
	{
		// if the initialization fails, print an error messages and shut down the engine, 
		// then prompt the user to exit
		std::cerr << "[ERROR::main] Failed to initialize OpenGL" << std::endl;
		engine->shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	// get the shader names and paths for the initial scene
	auto [shader_names, vertex_shader_paths, geometry_shader_paths, fragment_shader_paths]
		= define_shaders_info();

	// create, compile, and link the shader programs, and add them to the engine's node manager
	if (engine->compile_shaders(shader_names, vertex_shader_paths, fragment_shader_paths))
	{ // if the shaders are compiled successfully, print a success message
		std::cout << "[SUCCESS::main] Shaders compiled successfully" << std::endl;
	}
	else
	{ // if the shader compilation fails, print an error message and shut down the engine
		std::cerr << "[ERROR::main] Failed to compile shaders" << std::endl;
		engine->shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	setup_initial_scene(engine); // set up the initial scene with a camera, lights, and a model

	engine->run(); // run the main loop of the engine, which will render the scene and handle events

	engine->shutdown(); // clean up resources in the correct order and shut down the engine

	std::cout << "[SUCCESS::main] Application finished successfully" << std::endl;

	return 0;
}

std::tuple<std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>> define_shaders_info()
{
	std::vector<std::string> shader_names = {
		"Shape Model Shader",
		"Assimp Model Shader",
		"Single Albedo Shader",
		"Screen Quad Shader",
		"Picking Shader",
		"Skybox Shader",
		"Equirectangular to Cubemap Shader",
		"Reflective Shader",
		"Refractive Shader"
	};

	std::string shaders_dir = "resources/shaders/";

	std::vector<std::string> vertex_shader_paths = {
		shaders_dir + "shape_model.vert.glsl",
		shaders_dir + "assimp_model.vert.glsl",
		shaders_dir + "single_albedo.vert.glsl",
		shaders_dir + "screen_quad.vert.glsl",
		shaders_dir + "picking.vert.glsl",
		shaders_dir + "skybox.vert.glsl",
		shaders_dir + "equirectangular_to_cubemap.vert.glsl",
		shaders_dir + "reflective.vert.glsl",
		shaders_dir + "refractive.vert.glsl"
	};
	std::vector<std::string> geometry_shader_paths = {
		"", // no custom geometry shader for the shape model shader
		"", // no custom geometry shader for the assimp model shader
		""  // no custom geometry shader for the single albedo shader
		"", // no custom geometry shader for the screen shader
		"", // no custom geometry shader for the picking shader
		"", // no custom geometry shader for the skybox shader
		"", // no custom geometry shader for the equirectangular to cubemap shader
		"", // no custom geometry shader for the reflective shader
		""  // no custom geometry shader for the refractive shader
	};
	std::vector<std::string> fragment_shader_paths = {
		shaders_dir + "shape_model.frag.glsl",
		shaders_dir + "assimp_model.frag.glsl",
		shaders_dir + "single_albedo.frag.glsl",
		shaders_dir + "screen_quad.frag.glsl",
		shaders_dir + "picking.frag.glsl",
		shaders_dir + "skybox.frag.glsl",
		shaders_dir + "equirectangular_to_cubemap.frag.glsl",
		shaders_dir + "reflective.frag.glsl",
		shaders_dir + "refractive.frag.glsl"
	};
	return { shader_names, vertex_shader_paths, geometry_shader_paths, fragment_shader_paths };
}

void setup_initial_scene(Core* engine)
{
	// retrieve the node manager and scene manager from the engine
	auto& node_manager = engine->get_node_manager();
	auto& scene_manager = engine->get_scene_manager();

	// create a camera with a placeholder name and default parameters
	auto camera = std::make_shared<Camera>("Camera");

	// convert the camera's id to string and set it as part of the camera's name
	camera->set_name(String_Utils::generate_id_prefixed_name(camera));

	// set the camera as the active camera in the scene manager and add it to the node manager
	scene_manager->set_camera(camera);
	node_manager->add_node(std::move(camera));

	// create a directional light and add it along with its gizmo to the node manager
	auto directional_light = std::make_shared<Directional_Light>(
		"Directional Light",
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec4{ 1.0f, 1.0f, 0.7f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 2.4f, 8.0f, -3.0f }, // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f } // direction (overridden)
	);

	// convert the light's id to string and set it as part of the light's name
	directional_light->set_name(String_Utils::generate_id_prefixed_name(directional_light));

	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	directional_light->create_gizmo();
	// register the gizmo child for selection/picking before moving the light
	auto directional_light_gizmo = directional_light->get_gizmo();
	if (directional_light_gizmo) node_manager->add_node(std::move(directional_light_gizmo));
	node_manager->add_node(std::move(directional_light));

	// create a point light and add it along with its gizmo to the node manager
	auto point_light = std::make_shared<Point_Light>(
		"Point Light",
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec4{ 0.3f, 0.9f, 1.0f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f } // position (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	point_light->set_name(String_Utils::generate_id_prefixed_name(point_light));

	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	point_light->create_gizmo();
	// register the gizmo child for selection/picking before moving the light
	auto point_light_gizmo = point_light->get_gizmo();
	if (point_light_gizmo) node_manager->add_node(std::move(point_light_gizmo));
	node_manager->add_node(std::move(point_light));

	// create a spotlight and add it along with its gizmo to the node manager
	auto spotlight = std::make_shared<Spotlight>(
		"Spotlight",
		glm::vec3{ 0.1f }, // ambient color (override required although it is the default)
		glm::vec4{ 1.0f, 0.4f, 0.4f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 3.0f, -0.3f, -0.9f }, // position (overridden)
		glm::vec3{ -0.8f, 0.3f, 0.6f }, // direction (overridden)
		glm::cos(glm::radians(15.0f)), // inner cut-off (overridden)
		glm::cos(glm::radians(32.5f)) // outer cut-off (overridden)
	);
	// convert the light's id to string and set it as part of the light's name
	spotlight->set_name(String_Utils::generate_id_prefixed_name(spotlight));

	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	spotlight->create_gizmo();
	// register the gizmo child for selection/picking before moving the light
	auto spotlight_gizmo = spotlight->get_gizmo();
	if (spotlight_gizmo) node_manager->add_node(std::move(spotlight_gizmo));
	node_manager->add_node(std::move(spotlight));

	// create a composite model hierarchy mixing shapes and an Assimp model
	auto root_group = std::make_shared<Model>("Root Group");

	// convert the model's id to string and set it as part of the model's name
	root_group->set_name(String_Utils::generate_id_prefixed_name(root_group));

	// create a shapes group to hold multiple shapes as children of the root group
	auto shapes_group = std::make_shared<Model>(
		"Shapes Group",
		Node_Type::COMPOSITE_MODEL,
		glm::vec4{ 1.0f }, // albedo (overridden)
		glm::vec3{ 0.0f, 0.0f, -2.5f } // position (overridden - offset from root)
	);

	// convert the model's id to string and set it as part of the model's name
	shapes_group->set_name(String_Utils::generate_id_prefixed_name(shapes_group));

	// create three shapes with different colors and transformations as children of the shapes group
	auto red_cube = std::make_shared<Shape_Model>(
		"Red Cube Shape",
		cube_vertices_vector, cube_indices_vector,
		glm::vec4{ 0.9f, 0.2f, 0.2f, 1.0f }, // albedo (overridden)
		glm::vec3{ -1.0f, 0.0f, 0.0f }, // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(15.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.8f } // scale (overridden)
	);
	auto blue_cube = std::make_shared<Shape_Model>(
		"Blue Cube Shape",
		cube_vertices_vector, cube_indices_vector,
		glm::vec4{ 0.2f, 0.2f, 0.9f, 1.0f }, // albedo (overridden)
		glm::vec3{ 1.0f, 0.0f, 0.0f }, // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(-25.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.6f } // scale (overridden)
	);
	auto green_plane = std::make_shared<Shape_Model>(
		"Green Plane",
		plane_vertices_vector, plane_indices_vector,
		glm::vec4{ 0.3f, 0.9f, 0.3f, 0.5f }, // albedo (overridden - alpha below 1 for transparency)
		glm::vec3{ 0.0f, 0.75f, 0.0f }, // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ glm::radians(-90.0f), 0.0f, 0.0f }), // rotation (overridden - vertical plane)
		glm::vec3{ 2.5f } // scale (overridden)
	);
	green_plane->set_is_two_sided(true); // set the plane to be two-sided for transparency

	// convert the red cube's id to string and set it as part of the shape's name
	red_cube->set_name(String_Utils::generate_id_prefixed_name(red_cube));

	// convert the blue cube's id to string and set it as part of the shape's name
	blue_cube->set_name(String_Utils::generate_id_prefixed_name(blue_cube));

	// convert the green plane's id to string and set it as part of the shape's name
	green_plane->set_name(String_Utils::generate_id_prefixed_name(green_plane));

	// attach shapes under the shapes group
	shapes_group->add_child(red_cube);
	shapes_group->add_child(blue_cube);
	shapes_group->add_child(green_plane);

	// attach the shapes group under the root group
	root_group->add_child(shapes_group);

	// create an Assimp model and add it as a child of the root group
	std::string model_file_path = "resources/models/gltf/teapot/teapot.gltf";
	if (std::filesystem::exists(model_file_path))
	{ // check if the file exists before loading it
		auto model = std::make_shared<Assimp_Model>(
			"Teapot",
			model_file_path, // model file path
			glm::vec4{ 1.0f, 1.0f, 1.0f, 0.5f }, // albedo (overridden - alpha below 1 for transparency)
			glm::vec3{ 0.0f, 0.0f, 3.0f }, // position (overridden - offset from root)
			glm::quat(glm::vec3{ 0.0f }), // rotation (default)
			glm::vec3{ 0.25f } // scale (overridden)
		);

		// remove the path and the extension from the file path for the model's name
		std::string model_name = model_file_path.substr(
			model_file_path.find_last_of("/\\") + 1,
			model_file_path.find_last_of('.') - model_file_path.find_last_of("/\\") - 1
		);
		// uppercase the first letter of the model's name
		model_name[0] = std::toupper(model_name[0]);
		// convert the model's id to string and set it as part of the model's name
		model->set_name(String_Utils::generate_id_prefixed_name(model));

		// attach Assimp model under the root group
		root_group->add_child(model);

		// register child nodes for selection/picking (renderer skips them via parent check)
		node_manager->add_node(model);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::setup_initial_scene] Assimp model not found: "
			<< model_file_path << std::endl;
	}

	// register all nodes (including children) so they exist as nodes
	node_manager->add_node(root_group);
	node_manager->add_node(shapes_group);
	node_manager->add_node(red_cube);
	node_manager->add_node(blue_cube);
	node_manager->add_node(green_plane);

	// load an HDR skybox texture and set it as the skybox in the scene manager
	std::string skybox_file_path = "resources/textures/skyboxes/hdr/tiergarten_4k.hdr";
	if (std::filesystem::exists(skybox_file_path))
	{ // check if the file exists before loading it
		scene_manager->load_skybox(skybox_file_path);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::setup_initial_scene] Skybox file not found: " << skybox_file_path
			<< std::endl;
	}

	// print a success message indicating the initial scene setup is complete
	std::cout << "[SUCCESS::main::setup_initial_scene] Initial scene setup completed successfully"
		<< std::endl;
}

void setup_initial_test_scene(Core* engine)
{
	// retrieve the node manager and scene manager from the engine
	auto& node_manager = engine->get_node_manager();
	auto& scene_manager = engine->get_scene_manager();

	// create a camera with a placeholder name and default parameters
	auto camera = std::make_shared<Camera>("Camera");

	// convert the camera's id to string and set it as part of the camera's name
	camera->set_name(String_Utils::generate_id_prefixed_name(camera));
}