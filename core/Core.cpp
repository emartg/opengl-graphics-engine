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
#include "shader/Shader_Library.h"
#include "material/Material_Library.h"
#include "texture/Texture.h"
#include "gizmos/Line.h"               // for directional light gizmo rendering
#include "gizmos/TRIANGLE_FAN_PLANE.h" // for directional light gizmo rendering
#include "camera/Camera.h"
#include "managers/Node_Manager.h"
#include "managers/Input_Manager.h"
#include "managers/Scene_Manager.h"
#include "managers/Selection_Manager.h"
#include "renderer/Renderer.h"
#include "utils/file_system/File_System_Utils.h"

// Static Instance initialization
// ------------------------------
Core* Core::instance{ nullptr };

// Constructors
// ------------
Core::Core() :
	renderer{ nullptr },
	node_manager{ std::make_shared<Node_Manager>() },
	input_manager{ std::make_shared<Input_Manager>() },
	scene_manager{ std::make_shared<Scene_Manager>() },
	selection_manager{ std::make_shared<Selection_Manager>() },
	shader_library{ std::make_shared<Shader_Library>() },
	material_library{ std::make_shared<Material_Library>() }
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
		delete instance;    // destroy the Core instance
		instance = nullptr; // nullify the pointer to avoid dangling pointer issues
	}
}

// Private Methods
// ---------------
bool Core::resolve_resources_dir()
{
	if (!resources_dir.empty())
	{ // if the directory was set explicitly, only verify that it exists
		std::error_code error;
		if (!std::filesystem::is_directory(resources_dir, error))
		{ // if it doesn't exist, print an error message and return false
			std::cerr << "[ERROR::CORE::resolve_resources_dir] Resources directory not found: " << resources_dir.string() << std::endl;
			return false;
		}
		resources_dir = std::filesystem::absolute(resources_dir, error);
	}
	else
	{ // otherwise, search the candidate locations in order of precedence
		std::filesystem::path executable_dir = File_System_Utils::get_executable_directory();
		std::error_code       error;
		std::filesystem::path current_dir = std::filesystem::current_path(error);

		std::vector<std::filesystem::path> candidates;
		if (!executable_dir.empty())
		{
			// resources staged beside the executable (e.g., a packaged build)
			candidates.push_back(executable_dir / "resources");
			// installation layout (<prefix>/bin/<executable> and <prefix>/<data dir>/.../resources)
			candidates.push_back(executable_dir.parent_path() / ENGINE_INSTALL_RESOURCES_DIR);
		}
		if (!current_dir.empty())
		{ // resources in the current working directory
			candidates.push_back(current_dir / "resources");
		}
		// resources in the Engine's source tree (development builds, where they are not copied)
		candidates.push_back(ENGINE_SOURCE_RESOURCES_DIR);

		resources_dir = File_System_Utils::find_first_existing_directory(candidates);
		if (resources_dir.empty())
		{ // if none of the candidates exists, print the searched locations and return false
			std::cerr << "[ERROR::CORE::resolve_resources_dir] Resources directory not found. Searched in:" << std::endl;
			for (const auto& candidate : candidates) std::cerr << "  " << candidate.string() << std::endl;
			return false;
		}
	}

	std::cout << "[INFO::CORE::resolve_resources_dir] Using resources directory: " << resources_dir.string() << std::endl;
	return true;
}

bool Core::load_shaders()
{
	// load the engine's shader programs, described by the descriptor files of the shaders directory
	if (!shader_library->load_directory(resources_dir / "shaders"))
		return false;

	// give the renderer the shaders of its passes, and the selection manager the picking shader
	if (!renderer->set_shaders(*shader_library))
		return false;
	if (selection_manager)
		selection_manager->set_picking_shader(shader_library->get("Picking Shader"));

	return true;
}

bool Core::load_materials()
{
	// load the engine's materials, described by the descriptor files of the materials directory
	if (!material_library->load_directory(resources_dir / "materials", *shader_library))
		return false;

	// the renderer draws the meshes without a material of their own with the default material
	if (!material_library->get(Material_Library::DEFAULT_MATERIAL))
	{
		std::cerr << "[ERROR::CORE::load_materials] The material library has no material named '" << Material_Library::DEFAULT_MATERIAL
				  << "'" << std::endl;
		return false;
	}
	return true;
}

// Public Methods
// --------------
std::string Core::get_resource_path(const std::string& relative_path) const
{
	return (resources_dir / relative_path).string();
}

bool Core::init()
{
	// locate the resources directory before anything is loaded from it
	if (!resolve_resources_dir())
	{ // if the resources directory is not found, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to locate the resources directory" << std::endl;
		return false;
	}

	// initialize the renderer
	if (!renderer->init())
	{ // if the renderer initialization fails, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to initialize renderer" << std::endl;
		return false;
	}

	// create a window with the specified width, height, and title; and configure it
	if (!renderer->create_window(screen_width, screen_height, "Test Window"))
	{ // if the window cannot be created, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to create the window" << std::endl;
		return false;
	}
	renderer->configure_window();

	// set callback functions
	renderer->set_callback_functions();

	// load all OpenGL function pointers with GLAD
	if (!gladLoadGLLoader(renderer->get_proc_address()))
	{ // if GLAD fails to load OpenGL functions, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to initialize GLAD" << std::endl;
		return false;
	}
	// now the OpenGL context is set up, and we can use OpenGL functions

	// print the OpenGL version and renderer of the context (useful to diagnose driver issues)
	std::cout << "[INFO::CORE::init] OpenGL " << glGetString(GL_VERSION) << " (" << glGetString(GL_RENDERER) << ", "
			  << glGetString(GL_VENDOR) << ")" << std::endl;

	// set the viewport to the window size
	renderer->set_viewport(screen_width, screen_height);

	// initialize selection manager picking FBO with current window size
	if (selection_manager)
		selection_manager->resize(screen_width, screen_height);
	// configure OpenGL global state
	renderer->config_opengl();
	// initialize the user interface
	renderer->init_gui();

	// load the engine's shaders, required by the renderer
	if (!load_shaders())
	{ // if the shaders fail to load, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to load the shaders" << std::endl;
		return false;
	}

	// load the engine's materials, which use the shaders
	if (!load_materials())
	{ // if the materials fail to load, print an error message and return false
		std::cerr << "[ERROR::CORE::init] Failed to load the materials" << std::endl;
		return false;
	}

	// if all the initializations are successful, print a success message and return true
	std::cout << "[SUCCESS::CORE::init] Core initialized successfully" << std::endl;
	return true;
}

void Core::run(std::uint64_t max_frames)
{
	std::cout << "[INFO::CORE::run] Starting main loop..." << std::endl;
	if (max_frames > 0)
		std::cout << "[INFO::CORE::run] The main loop will stop after " << max_frames << " frames" << std::endl;

	// number of frames rendered without waiting for events, at startup and after each event: the GUI needs
	// a few frames to settle some changes (e.g., new windows are laid out and become visible on the frame
	// after they first appear), which would otherwise only be displayed after the next event
	constexpr int FRAMES_AFTER_EVENT{ 3 };

	std::uint64_t frame_count{ 0 };                     // number of frames rendered so far
	int           pending_frames{ FRAMES_AFTER_EVENT }; // frames still to render before waiting for events again
	while (!renderer->should_close())
	{
		// without a frame limit, wait for events once the pending frames have been rendered: the renderer
		// fully blocks until an event occurs, and when that happens, the renderer processes frames but
		// throttles the frame rate, reducing CPU / GPU usage and improving performance.
		// With a frame limit (e.g., automated tests without user input), never block, so that frames keep
		// being rendered (the events are still polled at the start of every frame)
		if (max_frames == 0)
		{
			if (pending_frames == 0)
			{
				renderer->wait_for_events();
				pending_frames = FRAMES_AFTER_EVENT;
			}
			--pending_frames;
		}

		renderer->frame_start_config(); // start of the frame configuration
		renderer->render_scene();       // composite the scene
		renderer->render_gui();         // render the GUI
		renderer->frame_end_config();   // end of the frame configuration

		// stop once the requested number of frames has been rendered (if there is a frame limit)
		if (max_frames > 0 && ++frame_count >= max_frames)
		{
			std::cout << "[INFO::CORE::run] Frame limit reached (" << frame_count << " frames)" << std::endl;
			break;
		}
	}
}

void Core::shutdown()
{
	if (renderer)
	{ // check if the renderer is not null before shutting it down
		std::cout << "[INFO::CORE::shutdown] Destroying renderer..." << std::endl;
		renderer->shutdown_gui(); // the GUI must be shut down before the renderer

		// release every OpenGL object while the OpenGL context still exists (i.e., before the renderer
		// destroys the window): the selection render passes, the scene nodes, the shader programs,
		// and the renderer's own objects
		selection_manager.reset();
		scene_manager.reset();
		node_manager.reset();
		material_library.reset();
		shader_library.reset();
		renderer->release_resources();

		delete renderer;    // destroy the renderer (and its window) before the Core instance
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

void Core::load_textures(
	const std::vector<std::string>& texture_names,
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
	screen_width  = static_cast<GLuint>(std::max(0, width));
	screen_height = static_cast<GLuint>(std::max(0, height));

	// update the default framebuffer viewport
	if (renderer)
		renderer->set_viewport(screen_width, screen_height);

	// keep picking/outline FBOs in sync with the default framebuffer size (the actual window size)
	if (selection_manager)
		selection_manager->resize(width, height);
}
