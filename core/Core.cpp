/*
* Core.h
* This file implments the Core class, which is is responsible for initializing OpenGL,
* creating a window, and running the main loop.
* It also manages the various managers used in the engine and holds the renderer instance,
* and is thus responsible for coordinating their interactions.
* It is a Singleton class.
*/

#include "Core.h"

#include <iostream>
#include <memory> 
#include <algorithm>

#include "shader/Shader.h"
#include "texture/Texture.h"
#include "gizmos/Line.h" // for directional light gizmo rendering
#include "gizmos/TRIANGLE_FAN_PLANE.h" // for directional light gizmo rendering
#include "camera/Camera.h"
#include "managers/Node_Manager.h"
#include "managers/Input_Manager.h"
#include "managers/Scene_Manager.h"
#include "managers/Selection_Manager.h"
#include "renderer/Renderer.h"

// Static Instance initialization
// ------------------------------
Core* Core::instance{ nullptr };

// Constructors
// ------------
Core::Core()
	: renderer{ nullptr },
	node_manager{ std::make_shared<Node_Manager>() },
	input_manager{ std::make_shared<Input_Manager>() },
	scene_manager{ std::make_shared<Scene_Manager>() },
	selection_manager{ std::make_shared<Selection_Manager>() }
{}

// Destructor
// ----------
Core::~Core()
{
	std::cout << "[INFO::CORE::~Core] Core destructor called" << std::endl;
}

// Public Static Methods
// ---------------------
Core* Core::get_instance()
{
	if (!instance)
	{ // if the Core instance is null, create a new instance
		std::cout << "[INFO::CORE::get_instance] Creating Core instance..." << std::endl;
		instance = new Core();
	}
	return instance;
}

// Private Static Methods
// ----------------------
void Core::destroy_instance()
{
	if (instance)
	{ // check if the Core instance is not null before destroying it
		std::cout << "[INFO::CORE::destroy_instance] Destroying Core instance..." << std::endl;
		delete instance; // destroy the Core instance
		instance = nullptr; // nullify the pointer to avoid dangling pointer issues
	}
}

// Public Methods
// --------------
bool Core::init() const
{
	// use the current time as seed for the random number generator 
	// (to get different positions, colors, etc. each run)
	srand(static_cast<unsigned int>(time(0)));

	// initialize the renderer
	if (!renderer->init())
	{ // if the renderer initialization fails, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to initialize renderer" << std::endl;
		return false;
	}

	// create a window with the specified width, height, and title; and configure it
	renderer->create_window(screen_width, screen_height, "Test Window");
	renderer->configure_window();

	// set callback functions
	renderer->set_callback_functions();

	// load all OpenGL function pointers with GLAD
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(renderer->get_proc_address())))
	{ // if GLAD fails to load OpenGL functions, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to initialize GLAD" << std::endl;
		return false;
	}
	// now the OpenGL context is set up, and we can use OpenGL functions

	// set the viewport to the window size
	renderer->set_viewport(screen_width, screen_height);

	// initialize selection manager picking FBO with current window size
	if (selection_manager)
		selection_manager->resize(screen_width, screen_height);
	// configure OpenGL global state
	renderer->config_opengl();
	// initialize the user interface
	renderer->init_gui();

	// if all the initializations are successful, print a success message and return true
	std::cout << "[SUCCESS::CORE::init] Core initialized successfully" << std::endl;
	return true;
}

void Core::run()
{
	std::cout << "[INFO::CORE::run] Starting main loop..." << std::endl;
	while (!renderer->should_close())
	{
		// always wait for events first: the renderer fully blocks until an event occurs,
		// and when that happens, the renderer processes frames but throttles the frame rate,
		// reducing CPU / GPU usage and improving performance
		renderer->wait_for_events();

		renderer->frame_start_config(); // start of the frame configuration
		renderer->render_scene(); // composite the scene
		renderer->render_gui(); // render the GUI
		renderer->frame_end_config(); // end of the frame configuration
	}
}

void Core::shutdown()
{
	if (renderer)
	{ // check if the renderer is not null before shutting it down
		std::cout << "[INFO::CORE::shutdown] Destroying renderer..." << std::endl;
		renderer->shutdown_gui(); // the GUI must be shut down before the renderer
		delete renderer; // destroy the renderer before the Core instance
		renderer = nullptr; // nullify the pointer to avoid dangling pointer issues
	}
	else
	{ // if the renderer is null, print an error message and return
		std::cerr << "[INFO::CORE::shutdown] Renderer is null during Core destruction!" << std::endl;
		return;
	}

	// destroy the Core instance itself and print a message to the console
	destroy_instance();
	std::cout << "[INFO::CORE::shutdown] Core shutdown complete" << std::endl;
}

bool Core::compile_shaders(const std::vector<std::string>& shader_names,
						   const std::vector<std::string>& vertex_shader_paths,
						   const std::vector<std::string>& fragment_shader_paths)
{
	size_t shader_count = shader_names.size(); // number of shaders to compile

	// ensure the sizes of the input vectors match
	if (vertex_shader_paths.size() != shader_count
		|| fragment_shader_paths.size() != shader_count)
	{ // if the sizes do not match, print an error message and return false
		std::cerr << "[ERROR::CORE::compile_shaders] Mismatched shader names and paths sizes!" << std::endl;
		return false;
	}

	for (size_t i{}; i < shader_count; i++)
	{ // iterate through the shader names and paths
		// create a new Shader object with the name and paths, and compile it
		auto shader = std::make_shared<Shader>(
			shader_names[i], vertex_shader_paths[i], fragment_shader_paths[i]);
		if (!shader->compile()) // compile the shader
		{ // if the shader compilation fails, print an error message and return false
			std::cerr << "[ERROR::CORE::compile_shaders] Failed to compile shader: "
				<< shader_names[i] << std::endl;
			return false;
		}

		// set the shader in the renderer by name
		if (!renderer->set_shader_by_name(shader->get_name(), shader))
		{ // if the shader was not set successfully, print an error message and return false
			std::cerr << "[ERROR::CORE::compile_shaders] Failed to set shader with name '" << shader_names[i]
				<< "' in the renderer (unknown name or null shader)" << std::endl;
			return false;
		}
		else
		{ // if the shader was set successfully, print a success message
			std::cout << "[SUCCESS::CORE::compile_shaders] Shader with name '" << shader_names[i]
				<< "' set successfully in the renderer" << std::endl;
		}

		// add the compiled shader to the node manager
		node_manager->add_node(std::move(shader));

		// if this is the picking shader, set it in the selection manager
		if (shader_names[i] == "Picking Shader" && selection_manager)
		{
			auto added = std::dynamic_pointer_cast<Shader>(node_manager->get_nodes(Node_Type::SHADER).back());
			selection_manager->set_picking_shader(added);
			std::cout << "[INFO::CORE::compile_shaders] Picking Shader assigned to selection manager"
				<< std::endl;
		}
	}

	// if all shaders are compiled successfully, print a success message and return true
	std::cout << "[SUCCESS::CORE::compile_shaders] Shaders compiled successfully" << std::endl;
	return true;
}

bool Core::compile_shaders(const std::vector<std::string>& shader_names,
						   const std::vector<std::string>& vertex_shader_paths,
						   const std::vector<std::string>& geometry_shader_paths,
						   const std::vector<std::string>& fragment_shader_paths)
{
	// number of shaders to compile
	size_t shader_count = shader_names.size();

	// ensure the sizes of the input vectors match
	if (vertex_shader_paths.size() != shader_count
		|| geometry_shader_paths.size() != shader_count
		|| fragment_shader_paths.size() != shader_count)
	{ // if the sizes do not match, print an error message and return false
		std::cerr << "[ERROR::CORE::compile_shaders] Mismatched shader names and paths sizes!" << std::endl;
		return false;
	}

	for (size_t i{}; i < shader_count; i++)
	{ // iterate through the shader names and paths
		// create a new Shader object with the name and paths, and compile it
		auto shader = std::make_shared<Shader>(shader_names[i],
											   vertex_shader_paths[i],
											   geometry_shader_paths[i],
											   fragment_shader_paths[i]);
		if (!shader->compile()) // compile the shader
		{ // if the shader compilation fails, print an error message and return false
			std::cerr << "[ERROR::CORE::compile_shaders] Failed to compile shader: "
				<< shader_names[i] << std::endl;
			return false;
		}
		// add the compiled shader to the node manager
		node_manager->add_node(std::move(shader));
	}

	// if all shaders are compiled successfully, print a success message and return true
	std::cout << "[SUCCESS::CORE::compile_shaders] Shaders compiled successfully" << std::endl;
	return true;
}

void Core::load_textures(const std::vector<std::string>& texture_names,
						 const std::vector<std::string>& texture_paths,
						 const std::vector<std::string>& texture_types)
{
	for (GLuint i{}; i < texture_names.size(); i++)
	{
		auto texture = std::make_shared<Texture>(texture_names[i], texture_paths[i], Texture_Type::DIFFUSE);
		node_manager->add_node(std::move(texture));
	}
}

void Core::framebuffer_size_callback(GLint width, GLint height)
{
	// keep Core's notion of the default framebuffer size in sync with the actual window size
	screen_width = static_cast<GLuint>(std::max(0, width));
	screen_height = static_cast<GLuint>(std::max(0, height));

	// update the default framebuffer viewport
	if (renderer) renderer->set_viewport(screen_width, screen_height);

	// keep picking/outline FBOs in sync with the default framebuffer size (the actual window size)
	if (selection_manager) selection_manager->resize(width, height);
}