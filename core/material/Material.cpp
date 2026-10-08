/*
 * Material.cpp
 * This file implements the Material class, which describes how the surface of an object is shaded: the shader
 * program that draws it, the values of the shader's material parameters (e.g., the shininess), and the textures
 * of its texture slots (e.g., the albedo map). Materials are assets, shared by every object that uses them,
 * and objects can override some parameters for themselves. A material loaded from a descriptor file (or saved
 * to one) remembers its file, so that it can be saved again or reloaded (see Material_Library).
 */

#include "Material.h"

#include "../shader/Shader.h"
#include "../texture/Texture.h"
#include "../utils/string/String_Utils.h"

#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace
{
	// Returns the zero value of a material parameter of the given OpenGL type, or an empty optional if a material
	// parameter cannot have that type (it is not a float, a boolean, or a vector of 2, 3, or 4 floats)
	std::optional<Material_Value> get_zero_value(GLenum type)
	{
		switch (type)
		{
			case GL_FLOAT: return Material_Value{ 0.0f };
			case GL_BOOL: return Material_Value{ false };
			case GL_FLOAT_VEC2: return Material_Value{ glm::vec2{ 0.0f } };
			case GL_FLOAT_VEC3: return Material_Value{ glm::vec3{ 0.0f } };
			case GL_FLOAT_VEC4: return Material_Value{ glm::vec4{ 0.0f } };
			default: return std::nullopt;
		}
	}

	// Returns the name of the material parameter that a uniform sets (its member of the u_material struct, e.g.,
	// "shininess" for "u_material.shininess"), or an empty string if the uniform does not set a material parameter
	std::string get_parameter_name(const std::string& uniform_name, const std::vector<std::string>& texture_slots)
	{
		const std::string_view prefix{ Material::UNIFORM_PREFIX };
		if (!uniform_name.starts_with(prefix))
			return {};
		// only the members of the struct (not the elements of an array member, nor the members of a nested struct)
		std::string parameter_name = uniform_name.substr(prefix.size());
		if (!String_Utils::is_glsl_identifier(parameter_name))
			return {};
		// the samplers of the texture slots and their has_<slot> booleans are set from the textures of the material
		for (const auto& slot : texture_slots)
			if (parameter_name == slot || parameter_name == "has_" + slot)
				return {};
		return parameter_name;
	}
}

// Constructors
// ------------
Material::Material(
	const std::string&         name,
	std::shared_ptr<Shader>    shader,
	const Material_Parameters& parameters,
	bool                       supports_instancing) :
	name{ name },
	shader{ std::move(shader) },
	parameters{ parameters },
	supports_instancing{ supports_instancing }
{}

// Public Static Methods
// ---------------------
Material_Parameters
Material::get_shader_parameters(const std::map<std::string, unsigned int>& uniforms, const std::vector<std::string>& texture_slots)
{
	Material_Parameters shader_parameters;
	for (const auto& [uniform_name, type] : uniforms)
	{
		const std::string parameter_name = get_parameter_name(uniform_name, texture_slots);
		const auto        zero_value     = get_zero_value(type);
		if (!parameter_name.empty() && zero_value)
			shader_parameters[parameter_name] = *zero_value;
	}
	return shader_parameters;
}

Material_Parameters Material::get_shader_parameters(const Shader& shader)
{
	return get_shader_parameters(shader.get_uniforms(), shader.get_texture_slots());
}

// Public Methods
// --------------
void Material::apply() const
{
	apply(parameters);
	if (!shader)
		return;
	const auto& slots = shader->get_texture_slots();

	// set the parameters of the shader that the material does not have to their zero values
	for (const auto& [uniform_name, type] : shader->get_uniforms())
	{
		const std::string parameter_name = get_parameter_name(uniform_name, slots);
		if (parameter_name.empty() || parameters.contains(parameter_name))
			continue;
		if (const auto zero_value = get_zero_value(type))
			set_uniform(uniform_name, *zero_value);
	}

	// bind the texture of each texture slot of the shader to the unit of the slot (or no texture, if the material
	// has none for it), and tell the shader which slots have a texture
	for (GLuint unit = 0; unit < slots.size(); ++unit)
	{
		const std::string& slot        = slots[unit];
		const auto         it          = textures.find(slot);
		const bool         has_texture = it != textures.end() && it->second;
		if (has_texture)
			it->second->bind(unit);
		else
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		shader->set_int(UNIFORM_PREFIX + slot, static_cast<GLint>(unit));
		shader->set_bool(std::string(UNIFORM_PREFIX) + "has_" + slot, has_texture);
	}
	glActiveTexture(GL_TEXTURE0); // leave the first unit active, as the rest of the renderer expects
}

void Material::apply(const Material_Parameters& parameters) const
{
	if (!shader)
		return;

	for (const auto& [parameter_name, value] : parameters) set_uniform(UNIFORM_PREFIX + parameter_name, value);
}

// Private Methods
// ---------------
void Material::set_uniform(const std::string& uniform_name, const Material_Value& value) const
{
	std::visit(
		[this, &uniform_name](const auto& typed_value) {
			using Type = std::decay_t<decltype(typed_value)>;
			if constexpr (std::is_same_v<Type, float>)
				shader->set_float(uniform_name, typed_value);
			else if constexpr (std::is_same_v<Type, bool>)
				shader->set_bool(uniform_name, typed_value);
			else if constexpr (std::is_same_v<Type, glm::vec2>)
				shader->set_vec2(uniform_name, typed_value);
			else if constexpr (std::is_same_v<Type, glm::vec3>)
				shader->set_vec3(uniform_name, typed_value);
			else if constexpr (std::is_same_v<Type, glm::vec4>)
				shader->set_vec4(uniform_name, typed_value);
		},
		value);
}
