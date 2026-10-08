/*
 * Material.h
 * This file defines the Material class, which describes how the surface of an object is shaded: the shader
 * program that draws it, the values of the shader's material parameters (e.g., the shininess), and the textures
 * of its texture slots (e.g., the albedo map). Materials are assets, shared by every object that uses them,
 * and objects can override some parameters for themselves.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>

#include <glm/glm.hpp>

class Shader;
class Texture;

// Value of a material parameter, set as a uniform of the shader (float, bool, or vector of floats)
using Material_Value = std::variant<float, bool, glm::vec2, glm::vec3, glm::vec4>;

// Material parameters by name (the name of the uniform without its "u_material." prefix, e.g., "shininess")
using Material_Parameters = std::map<std::string, Material_Value>;

// Material textures by the name of their texture slot (e.g., "albedo_map", see Shader::get_texture_slots)
using Material_Textures = std::map<std::string, std::shared_ptr<Texture>>;

// Environment map sampled by a material's shader (as the cubemap u_environment_map, e.g., for reflections)
enum class Environment_Mode
{
	NONE,   // the material does not sample an environment map
	SKYBOX, // the skybox of the scene (cheap, but without the objects of the scene)
	DYNAMIC // a cubemap captured every frame from the position of each object (expensive: 6 scene renders)
};

class Material
{
public:
	// Public Constants
	// ----------------
	// prefix of the uniforms that receive the material parameters (the members of the shader's u_material struct)
	static constexpr const char* UNIFORM_PREFIX{ "u_material." };
	// default, minimum, and maximum resolution of the faces of the dynamic environment cubemaps
	static constexpr unsigned int DEFAULT_ENVIRONMENT_RESOLUTION{ 512 };
	static constexpr unsigned int MIN_ENVIRONMENT_RESOLUTION{ 16 };
	static constexpr unsigned int MAX_ENVIRONMENT_RESOLUTION{ 4096 };

	// Constructors
	// ------------
	// Creates a material with a name, the shader that draws it, its parameters, and whether its shader supports
	// instanced rendering (drawing many objects with one draw call, with per-instance model matrices and colors)
	Material(
		const std::string&         name,
		std::shared_ptr<Shader>    shader,
		const Material_Parameters& parameters          = {},
		bool                       supports_instancing = false);

	// Public Methods
	// --------------
	// Sets every parameter of the material as a uniform of its shader (which must be in use), and binds the textures
	// of the shader's texture slots: the slot i uses the texture unit i (set as u_material.<slot>), and the boolean
	// u_material.has_<slot> tells the shader whether the material has a texture for it
	void apply() const;
	// Sets the given parameters as uniforms of the material's shader (which must be in use), e.g., the parameters
	// that an object overrides, after applying the material
	void apply(const Material_Parameters& parameters) const;

	// Getters
	const std::string&             get_name() const { return name; }
	Environment_Mode               get_environment_mode() const { return environment_mode; }
	unsigned int                   get_environment_resolution() const { return environment_resolution; }
	const std::shared_ptr<Shader>& get_shader() const { return shader; }
	const Material_Parameters&     get_parameters() const { return parameters; }
	const Material_Textures&       get_textures() const { return textures; }
	bool                           get_supports_instancing() const { return supports_instancing; }

	// Setters
	void set_name(const std::string& name) { this->name = name; }
	void set_parameter(const std::string& parameter_name, const Material_Value& value) { parameters[parameter_name] = value; }
	void set_texture(const std::string& slot, const std::shared_ptr<Texture>& texture) { textures[slot] = texture; }
	// sets the environment map of the material, and the resolution of the faces of its dynamic cubemaps. Returns false
	// (keeping the current values) if the resolution is outside [MIN_ENVIRONMENT_RESOLUTION, MAX_ENVIRONMENT_RESOLUTION]
	bool set_environment(Environment_Mode mode, unsigned int resolution = DEFAULT_ENVIRONMENT_RESOLUTION)
	{
		if (resolution < MIN_ENVIRONMENT_RESOLUTION || resolution > MAX_ENVIRONMENT_RESOLUTION)
			return false;
		environment_mode       = mode;
		environment_resolution = resolution;
		return true;
	}

private:
	// Private Attributes
	// ------------------
	std::string             name;
	std::shared_ptr<Shader> shader;                                                   // shader program that draws the material
	Material_Parameters     parameters;                                               // values of the material parameters
	Material_Textures       textures;                                                 // textures of the material, by texture slot
	bool                    supports_instancing;                                      // whether the shader supports instanced rendering
	Environment_Mode        environment_mode{ Environment_Mode::NONE };               // environment map of the shader
	unsigned int            environment_resolution{ DEFAULT_ENVIRONMENT_RESOLUTION }; // resolution of dynamic cubemaps
};
