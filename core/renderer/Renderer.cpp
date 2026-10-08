/*
 * Renderer.cpp
 * Implements the Renderer interface, which is responsible for:
 * - Initializing OpenGL
 * - Creating and managing windows
 * - Rendering graphics
 * - Managing GUI elements
 * - Event polling and buffer swapping
 * - Handling input
 * - Etc.
 * This interface allows for different implementations of renderers based on different windowing libraries,
 * such as GLFW, SDL, etc.
 */

#include "Renderer.h"

#include <cstddef>
#include <cstring>
#include <functional>
#include <map>
#include <utility>

#include "SKYBOX.h"      // skybox vertex data
#include "SCREEN_QUAD.h" // screen-quad vertex data
#include "Render_Pass.h"
#include "../Core.h"
#include "../Node.h"
#include "../gizmos/Line.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/Directional_Light.h"
#include "../light/Point_Light.h"
#include "../light/Spotlight.h"
#include "../model/Model.h"
#include "../model/mesh/Mesh.h"
#include "../model/mesh/Mesh_Geometry.h"
#include "../shader/Shader.h"
#include "../shader/Shader_Library.h"
#include "../material/Material.h"
#include "../material/Material_Library.h"
#include "../texture/Texture.h"
#include "../utils/geometry/Bounding_Box.h"
#include "../utils/geometry/Frustum.h"
#include "../managers/Node_Manager.h"
#include "../managers/Scene_Manager.h"
#include "../managers/Selection_Manager.h"

// Constructor
// -----------
Renderer::Renderer() : main_render_pass{ new Render_Pass() }, delta_time{ 0.0f }, last_frame_time{ 0.0f } {}

// Destructor
// ----------
Renderer::~Renderer()
{
	// release the OpenGL objects if they were not released yet (see release_resources)
	release_resources();

	// delete the main render pass and nullify the pointer to avoid dangling pointer issues
	if (main_render_pass)
	{
		delete main_render_pass;
		main_render_pass = nullptr;
	}

	std::cout << "[RENDERER::~Renderer] Renderer destructor called" << std::endl;
}

// Public Methods
// --------------
void Renderer::release_resources()
{
	// deallocate the OpenGL objects of the main render pass (the object itself is deleted by the destructor)
	if (main_render_pass)
		main_render_pass->deallocate_resources();

	// delete screen-quad GL objects if created, and reset their ids (so that they are not deleted twice)
	if (screen_quad_ebo)
		glDeleteBuffers(1, &screen_quad_ebo);
	if (screen_quad_vbo)
		glDeleteBuffers(1, &screen_quad_vbo);
	if (screen_quad_vao)
		glDeleteVertexArrays(1, &screen_quad_vao);
	screen_quad_ebo = screen_quad_vbo = screen_quad_vao = 0;

	// delete skybox GL objects if created, and reset their ids
	if (skybox_ebo)
		glDeleteBuffers(1, &skybox_ebo);
	if (skybox_vbo)
		glDeleteBuffers(1, &skybox_vbo);
	if (skybox_vao)
		glDeleteVertexArrays(1, &skybox_vao);
	skybox_ebo = skybox_vbo = skybox_vao = 0;

	// clean up dynamic environment map FBOs, RBOs, and cubemap textures
	for (auto& [id, entry] : dynamic_env_maps) release_dynamic_env_map(entry);
	dynamic_env_maps.clear();

	// delete the instance buffer if created, and reset its id
	if (instance_vbo)
		glDeleteBuffers(1, &instance_vbo);
	instance_vbo = 0;

	// delete the light buffer if created, and reset its id
	if (light_ssbo)
		glDeleteBuffers(1, &light_ssbo);
	light_ssbo = 0;
}

void Renderer::config_opengl() const
{
	// depth buffer configuration:
	// 1. Enable the depth test
	// 2. Set the depth function to GL_LESS, which is the default depth function,
	//    i.e., discard fragments whose depth value is greater than or equal to
	//    the current fragment's depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // default depth function (discard fragments behind the current fragment)

	// stencil buffer configuration:
	// 1. Enable the stencil test
	// 2. Set the stencil operation to the default operation, which means that
	//    the stencil value will not be modified by the stencil test whatever the outcome
	//    of the depth and stencil tests is
	// 3. Set the stencil function to pass only if the stencil value is not equal
	//    to the reference value, which is set to 1 in this case
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	// texture configuration:
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // enable seamless cubemap sampling

	// face culling configuration:
	glEnable(GL_CULL_FACE); // enable face culling
	glCullFace(GL_BACK);    // cull back faces (default)
	glFrontFace(GL_CCW);    // counter-clockwise wound faces are front faces (default)

	// blending configuration:
	glEnable(GL_BLEND);                                // enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // standard alpha blending function (default)
	glBlendEquation(GL_FUNC_ADD);                      // standard blend equation (default)
}

void Renderer::clear_buffers(Buffer_Type buffer_type) const
{
	GLbitfield mask = 0;
	switch (buffer_type)
	{
		case Buffer_Type::COLOR: mask |= GL_COLOR_BUFFER_BIT; break;
		case Buffer_Type::DEPTH: mask |= GL_DEPTH_BUFFER_BIT; break;
		case Buffer_Type::STENCIL: mask |= GL_STENCIL_BUFFER_BIT; break;
		case Buffer_Type::COLOR_DEPTH: mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT; break;
		case Buffer_Type::COLOR_STENCIL: mask |= GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT; break;
		case Buffer_Type::DEPTH_STENCIL: mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT; break;
		case Buffer_Type::ALL:
		default: mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT; break;
	}
	glClear(mask); // clear the specified buffers
}

void Renderer::set_viewport(int width, int height) const
{
	glViewport(0, 0, width, height);
}

void Renderer::set_clear_color(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a); // alpha is optional, default is 1.0f
}

bool Renderer::set_shaders(const Shader_Library& shader_library)
{
	// names of the shaders used by the render passes, and the member variables that hold them
	const std::pair<const char*, std::shared_ptr<Shader>*> required_shaders[]{
		{ "Single Albedo Shader", &single_albedo_shader },
		{ "Screen Quad Shader", &screen_quad_shader },
		{ "Picking Shader", &picking_shader },
		{ "Skybox Shader", &skybox_shader },
		{ "Equirectangular to Cubemap Shader", &equirect_to_cubemap_shader },
	};

	bool has_all_shaders = true;
	for (const auto& [name, member] : required_shaders)
	{
		*member = shader_library.get(name);
		if (!*member)
		{ // if the library does not have the shader, print an error message (and keep checking the rest)
			std::cerr << "[ERROR::RENDERER::set_shaders] The shader library has no shader named '" << name << "'" << std::endl;
			has_all_shaders = false;
		}
	}
	return has_all_shaders;
}

void Renderer::frame_start_config()
{
	poll_io_events(); // poll IO events (keyboard, mouse, etc.)

	build_gui(); // set up the GUI for the frame

	// update time attributes
	GLfloat current_frame = static_cast<GLfloat>(get_time());
	delta_time            = current_frame - last_frame_time;
	last_frame_time       = current_frame;

	ensure_offscren_render_pass(); // ensure offscreen target matches current window size

	auto  core          = Core::get_instance();
	auto& scene_manager = core->get_scene_manager();
	auto& camera        = scene_manager->get_camera();

	// perform all pre-render passes (cubemap generation, dynamic reflections, etc.)
	// before binding the main render pass and setting up the main camera

	// upload the lights of the scene to the light buffer, used by the lit shaders in every pass of the frame
	upload_lights();

	// ensure the skybox is a valid cubemap texture early (needed for environment mapping)
	// and update per-object dynamic environment maps (if any)
	if (scene_manager->get_skybox())
		convert_hdr_to_cubemap_if_needed();
	update_dynamic_env_maps();

	// process pending picking request before main scene rendering
	core->get_selection_manager()->process_pending_pick(camera.get(), core->get_node_manager().get());

	// bind offscreen FBO and clear it
	main_render_pass->bind();
	set_clear_color(0.1f, 0.1f, 0.1f);
	clear_buffers(Buffer_Type::ALL);
}

void Renderer::render_scene()
{
	//// start measuring time for performance profiling
	// auto start_time{ std::chrono::high_resolution_clock::now() };

	auto core = Core::get_instance(); // get the core instance
	if (!core)                        // ensure the core instance is valid before proceeding
	{                                 // if the core instance is null, print an error message and return
		std::cerr << "[ERROR::RENDERER::render_scene] Core instance is null" << std::endl;
		return;
	}

	// get the node manager, scene manager, and camera from the core instance
	auto& node_manager  = core->get_node_manager();
	auto& scene_manager = core->get_scene_manager();
	auto& camera        = scene_manager->get_camera();

	// ensure camera and shaders are valid before proceeding
	if (!camera || !single_albedo_shader || !screen_quad_shader || !picking_shader || !skybox_shader || !equirect_to_cubemap_shader)
	{ // if any of them are null, print an error message and return
		std::cerr << "[ERROR::RENDERER::render_scene] Camera or shaders aren't set up correctly" << std::endl;
		return;
	}

	// compute view and projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(camera->get_zoom()),
		static_cast<GLfloat>(core->get_screen_width()) / static_cast<GLfloat>(core->get_screen_height()),
		0.1f,
		100.0f);
	glm::mat4 view = camera->get_view_matrix();

	// transpose of the upper-left 3x3 submatrix of the view matrix, used for environment mapping
	// (i.e., the inverse matrix of the rotation part of the view matrix)
	glm::mat3 inv_view_rot = glm::transpose(glm::mat3(view));

	// set the view and projection matrices, and the inverse view rotation (used by the shaders that sample
	// environment maps), for each shader program of the shader library
	for (const auto& shader : core->get_shader_library()->get_shaders())
	{
		shader->use();
		shader->set_mat4("u_view", view);
		shader->set_mat4("u_projection", projection);
		shader->set_mat3("u_inv_view_rot", inv_view_rot);
	}

	// lights of the scene (their data is uploaded to the light buffer by upload_lights(), in frame_start_config())
	auto& lights = node_manager->get_nodes(Node_Type::LIGHT);

	// flatten scene graph into a single list of drawable nodes
	std::vector<std::shared_ptr<Node>> drawable_nodes;
	auto&                              all_nodes = node_manager->get_nodes(Node_Type::MODEL);

	// define a recursive lambda function to traverse the scene graph and collect drawable nodes,
	// which are nodes that are visible and not pure containers (i.e., they have geometry to render)
	std::function<void(const std::shared_ptr<Node>&)> collect_drawable_nodes = [&](const std::shared_ptr<Node>& node) {
		if (!node)
			return; // if the node is null, return
		if (!node->get_is_visible())
			return; // skip invisible nodes

		// if the node is not a pure container, add it to the list of drawables
		if (node->get_type() != Node_Type::COMPOSITE_MODEL)
			drawable_nodes.push_back(node);

		// recursively collect drawable nodes from the children of the current node
		for (const auto& child : node->get_children()) collect_drawable_nodes(child);
	};

	// start collecting drawable nodes from all root nodes in the scene graph
	for (const auto& node : all_nodes)
	{
		if (node && !node->get_parent())
			collect_drawable_nodes(node);
	}

	// frustum culling: skip the nodes whose bounding box is completely outside the camera's view
	// (nodes without a valid bounding box, i.e., without meshes, are kept, since they cannot be tested)
	drawable_node_count = drawable_nodes.size();
	culled_node_count   = 0;
	if (is_frustum_culling_enabled)
	{
		const Frustum frustum{ projection * view };
		std::erase_if(drawable_nodes, [&frustum](const std::shared_ptr<Node>& node) {
			const Bounding_Box bounding_box = node->get_world_bounding_box();
			return bounding_box.is_valid() && !frustum.intersects(bounding_box);
		});
		culled_node_count = drawable_node_count - drawable_nodes.size();
	}

	// separate all drawable nodes into opaque and transparent lists for correct rendering order
	std::vector<std::shared_ptr<Node>> opaque_nodes, transparent_nodes;
	opaque_nodes.reserve(drawable_nodes.size());
	transparent_nodes.reserve(drawable_nodes.size());
	for (const auto& node : drawable_nodes)
	{
		if (node->get_albedo().a < 1.0f) // if the node has alpha less than 1, consider it transparent
			transparent_nodes.push_back(node);
		else // otherwise, consider it opaque
			opaque_nodes.push_back(node);
	}

	// sort transparent nodes back-to-front based on their distance to the camera for correct blending
	const auto& camera_pos = camera->get_position();
	std::sort(transparent_nodes.begin(), transparent_nodes.end(), [&](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
		// compute the distance from the camera to each node using their world positions
		float dist_a = glm::length(camera_pos - a->get_world_position());
		float dist_b = glm::length(camera_pos - b->get_world_position());
		return dist_a > dist_b; // sort in descending order (back-to-front)
	});

	// opaque pass: render all opaque geometry first to ensure correct depth testing and early z-culling
	render_opaque_nodes(opaque_nodes);

	// skybox pass: render the skybox after opaque geometry, but before transparent geometry rendering
	// (this ensures the skybox is rendered behind all opaque geometry, but does not interfere with
	// transparent geometry rendering, which may require depth sorting and blending with the skybox)
	auto& skybox_texture = scene_manager->get_skybox();
	if (skybox_texture)
	{ // if a skybox texture is set, proceed to render the skybox
		// perform (or retry) HDR to cubemap conversion if needed
		convert_hdr_to_cubemap_if_needed();

		skybox_texture = scene_manager->get_skybox(); // refresh pointer (conversion replaces the texture)

		// render the skybox only if the texture is a cubemap now
		if (skybox_texture && skybox_texture->get_texture_type() == Texture_Type::CUBEMAP)
		{ // if the skybox texture is now a cubemap, render the skybox
			if (!skybox_vao)
				init_skybox_cube(); // initialize the skybox cube if not done yet
			render_skybox_cube(skybox_texture, view, projection);
		}
		else if (skybox_texture && skybox_texture->get_texture_type() == Texture_Type::HDR_EQUIRECTANGULAR)
		{ // if the skybox is still HDR, print a warning message but continue rendering
			// only warn occasionally (avoid spamming every frame)
			static uint32_t warn_counter = 0;
			if ((warn_counter++ % 240) == 0)
				std::cerr << "[WARNING::RENDERER::render_scene] "
							 "Skybox still HDR (conversion pending)"
						  << std::endl;
		}
	}

	// transparent pass: render all transparent nodes after opaque geometry and skybox
	// to ensure correct blending with the background and other geometry, and to allow depth testing
	// against the opaque geometry and skybox (but not against other transparent nodes,
	// which is why they are sorted back-to-front)
	glDepthMask(GL_FALSE); // disable depth writing for transparent pass to allow blending with background
	for (const auto& node : transparent_nodes) render_node(node);
	glDepthMask(GL_TRUE); // re-enable depth writing after transparent pass

	// disable face culling for light gizmos to ensure they are always visible
	glDisable(GL_CULL_FACE);

	// set rendering mode to wireframe for light gizmos
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// iterate over the vector of lights and render their gizmo children (if any)
	for (const auto& node : lights)
	{
		// dynamically cast the node to a Light object
		auto light = std::dynamic_pointer_cast<Light>(node);
		if (!light)
			continue; // if the cast fails, skip to the next node

		// set up the single albedo shader for rendering light gizmos
		single_albedo_shader->use();
		single_albedo_shader->set_mat4("u_projection", projection);
		single_albedo_shader->set_mat4("u_view", view);

		auto gizmo = light->get_gizmo();
		if (gizmo)
		{
			if (!gizmo->get_is_visible())
				continue; // skip invisible gizmos

			// set the model matrix and albedo color for the light gizmo
			single_albedo_shader->set_mat4("u_model", gizmo->get_model_matrix());
			// use light's diffuse color for the gizmo (to match light color)
			single_albedo_shader->set_vec3("u_albedo", light->get_diffuse());

			gizmo->draw(); // draw the light gizmo

			// for directional lights, also render the direction line as part of the gizmo
			if (light->get_light_type() == Light_Type::DIRECTIONAL_LIGHT)
			{
				auto directional_light = std::dynamic_pointer_cast<Directional_Light>(light);
				if (directional_light)
				{
					auto& direction_line = directional_light->get_gizmo_direction_line();
					if (direction_line)
					{
						// set the model matrix and albedo color for the direction line
						// identity matrix for the line (as it does not need any further transformation)
						single_albedo_shader->set_mat4("u_model", glm::mat4(1.0f));
						// use light's diffuse color for the direction line (to match light color)
						single_albedo_shader->set_vec3("u_albedo", directional_light->get_diffuse());

						direction_line->draw(); // draw the directional light direction line
					}
				}
			}
		}
	}

	// reset rendering mode to fill after rendering the light gizmos
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// re-enable face culling after rendering the light gizmos
	glEnable(GL_CULL_FACE);

	// after the scene is rendered offscreen, composite to the default framebuffer
	composite_to_screen();

	//// stop measuring time for performance profiling
	// auto end_time{ std::chrono::high_resolution_clock::now() };

	//// calculate the elapsed time in milliseconds and store it in the elapsed_time variable
	// auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	//// print the elapsed time for rendering the scene with a counter to avoid spamming the console
	// static uint32_t render_counter = 0;
	// if ((render_counter++ % 60) == 0) // print every 60 frames
	//	std::cout << "[PROFILER::RENDERER::render_scene] Scene rendered in "
	//	<< elapsed_time << " ms" << std::endl;
}

void Renderer::frame_end_config() const
{
	swap_buffers();
}

// Protected Methods
// -----------------
void Renderer::render_node(const std::shared_ptr<Node>& node)
{
	if (!node)
	{ // if the node is null, print an error message and return
		std::cerr << "[ERROR::RENDERER::render_node] Node is null" << std::endl;
		return;
	}

	if (!node->get_is_visible())
		return; // if the node is not visible, skip rendering

	// handle two-sided nodes by disabling face culling temporarily if needed
	const GLboolean cull_was_enabled     = glIsEnabled(GL_CULL_FACE);
	const bool      need_disable_culling = node->get_is_two_sided();
	if (need_disable_culling && cull_was_enabled)
		glDisable(GL_CULL_FACE);

	// render the node based on its gizmo type
	switch (node->get_gizmo_type())
	{
		case Gizmo_Type::NONE: // if the node is not a gizmo, render its meshes (if any) with their materials
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill mode for regular nodes
			draw_model_meshes(*node);
		}
		break;
		// gizmos for light sources (DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT)
		// are all rendered with the single albedo shader and in wireframe mode
		case Gizmo_Type::DIRECTIONAL_LIGHT:
		case Gizmo_Type::POINT_LIGHT:
		case Gizmo_Type::SPOTLIGHT:
		{
			single_albedo_shader->use();
			// set the color of the gizmo shape based on the node's albedo
			single_albedo_shader->set_vec3("u_albedo", node->get_albedo());
			single_albedo_shader->set_mat4("u_model", node->get_world_model_matrix());

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // line mode for light gizmos
			node->draw(*single_albedo_shader);
		}
		break;
		default: // if the gizmo type is unknown, print an error message
			std::cerr << "[ERROR::RENDERER::render_node] Unknown gizmo type for node: " << node->get_name() << std::endl;
			break;
	}

	// re-enable face culling if it was previously enabled
	if (need_disable_culling && cull_was_enabled)
		glEnable(GL_CULL_FACE);
}

std::shared_ptr<Material> Renderer::get_mesh_material(const Node& node, const Mesh& mesh) const
{
	// the node's material (which replaces the materials of all its meshes), the mesh's own material
	// (e.g., the material of an imported model), or the default material
	if (node.get_material())
		return node.get_material();
	if (mesh.get_material())
		return mesh.get_material();
	return Core::get_instance()->get_material_library()->get(Material_Library::DEFAULT_MATERIAL);
}

std::shared_ptr<Shader> Renderer::bind_material(const Material& material, const Node& node) const
{
	if (!material.get_shader())
	{ // if the material has no shader, print an error message and return
		std::cerr << "[ERROR::RENDERER::bind_material] The material " << material.get_name() << " has no shader" << std::endl;
		return nullptr;
	}

	// use the material's shader, and set the material parameters and textures, the parameters that the node
	// overrides, the albedo color of the node (whose alpha is its opacity), and its world model matrix
	const auto& shader = material.get_shader();
	shader->use();
	material.apply();
	material.apply(node.get_material_overrides());
	shader->set_vec4("u_object_albedo", node.get_albedo());
	shader->set_mat4("u_model", node.get_world_model_matrix());
	if (material.get_environment_mode() != Environment_Mode::NONE)
		bind_environment_map(material, node, *shader);
	return shader;
}

void Renderer::bind_environment_map(const Material& material, const Node& node, const Shader& shader) const
{
	// the environment maps use a texture unit after the units of the texture slots of the shaders (which are limited)
	static_assert(ENVIRONMENT_MAP_UNIT >= Shader_Library::MAX_TEXTURE_SLOTS, "The environment map unit must follow the texture slots");

	// the skybox cubemap (if any), used by the skybox materials, and by the dynamic ones until their cubemap exists
	GLuint      cubemap_id     = 0;
	const auto& skybox_texture = Core::get_instance()->get_scene_manager()->get_skybox();
	if (skybox_texture && skybox_texture->get_texture_type() == Texture_Type::CUBEMAP)
		cubemap_id = skybox_texture->get_texture_id();

	// the dynamic cubemap of the node: while the cubemaps are being captured, the one of the previous frame (complete),
	// and afterwards, the one captured in this frame
	if (material.get_environment_mode() == Environment_Mode::DYNAMIC)
	{
		const auto it = dynamic_env_maps.find(node.get_id());
		if (it != dynamic_env_maps.end() && it->second.initialized)
		{
			const auto& entry = it->second;
			if (!is_capturing_dynamic_env_map)
				cubemap_id = entry.cubemap_tex_id;
			else if (entry.has_prev_cubemap)
				cubemap_id = entry.prev_cubemap_tex_id;
		}
	}

	// bind the cubemap to its texture unit, after the units of the material's texture slots
	glActiveTexture(GL_TEXTURE0 + ENVIRONMENT_MAP_UNIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
	shader.set_int("u_environment_map", ENVIRONMENT_MAP_UNIT);
	glActiveTexture(GL_TEXTURE0);
}

void Renderer::draw_model_meshes(const Node& node) const
{
	// draw each mesh of the node with its material, binding a material only when it changes between meshes
	const Material* bound_material = nullptr;
	for (const auto& mesh : node.get_meshes())
	{
		if (!mesh)
			continue;

		const auto material = get_mesh_material(node, *mesh);
		if (!material)
		{ // if there is no material (not even the default one), print an error message and skip the mesh
			std::cerr << "[ERROR::RENDERER::draw_model_meshes] No material to render a mesh of " << node.get_name() << std::endl;
			continue;
		}
		if (material.get() != bound_material)
		{
			if (!bind_material(*material, node))
				continue;
			bound_material = material.get();
		}
		mesh->draw();
	}
}

void Renderer::upload_lights()
{
	// light data, laid out as the Light_Data struct of the lit shaders (std430: each vec4 takes 16 bytes)
	struct Light_Data
	{
		glm::vec4 position;    // xyz: world-space position (point lights and spotlights), w: light type
		glm::vec4 direction;   // xyz: world-space direction (directional lights and spotlights)
		glm::vec4 ambient;     // rgb: ambient component
		glm::vec4 diffuse;     // rgb: diffuse component
		glm::vec4 specular;    // rgb: specular component
		glm::vec4 attenuation; // x: constant, y: linear, z: quadratic factor (point lights and spotlights)
		glm::vec4 cutoffs;     // x: inner, y: outer cut-off (spotlights)
	};
	static_assert(sizeof(Light_Data) == 7 * sizeof(glm::vec4), "Light_Data must match the std430 layout of the shaders");

	// light types, as defined in the lit shaders
	constexpr float DIRECTIONAL_LIGHT{ 0.0f };
	constexpr float POINT_LIGHT{ 1.0f };
	constexpr float SPOTLIGHT{ 2.0f };

	// gather the lights sorted by type (directional lights, point lights, and spotlights, in their order within the
	// scene), so that the shaders accumulate their contributions in the same order as before
	std::vector<Light_Data> directional_lights, point_lights, spotlights;
	for (const auto& node : Core::get_instance()->get_node_manager()->get_nodes(Node_Type::LIGHT))
	{
		auto light = dynamic_cast<Light*>(node.get());
		if (!light)
			continue;

		switch (light->get_light_type())
		{
			case Light_Type::DIRECTIONAL_LIGHT:
			{
				auto directional_light = dynamic_cast<Directional_Light*>(light);
				directional_lights.push_back(
					Light_Data{ glm::vec4{ 0.0f, 0.0f, 0.0f, DIRECTIONAL_LIGHT },
								glm::vec4{ directional_light->get_direction(), 0.0f },
								glm::vec4{ directional_light->get_ambient(), 0.0f },
								glm::vec4{ directional_light->get_diffuse(), 0.0f },
								glm::vec4{ directional_light->get_specular(), 0.0f },
								glm::vec4{ 0.0f },
								glm::vec4{ 0.0f } });
			}
			break;
			case Light_Type::POINT_LIGHT:
			{
				auto point_light = dynamic_cast<Point_Light*>(light);
				point_lights.push_back(
					Light_Data{ glm::vec4{ point_light->get_position(), POINT_LIGHT },
								glm::vec4{ 0.0f },
								glm::vec4{ point_light->get_ambient(), 0.0f },
								glm::vec4{ point_light->get_diffuse(), 0.0f },
								glm::vec4{ point_light->get_specular(), 0.0f },
								glm::vec4{ point_light->get_constant(), point_light->get_linear(), point_light->get_quadratic(), 0.0f },
								glm::vec4{ 0.0f } });
			}
			break;
			case Light_Type::SPOTLIGHT:
			{
				auto spotlight = dynamic_cast<Spotlight*>(light);
				spotlights.push_back(
					Light_Data{ glm::vec4{ spotlight->get_position(), SPOTLIGHT },
								glm::vec4{ spotlight->get_direction(), 0.0f },
								glm::vec4{ spotlight->get_ambient(), 0.0f },
								glm::vec4{ spotlight->get_diffuse(), 0.0f },
								glm::vec4{ spotlight->get_specular(), 0.0f },
								glm::vec4{ spotlight->get_constant(), spotlight->get_linear(), spotlight->get_quadratic(), 0.0f },
								glm::vec4{ spotlight->get_inner_cutoff(), spotlight->get_outer_cutoff(), 0.0f, 0.0f } });
			}
			break;
			default:
				// lights of an undefined or unknown type are skipped
				std::cerr << "[ERROR::RENDERER::upload_lights] Unknown light type for light: " << light->get_name() << std::endl;
				break;
		}
	}

	// buffer contents: the number of lights (padded to 16 bytes, the alignment of the struct array that follows
	// it in the std430 layout), followed by the lights
	const GLint            light_count = static_cast<GLint>(directional_lights.size() + point_lights.size() + spotlights.size());
	std::vector<std::byte> buffer_data(16 + light_count * sizeof(Light_Data));
	std::memcpy(buffer_data.data(), &light_count, sizeof(light_count));
	std::size_t offset{ 16 };
	for (const auto* group : { &directional_lights, &point_lights, &spotlights })
	{
		std::memcpy(buffer_data.data() + offset, group->data(), group->size() * sizeof(Light_Data));
		offset += group->size() * sizeof(Light_Data);
	}

	// upload the data (reallocating the storage every frame, so that the driver does not wait for the previous
	// frame's draw calls) and bind the buffer to the binding point read by the shaders
	if (!light_ssbo)
		glGenBuffers(1, &light_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_data.size(), buffer_data.data(), GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_BUFFER_BINDING, light_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	light_count_last_frame = static_cast<std::size_t>(light_count);
}

void Renderer::render_opaque_nodes(const std::vector<std::shared_ptr<Node>>& nodes)
{
	instanced_node_count      = 0;
	instanced_draw_call_count = 0;

	// per-instance data of a node drawn with instancing (read by the material's shader as vertex attributes)
	struct Instance_Data
	{
		glm::mat4 model;  // world model matrix (attribute locations 3 to 6)
		glm::vec4 albedo; // albedo color (attribute location 7)
	};

	// group of nodes sharing a geometry and a material
	struct Instance_Group
	{
		Mesh_Geometry*                     geometry{ nullptr };
		std::shared_ptr<Material>          material;
		std::vector<std::shared_ptr<Node>> nodes;
	};

	// group the nodes that can be drawn with instancing (those whose material supports it, without overridden
	// parameters, untextured, single-sided, and with a single mesh) by geometry and material (in order of first
	// appearance), and render every other node (and every node, if instancing is disabled) one by one
	std::vector<Instance_Group>                                 groups;
	std::map<std::pair<Mesh_Geometry*, Material*>, std::size_t> group_indices; // index of each group
	for (const auto& node : nodes)
	{
		const auto& meshes          = node->get_meshes();
		const bool  has_single_mesh = meshes.size() == 1 && meshes[0] && meshes[0]->get_geometry();
		const auto  material        = has_single_mesh ? get_mesh_material(*node, *meshes[0]) : nullptr;
		const bool  is_instanceable = is_instancing_enabled && material && material->get_supports_instancing() &&
			node->get_material_overrides().empty() && node->get_gizmo_type() == Gizmo_Type::NONE && !node->get_is_two_sided();
		if (!is_instanceable)
		{
			render_node(node);
			continue;
		}

		Mesh_Geometry* geometry = meshes[0]->get_geometry().get();
		auto [it, inserted]     = group_indices.try_emplace({ geometry, material.get() }, groups.size());
		if (inserted)
			groups.push_back(Instance_Group{ geometry, material, {} });
		groups[it->second].nodes.push_back(node);
	}

	for (const auto& group : groups)
	{
		// a geometry used by a single node is rendered as usual (instancing would not save any draw call)
		if (group.nodes.size() < 2)
		{
			render_node(group.nodes.front());
			continue;
		}

		// gather the per-instance data of the group
		std::vector<Instance_Data> instances;
		instances.reserve(group.nodes.size());
		for (const auto& node : group.nodes) instances.push_back(Instance_Data{ node->get_world_model_matrix(), node->get_albedo() });

		// upload the per-instance data (the buffer is created on first use, and its storage is reallocated every
		// time, so that the driver does not have to wait for the previous draw calls that read it)
		if (!instance_vbo)
			glGenBuffers(1, &instance_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
		glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(Instance_Data), instances.data(), GL_STREAM_DRAW);

		// bind the shared geometry and add the per-instance attributes to it (advancing once per instance):
		// a mat4 attribute takes four locations, one per column
		glBindVertexArray(group.geometry->get_vao());
		for (GLuint column = 0; column < 4; ++column)
		{
			const GLuint location = 3 + column;
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(
				location,
				4,
				GL_FLOAT,
				GL_FALSE,
				sizeof(Instance_Data),
				(void*)(offsetof(Instance_Data, model) + column * sizeof(glm::vec4)));
			glVertexAttribDivisor(location, 1);
		}
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Instance_Data), (void*)offsetof(Instance_Data, albedo));
		glVertexAttribDivisor(7, 1);

		// draw all the instances with a single draw call
		const auto& shader = group.material->get_shader();
		shader->use();
		group.material->apply();
		shader->set_bool("u_instanced", GL_TRUE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElementsInstanced(
			GL_TRIANGLES,
			group.geometry->get_index_count(),
			GL_UNSIGNED_INT,
			0,
			static_cast<GLsizei>(instances.size()));
		shader->set_bool("u_instanced", GL_FALSE);

		// remove the per-instance attributes from the geometry, which is also drawn without instancing
		for (GLuint location = 3; location <= 7; ++location) glDisableVertexAttribArray(location);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		instanced_node_count += group.nodes.size();
		++instanced_draw_call_count;
	}
}

void Renderer::ensure_offscren_render_pass()
{
	// get the core instance and screen dimensions
	auto      core   = Core::get_instance();
	const int width  = core->get_screen_width();
	const int height = core->get_screen_height();

	if (width <= 0 || height <= 0)
		return; // ensure valid dimensions

	// if the dimensions have changed or the FBO is not created yet,
	// recreate the offscreen render pass with the new dimensions
	if (width != offscreen_width || height != offscreen_height || main_render_pass->get_fbo_id() == 0)
	{
		// reset the specification and set new values
		Render_Pass_Specification spec{};
		spec.width                  = width;
		spec.height                 = height;
		spec.color_attachment_count = 1;
		spec.has_depth_attachment   = true;
		spec.has_stencil_attachment = true;
		// enable depth texture as it is needed later for depth-aware outline composite
		// (to sample the selected object's depth in the main scene)
		spec.depth_as_texture = true;

		// create the main render pass with the new specification and dimensions
		main_render_pass->create(spec);
		offscreen_width  = width;
		offscreen_height = height;

		// print a message indicating the offscreen render pass has been recreated
		std::cout << "[INFO::RENDERER::ensure_offscren_render_pass] "
					 "\nOffscreen render pass recreated with updated dimensions:\n"
				  << main_render_pass->get_specification_str() << std::endl;
	}

	init_screen_quad(); // initialize the screen quad if it hasn't been initialized yet
}

void Renderer::init_screen_quad()
{
	if (screen_quad_vao)
		return; // if the screen quad VAO is already initialized, return

	// create the screen quad VAO, VBO, and EBO
	glGenVertexArrays(1, &screen_quad_vao);
	glGenBuffers(1, &screen_quad_vbo);
	glGenBuffers(1, &screen_quad_ebo);

	glBindVertexArray(screen_quad_vao); // bind the VAO to set up the vertex attributes

	// bind the VBO and EBO, and send the screen quad vertex and index data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(screen_quad_vertices_vector.size() * sizeof(GLfloat)),
		screen_quad_vertices_vector.data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screen_quad_ebo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(screen_quad_indices_vector.size() * sizeof(GLuint)),
		screen_quad_indices_vector.data(),
		GL_STATIC_DRAW);

	// set the vertex attribute pointers for the screen quad
	// compute a local stride for the vertex attributes (5 floats per vertex: 3 pos, 2 tex coords)
	const GLsizei stride = static_cast<GLsizei>(5 * sizeof(GLfloat));
	// position attribute at index 0 (3 floats)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	// texture coordinate attribute at index 1 (2 floats)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(GLfloat)));

	glBindVertexArray(0); // unbind the VAO to avoid accidental modifications
}

void Renderer::composite_to_screen()
{
	// get the core instance and screen dimensions
	auto         core   = Core::get_instance();
	const GLuint width  = core->get_screen_width();
	const GLuint height = core->get_screen_height();

	// get the selection manager from the core instance
	auto selection_manager = core->get_selection_manager();

	// depending on the debug mode and whether there is a selection,
	// render the outline mask or the picking visualization
	if (screen_debug_params.debug_mode <= 1 && selection_manager->get_selected_node_id() != 0)
	{
		// if debug mode is either normal mode (1) or Inverted Colors mode (3), and there is a selection,
		// render the outline mask
		auto camera = core->get_scene_manager()->get_camera();
		selection_manager->render_outline_mask(camera.get(), core->get_node_manager().get());
	}
	else if (screen_debug_params.debug_mode == 2)
	{
		// if debug mode is the Picking Colors mode (2), render picking visualization
		auto camera = core->get_scene_manager()->get_camera();
		selection_manager->render_picking_visualization(camera.get(), core->get_node_manager().get());
	}

	// unbind offscreen target, effectively switching back to the default framebuffer
	main_render_pass->unbind();

	// draw the offscreen color texture to the back buffer via a screen quad
	glViewport(0, 0, width, height);           // set the viewport to the screen dimensions
	glDisable(GL_DEPTH_TEST);                  // depth test is not needed for screen quad rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure we are in fill mode for the screen quad

	// clear the default framebuffer color (keep depth and stencil buffers intact)
	clear_buffers(Buffer_Type::COLOR);

	if (!screen_quad_shader)
	{ // if the screen shader is not set, print an error message and return
		std::cerr << "[ERROR::RENDERER::composite_to_screen] Screen Quad Shader not set" << std::endl;
		return;
	}

	// use the screen shader and set the texture to be rendered
	screen_quad_shader->use();
	// set the screen texture uniform to texture unit 0
	screen_quad_shader->set_int("u_screen_texture", 0);
	// set screen debug parameters as uniforms in the screen fragment shader
	std::string prefix = "u_screen_debug_params."; // prefix for the screen debug parameters
	screen_quad_shader->set_int(prefix + "debug_mode", screen_debug_params.debug_mode);
	screen_quad_shader->set_vec3(prefix + "solid_color", screen_debug_params.solid_color);
	screen_quad_shader->set_int(prefix + "grid_line_count", screen_debug_params.grid_line_count);
	screen_quad_shader->set_float(prefix + "grid_line_thickness", screen_debug_params.grid_line_thickness);
	screen_quad_shader->set_vec3(prefix + "grid_bg_color", screen_debug_params.grid_bg_color);
	screen_quad_shader->set_vec3(prefix + "grid_line_color", screen_debug_params.grid_line_color);
	// set the screen dimensions as a uniform in the screen fragment shader
	screen_quad_shader->set_vec2("u_screen_size", glm::vec2(width, height));

	// bind the offscreen render pass texture to texture unit 0 and set it as the active texture
	glActiveTexture(GL_TEXTURE0);

	// if debug mode is Picking Colors mode (2), depending on whether the picking texture is available,
	// bind the picking texture or the main color texture; otherwise, bind the main color texture
	if (screen_debug_params.debug_mode == 2)
	{
		if (selection_manager->get_picking_texture_id() != 0)
		{ // if the picking texture is available, bind it and set nearest filtering
			glBindTexture(GL_TEXTURE_2D, selection_manager->get_picking_texture_id());
			// use nearest filtering to avoid interpolating encoded ids
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{ // if the picking texture is not available, print a warning and bind the main color texture instead
			std::cerr << "[WARNING::RENDERER::composite_to_screen] Picking texture not available, "
						 "binding main render pass color texture instead"
					  << std::endl;
			glBindTexture(GL_TEXTURE_2D, main_render_pass->get_texture_id(0));
		}
	}
	else
	{ // otherwise, bind the main render pass color texture
		glBindTexture(GL_TEXTURE_2D, main_render_pass->get_texture_id(0));
	}

	// local variables for outline parameters
	// there is only and outline if there is a selected node and the outline mask texture is available
	const bool has_outline = selection_manager->get_selected_node_id() != 0 && selection_manager->get_outline_mask_texture_id() != 0;
	glm::vec3  outline_color{ 0.0f };
	GLuint     outline_thickness = 0;

	auto& params = selection_manager->get_outline_params(); // get the outline parameters
	if (screen_debug_params.debug_mode <= 1 && has_outline)
	{
		// if debug mode is either v_normal mode (1) or Inverted Colors mode (2)
		// and there is a selection, use the outline parameters from the selection manager
		outline_color     = params.color;
		outline_thickness = params.thickness;
	}

	// set outline parameters as uniforms in the screen fragment shader
	screen_quad_shader->set_int("u_has_outline", has_outline);
	screen_quad_shader->set_vec3("u_outline_color", outline_color);
	screen_quad_shader->set_int("u_outline_thickness", outline_thickness);

	// bind depth textures if available for depth-aware outline rendering and set related uniforms
	GLuint scene_depth_tex = main_render_pass->get_depth_texture_id(); // retrieve the scene depth texture
	// for the outline depth texture, reuse selection manager's outline pass depth (valid if mask rendered)
	GLuint outline_depth_tex = 0;
	if (has_outline)
	{ // if there is an outline to render, get the outline depth texture
		outline_depth_tex = selection_manager->get_outline_depth_texture_id();
	}

	// inform the shader if depth textures are available and set a small bias to avoid z-fighting
	// (need at least the scene depth texture for depth-aware outline rendering,
	// outline depth is tested within the shader if available)
	const bool depth = scene_depth_tex != 0;
	screen_quad_shader->set_int("u_has_depth_textures", depth ? 1 : 0);
	screen_quad_shader->set_float("u_outline_depth_bias", 0.0005f);

	// bind depth textures to texture units 2 and 3 (only if available)
	glActiveTexture(GL_TEXTURE2);
	screen_quad_shader->set_int("u_scene_depth_texture", 2);
	glBindTexture(GL_TEXTURE_2D, scene_depth_tex);
	glActiveTexture(GL_TEXTURE3);
	screen_quad_shader->set_int("u_outline_depth_texture", 3);
	glBindTexture(GL_TEXTURE_2D, outline_depth_tex);

	// bind the outline mask texture to texture unit 1 and set it as the active texture
	glActiveTexture(GL_TEXTURE1);
	screen_quad_shader->set_int("u_outline_mask_texture", 1);
	if (has_outline) // if there is an outline to render, bind the outline mask texture
		glBindTexture(GL_TEXTURE_2D, selection_manager->get_outline_mask_texture_id());
	else // otherwise, bind texture 0 to avoid undefined behavior in the shader
		glBindTexture(GL_TEXTURE_2D, 0);

	// bind the screen quad VAO and draw the screen quad
	glBindVertexArray(screen_quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	// unbind the texture to avoid accidental modifications
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST); // re-enable the depth test for subsequent rendering
}

void Renderer::init_skybox_cube()
{
	// create skybox VAO, VBO, and EBO
	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glGenBuffers(1, &skybox_ebo);

	// bind and set skybox VAO, VBO, and EBO
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, skybox_positions_vector.size() * sizeof(GLfloat), skybox_positions_vector.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox_indices_vector.size() * sizeof(GLuint), skybox_indices_vector.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindVertexArray(0); // unbind the VAO after setting it up
}

void Renderer::render_skybox_cube(std::shared_ptr<Texture> skybox_texture, glm::mat4& view, glm::mat4& projection)
{
	// disable depth writing for the skybox to prevent it from overwriting
	// the depth values of the scene's objects (skybox is rendered at the farthest depth),
	// and set the depth function to GL_LEQUAL to ensure the skybox is rendered
	// correctly when depth values are equal (skybox depth is 1.0) - avoids z-fighting
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	// disable face culling for skybox rendering since camera is inside the cube
	// looking at the interior (back) faces. This ensures the skybox is rendered correctly,
	// and avoids winding order ambiguity when rendering from the inside
	glDisable(GL_CULL_FACE);

	// set view and projection matrices for the skybox shader
	skybox_shader->use();
	// for the skybox, remove the translation from the view matrix, since the skybox
	// should always be centered and appear infinitely far away,
	// so we cast the mat4 to a mat3 and back to a mat4 to remove the translation component.
	// This way, the skybox will not move when the camera moves
	// and will only rotate based on the camera's orientation
	glm::mat4 view_no_translation = glm::mat4(glm::mat3(view)); // remove translation with a mat3 cast
	skybox_shader->set_mat4("u_view", view_no_translation);
	skybox_shader->set_mat4("u_projection", projection);
	// explicitly set the skybox texture unit to 0 to prevent reliance on defaults or previous bindings
	skybox_shader->set_int("u_skybox", 0);

	// bind the VAO for the skybox and bind the cubemap texture, then render the skybox
	glBindVertexArray(skybox_vao);
	glActiveTexture(GL_TEXTURE0); // ensure the correct texture unit is active before binding (0)
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture->get_texture_id());
	glDrawElements(GL_TRIANGLES, skybox_index_count, GL_UNSIGNED_INT, 0);

	// reset depth function and re-enable depth writing after rendering the skybox
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// re-enable face culling after skybox rendering
	glEnable(GL_CULL_FACE);

	glBindVertexArray(0); // unbind the VAO after rendering
}

void Renderer::convert_hdr_to_cubemap_if_needed()
{
	// get the core instance, the scene manager, and the current skybox and check if it exists
	auto  core          = Core::get_instance();
	auto& scene_manager = core->get_scene_manager();
	auto& skybox        = scene_manager->get_skybox();
	if (!skybox)
		return; // if no skybox is set, return

	// in case the skybox is not HDR, there is nothing to convert, so return
	if (skybox->get_texture_type() != Texture_Type::HDR_EQUIRECTANGULAR)
		return;

	if (hdr_source_tex_id != skybox->get_texture_id())
	{ // if the HDR source texture has changed, reset the converted flag
		hdr_source_tex_id        = skybox->get_texture_id();
		hdr_to_cubemap_converted = false;
	}
	// only convert if not converted yet, otherwise return
	if (hdr_to_cubemap_converted)
		return;

	if (!equirect_to_cubemap_shader)
	{ // if the equirectangular to cubemap shader is not set, print an error message and return
		std::cerr << "[ERROR::RENDERER::convert_hdr_to_cubemap_if_needed] "
					 "Equirectangular to Cubemap Shader not set"
				  << std::endl;
		return;
	}

	// create HDR to cubemap FBO and RBO if not created yet
	// (they are capture FBO and RBOs because they are used to capture the six faces of the cubemap)
	if (!hdr_to_cubemap_fbo)
	{ // if the capture FBO and RBO are not created yet, create them
		glGenFramebuffers(1, &hdr_to_cubemap_fbo);
		glGenRenderbuffers(1, &hdr_to_cubemap_rbo);
	}

	// save currently bound FBO (main offscreen pass) so it can be restored later
	GLint prev_fbo{};
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo); // get currently bound FBO

	// set the capture resolution to 4096x4096 (sufficient for good quality skyboxes)
	const GLuint capture_size{ 4096 };

	// bind capture FBO and (re)configure depth RBO for the capture dimensions
	glBindFramebuffer(GL_FRAMEBUFFER, hdr_to_cubemap_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, hdr_to_cubemap_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, capture_size, capture_size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdr_to_cubemap_rbo);

	// explicitely select color attachment 0 for rendering (avoids issues on some drivers and platforms)
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// validate capture FBO completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{ // if the capture FBO is not complete, print an error message, restore previous FBO, and return
		std::cerr << "[ERROR::RENDERER::convert_hdr_to_cubemap_if_needed] Capture FBO incomplete" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
		return;
	}

	// create empty cubemap texture to render to and attach to FBO (RGB16F for HDR)
	GLuint env_cubemap;
	glGenTextures(1, &env_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
	for (unsigned int i{}; i < 6; ++i) // allocate space for the 6 faces of the cubemap
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, capture_size, capture_size, 0, GL_RGB, GL_FLOAT, nullptr);
	// set the cubemap texture parameters
	// set wrapping to clamp to edge to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// set filtering to linear and enable mipmaps for the cubemap
	// (i.e., use trilinear filtering when sampling the cubemap with mipmaps)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// view/projection matrices for capturing data onto the 6 cubemap face directions
	glm::mat4 capture_proj    = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 capture_views[] = {
		// 6 view matrices for the 6 faces of the cubemap (right, left, top, bottom, front, back)
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +X
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),   // +Y
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +Z
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))  // -Z
	};

	if (!skybox_vao)
		init_skybox_cube(); // initialize the skybox cube if not done yet

	// activate the equirectangular to cubemap shader and set uniforms for conversion
	equirect_to_cubemap_shader->use();
	equirect_to_cubemap_shader->set_int("u_equirect_map", 0);
	equirect_to_cubemap_shader->set_mat4("u_projection", capture_proj);

	// bind the HDR equirectangular map to texture unit 0 for the shader to sample from
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox->get_texture_id());

	// save current viewport so it can be restored later
	GLint prev_viewport[4];
	glGetIntegerv(GL_VIEWPORT, prev_viewport);
	glViewport(0, 0, capture_size, capture_size); // set viewport to the capture dimensions

	// render to each face of the cubemap by attaching it to the FBO and rendering the scene
	for (unsigned int i = 0; i < 6; ++i)
	{ // for each of the 6 faces of the cubemap, render the scene using the capture view
		equirect_to_cubemap_shader->set_mat4("u_view", capture_views[i]);
		// attach the face of the cubemap texture to the correponding FBO color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear the necessary buffers
		glBindVertexArray(skybox_vao);                                        // bind the skybox VAO
		glDrawElements(GL_TRIANGLES, skybox_index_count, GL_UNSIGNED_INT, 0); // render the skybox cube
	}

	// generate mipmaps for the cubemap texture to enable trilinear filtering
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// restore previous FBO
	glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
	// restore original viewport
	glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);

	// replace the skybox with the new cubemap texture (the old HDR texture is released by its Texture object
	// once it is no longer used)
	scene_manager->set_skybox(std::make_shared<Texture>("Skybox Cubemap From HDR", env_cubemap, Texture_Type::CUBEMAP));
	hdr_to_cubemap_converted = true; // mark as converted to avoid redundant conversions

	std::cout << "[SUCCESS::RENDERER::convert_hdr_to_cubemap_if_needed] Converted HDR (src texId = " << hdr_source_tex_id
			  << ") to cubemap (texId = " << env_cubemap << ")" << std::endl;

	// reset source id so a new HDR selection triggers fresh conversion logic properly
	hdr_source_tex_id = env_cubemap; // the active skybox is now the cubemap
}

void Renderer::release_dynamic_env_map(Dynamic_Env_Map_Entry& entry)
{
	// delete the FBO, RBO, and cubemap textures of the entry (if created), and reset their ids
	if (entry.fbo)
		glDeleteFramebuffers(1, &entry.fbo);
	if (entry.rbo)
		glDeleteRenderbuffers(1, &entry.rbo);
	if (entry.cubemap_tex_id)
		glDeleteTextures(1, &entry.cubemap_tex_id);
	if (entry.prev_cubemap_tex_id)
		glDeleteTextures(1, &entry.prev_cubemap_tex_id);
	entry = Dynamic_Env_Map_Entry{};
}

void Renderer::update_dynamic_env_maps()
{
	//// start measuring time for performance profiling
	// auto start_time{ std::chrono::high_resolution_clock::now() };

	// if already capturing, return
	if (is_capturing_dynamic_env_map)
		return;

	auto  core         = Core::get_instance();                      // get the core instance
	auto& node_manager = core->get_node_manager();                  // get the node manager
	auto& models       = node_manager->get_nodes(Node_Type::MODEL); // get the models from the node manager

	// find the models drawn with a material that needs a dynamic environment map (in any of their meshes),
	// with the highest resolution requested by their materials
	std::unordered_map<std::uint32_t, GLuint> required_resolutions;
	for (const auto& model : models)
	{
		if (!model || model->get_gizmo_type() != Gizmo_Type::NONE)
			continue;
		// skip the models that are not drawn: invisible ones, or those with an invisible ancestor (the main pass
		// does not traverse the children of invisible nodes)
		bool is_effectively_visible = model->get_is_visible();
		for (auto parent = model->get_parent(); is_effectively_visible && parent; parent = parent->get_parent())
			is_effectively_visible = parent->get_is_visible();
		if (!is_effectively_visible)
			continue;
		for (const auto& mesh : model->get_meshes())
		{
			const auto material = mesh ? get_mesh_material(*model, *mesh) : nullptr;
			if (material && material->get_environment_mode() == Environment_Mode::DYNAMIC)
			{
				auto& resolution = required_resolutions[model->get_id()];
				resolution       = std::max<GLuint>(resolution, material->get_environment_resolution());
			}
		}
	}

	// release the cubemaps that are no longer needed (or whose resolution changed), and add the new ones
	for (auto it = dynamic_env_maps.begin(); it != dynamic_env_maps.end();)
	{
		const auto required = required_resolutions.find(it->first);
		if (required == required_resolutions.end() || required->second != it->second.resolution)
		{
			release_dynamic_env_map(it->second);
			it = dynamic_env_maps.erase(it);
		}
		else
			++it;
	}
	for (const auto& [id, resolution] : required_resolutions)
		if (!dynamic_env_maps.contains(id))
			dynamic_env_maps[id].resolution = resolution;

	if (dynamic_env_maps.empty())
		return; // if no model needs a dynamic environment map, return

	// ensure a skybox cubemap exists for the dynamic env map captures
	auto& skybox_texture = core->get_scene_manager()->get_skybox();
	if (skybox_texture)
		convert_hdr_to_cubemap_if_needed();

	// flip buffers at the start of the update
	// (this makes last frame's "write" buffer -cubemap_tex_id-
	// this frame's "read" buffer -prev_cubemap_tex_id-)
	for (auto& [id, entry] : dynamic_env_maps)
	{
		if (!entry.initialized)
			continue; // a new cubemap has no previous capture yet
		std::swap(entry.cubemap_tex_id, entry.prev_cubemap_tex_id);
		entry.has_prev_cubemap = true; // mark that a valid "previous" cubemap now exists for reading
	}

	// set capturing flag to prevent re-entrance
	is_capturing_dynamic_env_map = true;

	// iterate over the models and capture the dynamic env maps of those that need one
	for (const auto& node : models)
	{
		// dynamically cast the node to a Model object
		auto model = dynamic_cast<Node*>(node.get());
		// if the cast fails or the model has a gizmo, skip to next model
		if (!model || model->get_gizmo_type() != Gizmo_Type::NONE)
			continue;

		// check if the model needs a dynamic env map
		auto it = dynamic_env_maps.find(model->get_id());
		if (it == dynamic_env_maps.end())
			continue; // if not found, skip to next model

		auto& entry = it->second; // get the dynamic env map entry
		// dynamically cast the model to a shared pointer for passing to the capture function
		auto model_ptr = std::dynamic_pointer_cast<Node>(node);
		// capture the dynamic environment map for the model into the "write" buffer (cubemap_tex_id)
		capture_dynamic_env_map_for_model(model_ptr, entry);
	}

	is_capturing_dynamic_env_map = false; // reset capturing flag after processing all models

	//// stop measuring time for performance profiling
	// auto end_time{ std::chrono::high_resolution_clock::now() };

	//// calculate the elapsed time in milliseconds and store it in the elapsed_time variable
	// auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	//// print the elapsed time for the dynamic env map update with a counter to avoid spamming the console
	// static uint32_t render_counter = 0;
	// if ((render_counter++ % 60) == 0) // print every 60 frames
	//	std::cout << "[PROFILER::RENDERER::update_dynamic_env_maps] Dynamic env map update took "
	//	<< elapsed_time << " ms" << std::endl;
}

void Renderer::capture_dynamic_env_map_for_model(const std::shared_ptr<Node>& model, Dynamic_Env_Map_Entry& entry)
{
	// lazy initialization of cubemap texture, FBO, and RBO for the dynamic env map entry
	if (!entry.initialized)
	{
		// current cubemap texture initialization and configuration
		glGenTextures(1, &entry.cubemap_tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, entry.cubemap_tex_id);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB16F,
				entry.resolution,
				entry.resolution,
				0,
				GL_RGB,
				GL_FLOAT,
				nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// previous cubemap texture initialization and configuration (for temporal effects)
		glGenTextures(1, &entry.prev_cubemap_tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, entry.prev_cubemap_tex_id);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB16F,
				entry.resolution,
				entry.resolution,
				0,
				GL_RGB,
				GL_FLOAT,
				nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// FBO and RBO initialization
		glGenFramebuffers(1, &entry.fbo);
		glGenRenderbuffers(1, &entry.rbo);

		entry.initialized      = true;  // mark the entry as initialized
		entry.has_prev_cubemap = false; // no previous cubemap yet (no data captured)
	}

	GLint prev_fbo{}, prev_viewport[4];
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo); // save currently bound FBO
	glGetIntegerv(GL_VIEWPORT, prev_viewport);        // save current viewport

	glBindFramebuffer(GL_FRAMEBUFFER, entry.fbo);                                                     // bind the dynamic env map FBO
	glBindRenderbuffer(GL_RENDERBUFFER, entry.rbo);                                                   // bind the RBO
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, entry.resolution, entry.resolution); // configure RBO storage
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, entry.rbo);       // attach RBO to FBO depth attachment
	glDrawBuffer(GL_COLOR_ATTACHMENT0); // select color attachment 0 for rendering
	// attach the first face of the cubemap before checking the completeness of the FBO (the loop below attaches
	// each face in turn), so that the FBO is checked with the color attachment it renders to
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, entry.cubemap_tex_id, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{ // validate FBO completeness, if incomplete, print error, restore FBO, and return
		std::cerr << "[ERROR::RENDERER::capture_dynamic_env_map_for_model] "
					 "Dynamic Env Map FBO incomplete for model id "
				  << model->get_id() << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo); // restore previous FBO
		return;
	}

	// set up capture projection and views for the 6 cubemap faces
	glm::mat4       capture_proj    = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::vec3 pos             = model->get_world_position(); // capture from the model's position in world space
	glm::mat4       capture_views[] = {
		// 6 view matrices for the 6 faces of the cubemap (right, left, top, bottom, front, back)
		glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +X
		glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
		glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),   // +Y
		glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
		glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +Z
		glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))  // -Z
	};

	glViewport(0, 0, entry.resolution, entry.resolution); // set viewport to capture resolution

	// render the scene from the model's position to each face of the cubemap
	for (GLuint i{}; i < 6; ++i)
	{
		// attach the face of the cubemap texture to the FBO color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, entry.cubemap_tex_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear necessary buffers

		// render the scene for the current capture view, excluding the model itself
		render_scene_for_env_map_capture(capture_views[i], capture_proj, model);
	}

	// generate mipmaps for the cubemap texture to enable trilinear filtering
	glBindTexture(GL_TEXTURE_CUBE_MAP, entry.cubemap_tex_id);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// restore previous FBO and viewport
	glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
	glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
}

void Renderer::render_scene_for_env_map_capture(
	const glm::mat4&             capture_views,
	const glm::mat4&             capture_proj,
	const std::shared_ptr<Node>& exclude_model)
{
	auto  core          = Core::get_instance();      // get the core instance
	auto& node_manager  = core->get_node_manager();  // get the node manager
	auto& scene_manager = core->get_scene_manager(); // get the scene manager

	// inverse of the rotation part of the capture view matrix (used by the shaders that sample environment maps)
	glm::mat3 inv_view_rot = glm::transpose(glm::mat3(capture_views));

	// set the capture view and projection matrices, and the inverse view rotation, in every shader of the shader
	// library (the main pass sets them again with the camera's matrices)
	for (const auto& shader : core->get_shader_library()->get_shaders())
	{
		shader->use();
		shader->set_mat4("u_view", capture_views);
		shader->set_mat4("u_projection", capture_proj);
		shader->set_mat3("u_inv_view_rot", inv_view_rot);
	}

	// the lights are read from the light buffer, uploaded once per frame by upload_lights() in world space,
	// and transformed to view space by the shaders with the capture view matrix

	// render every model of the scene once (in a depth-first traversal from the root models), except the
	// excluded model (the one whose environment map is being captured) and the gizmos
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	std::vector<std::shared_ptr<Node>> stack;
	for (const auto& node : node_manager->get_nodes(Node_Type::MODEL))
		if (node && !node->get_parent())
			stack.push_back(node);
	while (!stack.empty())
	{
		const auto model = stack.back();
		stack.pop_back();
		if (!model || model == exclude_model || model->get_gizmo_type() != Gizmo_Type::NONE)
			continue;

		// visit the children later
		const auto children = model->get_children();
		stack.insert(stack.end(), children.begin(), children.end());

		// render the model's own meshes (if any) with their materials
		draw_model_meshes(*model);
	}

	// render the skybox as background if available
	auto& skybox_texture = scene_manager->get_skybox();
	if (skybox_texture && skybox_texture->get_texture_type() == Texture_Type::CUBEMAP)
	{ // only render if skybox is a cubemap
		if (!skybox_vao)
			init_skybox_cube(); // initialize skybox cube if not done yet
		// render the skybox cube using the skybox shader and the provided view and projection matrices
		// (const_cast is used here to match the function signature)
		render_skybox_cube(skybox_texture, const_cast<glm::mat4&>(capture_views), const_cast<glm::mat4&>(capture_proj));
	}
}
