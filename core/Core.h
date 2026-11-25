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
#include <optional>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer/Scene_Viewport.h"
#include "utils/geometry/Ray.h"

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Camera;
class Renderer;
class Node_Manager;
class Input_Manager;
class Scene_Manager;
class Selection_Manager;
class Shader_Library;
class Material_Library;

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
	// library of the materials, loaded from the descriptor files of the assets directory
	std::shared_ptr<Material_Library> material_library;

	// screen settings
	GLuint screen_width{ 1600 }, screen_height{ 1000 }; // default screen width and height
	// the rectangle of the framebuffer where the application draws the scene, or none for the whole framebuffer (the default)
	std::optional<Scene_Viewport> requested_scene_viewport;

	// absolute path of the Engine's resources directory (shaders, fonts, etc.)
	std::filesystem::path resources_dir;
	// absolute path of the Engine's assets directory (materials, models, and skyboxes, and in the future textures, etc.), which
	// applications and users organize in folders
	std::filesystem::path assets_dir;

	// Private Static Methods
	// ----------------------
	// Destroys the instance of the Core class (Singleton)
	static void destroy_instance();

	// Private Methods
	// ---------------
	// Locates a directory of the Engine (unless it was already set explicitly, in which case it only checks that
	// it exists), searching, in order, beside the executable, in the installation layout, in the current working
	// directory, and in the Engine's source tree. The folder name is the one searched beside the executable and
	// in the working directory (e.g., "resources"), and the other two are the locations of the installation layout
	// and of the source tree. Returns true if the directory was found, false otherwise
	static bool resolve_directory(
		std::filesystem::path&       directory,
		const std::string&           folder_name,
		const std::filesystem::path& install_directory,
		const std::filesystem::path& source_directory);

	// Locates the Engine's resources directory (see resolve_directory)
	bool resolve_resources_dir();

	// Locates the Engine's assets directory (see resolve_directory)
	bool resolve_assets_dir();

	// Loads the shader programs described by the descriptor files of the "shaders" subdirectory of the
	// resources directory into the shader library, and gives the renderer and the selection manager the
	// shaders they use. Returns true if every shader was loaded and the required ones exist, false otherwise
	bool load_shaders();
	// Loads the materials described by the descriptor files of the "materials" subdirectory of the assets
	// directory into the material library. Returns true if every material was loaded and the default
	// material exists, false otherwise
	bool load_materials();

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
	const std::shared_ptr<Material_Library>&  get_material_library() const { return material_library; }

	const GLuint&                get_screen_width() const { return screen_width; }
	const GLuint&                get_screen_height() const { return screen_height; }
	const std::filesystem::path& get_resources_dir() const { return resources_dir; }
	// returns the absolute path of a resource file given its path relative to the resources directory
	// (e.g., "shaders/skybox.vert" for a shader file located in the engine->get_resource_path("shaders") directory)
	std::string get_resource_path(const std::string& relative_path) const;

	const std::filesystem::path& get_assets_dir() const { return assets_dir; }
	// returns the absolute path of an asset file given its path relative to the assets directory
	// (e.g., "materials/default.material.json")
	std::string get_asset_path(const std::string& relative_path) const;

	// Returns the rectangle of the framebuffer where the scene is drawn: the one set with set_scene_viewport, kept inside the
	// framebuffer (Scene_Viewport::clipped_to), or the whole framebuffer if none was set. The projection of the camera, the
	// offscreen render pass, the picking, and the outline of the selection all have its size
	Scene_Viewport get_scene_viewport() const;
	// Returns the point of the surface drawn at a point of the framebuffer (in pixels from its top-left corner, as for
	// set_scene_viewport), read from the depth of the last rendered frame, or nothing when the point is outside the scene viewport,
	// over the sky or empty space, or there is no camera. A GUI layer calls it while it is drawn, so it finds what is on the screen
	// (for example, where to place an object that is dropped onto the scene). Transparent objects are not found
	std::optional<glm::vec3> find_surface_point(GLdouble x, GLdouble y) const;
	// Returns the ray from the camera through a point of the framebuffer (in pixels from its top-left corner), or nothing when the
	// point is outside the scene viewport or there is no camera
	std::optional<Ray> get_scene_ray(GLdouble x, GLdouble y) const;

	// Setters
	void set_renderer(Renderer* renderer) { this->renderer = renderer; }

	void set_screen_width(GLuint width) { screen_width = width; }
	void set_screen_height(GLuint height) { screen_height = height; }
	// Sets the rectangle of the framebuffer where the scene is drawn (e.g., the area that the windows of a GUI leave free), in
	// pixels from the top-left corner of the framebuffer; the rest of the framebuffer is cleared. It may change every frame: the
	// frame uses the rectangle that is set when the scene is rendered (a GUI layer sets it while it is drawn, before that)
	void set_scene_viewport(const Scene_Viewport& viewport) { requested_scene_viewport = viewport; }
	// Draws the scene on the whole framebuffer again (the default)
	void reset_scene_viewport() { requested_scene_viewport.reset(); }
	// Sets the resources directory explicitly (must be called before init() to take effect on
	// the engine's shaders); otherwise, it is located automatically during init()
	void set_resources_dir(const std::filesystem::path& dir) { resources_dir = dir; }
	// Sets the assets directory explicitly (must be called before init() to take effect on the engine's
	// materials); otherwise, it is located automatically during init()
	void set_assets_dir(const std::filesystem::path& dir) { assets_dir = dir; }

	// Initializes the core engine (OpenGL, window, GUI, etc.), locates the resources and assets directories,
	// and loads the engine's shaders and materials into the shader and material libraries
	bool init();
	// Runs the main loop of the engine until the renderer signals that the window should close or,
	// if max_frames is greater than 0, until that number of frames has been rendered (e.g., for automated
	// tests without user input, in which case the loop does not wait for events between frames). While the window
	// is minimized, nothing is rendered: the loop waits for events until it is restored
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
