/*
 * Core.h
 * This file defines the Core class, which is is responsible for initializing OpenGL,
 * creating a window, and running the main loop.
 * It also manages the various managers used in the engine and holds the renderer instance,
 * and is thus responsible for coordinating their interactions.
 * It is a Singleton class.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <cstdint>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Camera;
class Renderer;
class Node_Manager;
class Input_Manager;
class Scene_Manager;
class Selection_Manager;
class Shader_Library;

class Core
{
private:
	// Constructors
	// ------------
	Core();

	// Destructor
	// ----------
	~Core();

	// Private Static Instance
	// -----------------------
	static Core* instance; // instance of the Core class (Singleton)

	// Private Attributes
	// ------------------
	Renderer* renderer;

	// manager instances
	std::shared_ptr<Node_Manager>      node_manager;
	std::shared_ptr<Input_Manager>     input_manager;
	std::shared_ptr<Scene_Manager>     scene_manager;
	std::shared_ptr<Selection_Manager> selection_manager;

	// library of the shader programs, loaded from the descriptor files of the resources directory
	std::shared_ptr<Shader_Library> shader_library;

	// screen settings
	GLuint screen_width{ 1600 }, screen_height{ 1000 }; // default screen width and height

	// absolute path of the Engine's resources directory (shaders, fonts, models, textures, etc.)
	std::filesystem::path resources_dir;

	// Private Static Methods
	// ----------------------
	// Destroys the instance of the Core class (Singleton)
	static void destroy_instance();

	// Private Methods
	// ---------------
	// Locates the Engine's resources directory (unless it was already set explicitly),
	// searching, in order, beside the executable, in the installation layout,
	// in the current working directory, and in the Engine's source tree.
	// Returns true if the directory was found, false otherwise
	bool resolve_resources_dir();

	// Loads the shader programs described by the descriptor files of the "shaders" subdirectory of the
	// resources directory into the shader library, and gives the renderer and the selection manager the
	// shaders they use. Returns true if every shader was loaded and the required ones exist, false otherwise
	bool load_shaders();

public:
	// Constructors
	// ------------
	Core(Core const&) = delete; // copy constructor (Singleton is not cloneable)

	// Operator overloads
	// ------------------
	void operator=(Core const&) = delete; // assignment operator (Singleton is not assignable)

	// Public Static Methods
	// ---------------------
	// Returns the instance of the Core class (Singleton)
	static Core* get_instance();

	// Public Methods
	// --------------
	// Getters
	Renderer* get_renderer() const { return renderer; }

	const std::shared_ptr<Node_Manager>&      get_node_manager() const { return node_manager; }
	const std::shared_ptr<Input_Manager>&     get_input_manager() const { return input_manager; }
	const std::shared_ptr<Scene_Manager>&     get_scene_manager() const { return scene_manager; }
	const std::shared_ptr<Selection_Manager>& get_selection_manager() const { return selection_manager; }
	const std::shared_ptr<Shader_Library>&    get_shader_library() const { return shader_library; }

	const GLuint&                get_screen_width() const { return screen_width; }
	const GLuint&                get_screen_height() const { return screen_height; }
	const std::filesystem::path& get_resources_dir() const { return resources_dir; }
	// returns the absolute path of a resource file given its path relative to the resources directory
	// (e.g., "shaders/skybox.vert" for a shader file located in the engine->get_resource_path("shaders") directory)
	std::string get_resource_path(const std::string& relative_path) const;

	// Setters
	void set_renderer(Renderer* renderer) { this->renderer = renderer; }

	void set_screen_width(GLuint width) { screen_width = width; }
	void set_screen_height(GLuint height) { screen_height = height; }
	// Sets the resources directory explicitly (must be called before init() to take effect on
	// the engine's shaders); otherwise, it is located automatically during init()
	void set_resources_dir(const std::filesystem::path& dir) { resources_dir = dir; }

	// Initializes the core engine (OpenGL, window, GUI, etc.), locates the resources directory,
	// and loads the engine's shaders into the shader library
	bool init();
	// Runs the main loop of the engine until the renderer signals that the window should close or,
	// if max_frames is greater than 0, until that number of frames has been rendered (e.g., for automated
	// tests without user input, in which case the loop does not wait for events between frames)
	void run(std::uint64_t max_frames = 0);
	// Frees resources in the correct order and shuts down the engine, destroying the Core instance
	void shutdown();

	// Loads the textures and adds them to the engine
	void load_textures(
		const std::vector<std::string>& texture_names,
		const std::vector<std::string>& texture_paths,
		const std::vector<std::string>& texture_types);

	// Callback functions
	void framebuffer_size_callback(GLint width, GLint height);
};
