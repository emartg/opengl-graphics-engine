/*
 * Renderer.h
 * Defines the interface for all renderers, which are responsible for:
 * - Initializing OpenGL
 * - Creating and managing windows
 * - Rendering graphics
 * - Managing GUI elements
 * - Event polling and buffer swapping
 * - Handling input
 * - Etc.
 * This interface allows for different implementations of renderers, such as GLFW_Renderer, SDLRenderer, etc.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Core;
class Node;
class Render_Pass;
class Shader;
class Shader_Library;
class Material;
class Mesh;
class Texture;

enum class Buffer_Type
{
	UNDEFINED = 0,
	COLOR,
	DEPTH,
	STENCIL,
	COLOR_DEPTH,
	COLOR_STENCIL,
	DEPTH_STENCIL,
	ALL
};

class Renderer
{
public:
	// Public Types
	// ------------
	// Struct that holds screen debug parameters to test screen-to-texture rendering (with default values)
	struct Screen_Debug_Params
	{
		GLuint    debug_mode{ 0 };                    // 0: regular rendering, 1: solid color, 2: grid overlay, 3: inverted colors
		glm::vec3 solid_color{ 0.75f, 0.25f, 0.25f }; // solid color for debug mode 1
		GLuint    grid_line_count{ 50 };              // number of lines for the grid overlay for debug mode 2
		GLfloat   grid_line_thickness{ 1.50f };       // line thickness in pixels for the grid overlay for debug mode 2
		glm::vec3 grid_bg_color{ 0.25f };             // background color for the grid overlay for debug mode 2
		glm::vec3 grid_line_color{ 0.75f };           // line color for the grid overlay for debug mode 2
	};

	// Constructor
	// -----------
	Renderer();

	// Destructor
	// ----------
	// pure virtual destructor, must be defined to allow derived classes to implement it
	virtual ~Renderer();

	// Public Methods
	// --------------
	virtual bool init() const = 0;
	virtual void config_opengl() const;
	// Creates a window with an OpenGL 4.5 core profile context and makes the context current.
	// Returns true if successful, false otherwise (e.g., if OpenGL 4.5 is not supported)
	virtual bool create_window(int width, int height, const char* title) = 0;
	virtual void configure_window() const                                = 0;
	virtual void poll_io_events() const                                  = 0;
	virtual void swap_buffers() const                                    = 0;
	virtual void clear_buffers(Buffer_Type buffer_type = Buffer_Type::ALL) const;
	virtual bool should_close() const = 0;

	// Blocks the main thread until an event occurs
	// (mouse movement, key press, node manipulation via GUI, etc.)
	// Keeps held widgets responsive thanks to a reasonable throttle refresh rate
	// when events are being processed
	virtual void wait_for_events() const = 0;

	// Getters
	// returns the function used by GLAD to load the OpenGL function pointers of the current context
	virtual GLADloadproc get_proc_address() const = 0;
	virtual float        get_time() const         = 0;
	virtual float        get_delta_time() const { return delta_time; }

	virtual Screen_Debug_Params& get_screen_debug_params() { return screen_debug_params; }

	bool get_is_frustum_culling_enabled() const { return is_frustum_culling_enabled; }
	// number of drawable nodes in the scene, and number of them skipped by frustum culling, in the last frame
	std::size_t get_drawable_node_count() const { return drawable_node_count; }
	std::size_t get_culled_node_count() const { return culled_node_count; }

	bool get_is_instancing_enabled() const { return is_instancing_enabled; }
	// number of nodes drawn with instancing, and number of instanced draw calls used for them, in the last frame
	std::size_t get_instanced_node_count() const { return instanced_node_count; }
	std::size_t get_instanced_draw_call_count() const { return instanced_draw_call_count; }

	// number of lights uploaded to the light buffer in the last frame
	std::size_t get_light_count() const { return light_count_last_frame; }

	// Setters
	virtual void set_callback_functions() const = 0;
	virtual void set_viewport(int width, int height) const;
	virtual void set_clear_color(float r, float g, float b, float a = 1.0f) const;
	virtual void set_window_should_close() const = 0;

	virtual void set_screen_debug_params(const Screen_Debug_Params& params) { screen_debug_params = params; }

	void set_is_frustum_culling_enabled(bool is_enabled) { is_frustum_culling_enabled = is_enabled; }
	void set_is_instancing_enabled(bool is_enabled) { is_instancing_enabled = is_enabled; }

	// Takes the shader programs used by the render passes from the shader library, by name (e.g., "Skybox Shader").
	// Returns true if the library has all of them, false otherwise (printing the missing ones)
	virtual bool set_shaders(const Shader_Library& shader_library);

	// Releases the OpenGL objects owned by the renderer (render passes, buffers, etc.).
	// It must be called while the OpenGL context still exists (i.e., before the window is destroyed);
	// otherwise, the destructor releases them. It can be called more than once
	void release_resources();

	// GUI
	virtual void init_gui() {};
	virtual void build_gui() const {};
	virtual void render_gui() const {};
	virtual void shutdown_gui() {};

	// Render Passes
	virtual void frame_start_config();
	virtual void render_scene();
	virtual void frame_end_config() const;

	// Registration/Unregistration for dynamic environment map capture for models by their unique ids
	virtual void register_model_for_dynamic_env_map_capture(std::uint32_t model_id, GLuint resolution = 512);
	virtual void unregister_model_for_dynamic_env_map_capture(std::uint32_t model_id);

protected:
	// Protected Attributes
	// --------------------
	Render_Pass* main_render_pass;
	GLfloat      delta_time, last_frame_time; // time settings

	// shader programs' smart pointers
	std::shared_ptr<Shader> single_albedo_shader;
	std::shared_ptr<Shader> screen_quad_shader;
	std::shared_ptr<Shader> picking_shader;
	std::shared_ptr<Shader> skybox_shader;
	std::shared_ptr<Shader> equirect_to_cubemap_shader;
	std::shared_ptr<Shader> reflective_shader;
	std::shared_ptr<Shader> refractive_shader;

	// buffers for the screen quad (for rendering the offscreen texture to the screen)
	GLuint screen_quad_vao{}, screen_quad_vbo{}, screen_quad_ebo{};
	GLuint offscreen_width{}, offscreen_height{};

	// screen debug parameters for testing screen-to-texture rendering
	Screen_Debug_Params screen_debug_params;

	// frustum culling: whether the nodes outside the camera's view are skipped, and statistics of the last frame
	bool        is_frustum_culling_enabled{ true };
	std::size_t drawable_node_count{ 0 }; // nodes with geometry to draw (visible and not pure containers)
	std::size_t culled_node_count{ 0 };   // drawable nodes skipped because they are outside the camera's view

	// instanced rendering: whether opaque shapes sharing a geometry are drawn with a single draw call,
	// the buffer with their per-instance data, and statistics of the last frame
	bool        is_instancing_enabled{ true };
	GLuint      instance_vbo{ 0 };
	std::size_t instanced_node_count{ 0 };      // nodes drawn with instancing
	std::size_t instanced_draw_call_count{ 0 }; // instanced draw calls (one per group of nodes sharing a geometry)

	// light buffer: a shader storage buffer with all the lights of the scene in world space (any number of them),
	// uploaded once per frame and read by the lit shaders from the binding point LIGHT_BUFFER_BINDING
	static constexpr GLuint LIGHT_BUFFER_BINDING{ 0 };
	GLuint                  light_ssbo{ 0 };
	std::size_t             light_count_last_frame{ 0 };

	// buffers for the skybox cube
	GLuint skybox_vao{}, skybox_vbo{}, skybox_ebo{};

	// buffers and flags for the HDR to cubemap conversion
	GLuint hdr_to_cubemap_fbo{}, hdr_to_cubemap_rbo{};
	bool   hdr_to_cubemap_converted{ false };
	GLuint hdr_source_tex_id{ 0 }; // to track if the HDR source texture has changed

	// attributes for dynamic environment maps
	struct Dynamic_Env_Map_Entry
	{
		GLuint fbo{ 0 }, rbo{ 0 };
		GLuint cubemap_tex_id{ 0 };      // write target for this frame
		GLuint prev_cubemap_tex_id{ 0 }; // stable sampling source for this frame
		GLuint resolution{ 512 };        // default resolution
		bool   initialized{ false };
		bool   has_prev_cubemap{ false }; // indicates if prev_cubemap_tex_id is valid (rendered at least once)
	};
	// map of dynamic env maps by entity id
	std::unordered_map<std::uint32_t, Dynamic_Env_Map_Entry> dynamic_env_maps;
	bool is_capturing_dynamic_env_map{ false }; // flag to prevent recursion during dynamic env map capture

	// Protected Methods
	// -----------------
	// Renders a single node (i.e., only its own meshes if it has any, without rendering its children)
	// with the appropriate shader program
	void render_node(const std::shared_ptr<Node>& node);

	// Returns the material that draws a mesh of a node: the node's material (if it has one, for all its meshes),
	// the mesh's own material (e.g., the material of an imported model), or the default material
	std::shared_ptr<Material> get_mesh_material(const Node& node, const Mesh& mesh) const;
	// Uses the shader of the material, and sets the material parameters and textures, the parameters that the node
	// overrides, the albedo color of the node, and its world model matrix. Returns the shader, or nullptr if the
	// material has no shader
	std::shared_ptr<Shader> bind_material(const Material& material, const Node& node) const;
	// Draws the meshes of a node (if any), each one with its material
	void draw_model_meshes(const Node& node) const;

	// Renders the opaque nodes: the nodes that share a geometry and a material that supports instancing (with a
	// single mesh, single-sided, and without overridden parameters) are drawn with one instanced draw call per
	// geometry and material, and the other nodes one by one
	void render_opaque_nodes(const std::vector<std::shared_ptr<Node>>& nodes);

	// Uploads the lights of the scene to the light buffer (in world space, sorted by type), and binds it
	void upload_lights();

	// Ensures the offscreen render pass is created with the current window size
	void ensure_offscren_render_pass();
	// Initializes the screen quad if it has not been initialized yet
	void init_screen_quad();
	// Composites the offscreen render pass texture to the screen
	void composite_to_screen();

	// Creates and configures the skybox cube buffers if not already done
	void init_skybox_cube();
	// Renders the skybox cube with the given texture, view, and projection matrices
	void render_skybox_cube(std::shared_ptr<Texture> skybox_texture, glm::mat4& view, glm::mat4& projection);

	// Converts an equirectangular HDR texture to a cubemap texture if needed
	void convert_hdr_to_cubemap_if_needed();

	// Dynamic environment map helpers
	void update_dynamic_env_maps();
	void capture_dynamic_env_map_for_model(const std::shared_ptr<Node>& model, Dynamic_Env_Map_Entry& entry);
	void render_scene_for_env_map_capture(
		const glm::mat4&             capture_view,
		const glm::mat4&             capture_projection,
		const std::shared_ptr<Node>& exclude_model);
};
