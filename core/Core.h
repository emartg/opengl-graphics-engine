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
	std::shared_ptr<Node_Manager> node_manager;
	std::shared_ptr<Input_Manager> input_manager;
	std::shared_ptr<Scene_Manager> scene_manager;
	std::shared_ptr<Selection_Manager> selection_manager;

	// screen settings
	GLuint screen_width{ 1600 }, screen_height{ 1000 }; // default screen width and height

	// Private Static Methods
	// ----------------------
	// Destroys the instance of the Core class (Singleton)
	static void destroy_instance();

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

	const std::shared_ptr<Node_Manager>& get_node_manager() const { return node_manager; }
	const std::shared_ptr<Input_Manager>& get_input_manager() const { return input_manager; }
	const std::shared_ptr<Scene_Manager>& get_scene_manager() const { return scene_manager; }
	const std::shared_ptr<Selection_Manager>& get_selection_manager() const { return selection_manager; }

	const GLuint& get_screen_width() const { return screen_width; }
	const GLuint& get_screen_height() const { return screen_height; }

	// Setters
	void set_renderer(Renderer* renderer) { this->renderer = renderer; }

	void set_screen_width(GLuint width) { screen_width = width; }
	void set_screen_height(GLuint height) { screen_height = height; }

	// Initializes the core engine (OpenGL, window, GUI, etc.)
	bool init() const;
	// Runs the main loop of the engine until the renderer signals that the window should close
	void run();
	// Frees resources in the correct order and shuts down the engine, destroying the Core instance
	void shutdown();

	// Creates the shader programs, and delegates the shader compilation and program linking to the renderer.
	// Returns true if compilation and linking were successful, false otherwise
	bool compile_shaders(const std::vector<std::string>& shader_names,
						 const std::vector<std::string>& vertex_shader_paths,
						 const std::vector<std::string>& fragment_shader_paths);
	bool compile_shaders(const std::vector<std::string>& shader_names,
						 const std::vector<std::string>& vertex_shader_paths,
						 const std::vector<std::string>& geometry_shader_paths,
						 const std::vector<std::string>& fragment_shader_paths);

	// Loads the textures and adds them to the engine
	void load_textures(const std::vector<std::string>& texture_names,
					   const std::vector<std::string>& texture_paths,
					   const std::vector<std::string>& texture_types);

	// Callback functions
	void framebuffer_size_callback(GLint width, GLint height);

};
