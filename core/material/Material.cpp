/*
 * Material.cpp
 * This file implements the Material class, which describes how the surface of an object is shaded: the shader
 * program that draws it and the values of the shader's material parameters (e.g., the shininess). Materials
 * are assets, shared by every object that uses them, and objects can override some parameters for themselves.
 */

#include "Material.h"

#include "../shader/Shader.h"

#include <type_traits>
#include <utility>

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

// Public Methods
// --------------
void Material::apply() const
{
	apply(parameters);
}

void Material::apply(const Material_Parameters& parameters) const
{
	if (!shader)
		return;

	for (const auto& [parameter_name, value] : parameters)
	{
		// set the uniform with the setter of the value's type
		const std::string uniform_name = UNIFORM_PREFIX + parameter_name;
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
}
