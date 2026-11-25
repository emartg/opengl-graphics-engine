/*
 * Node_Factory.cpp
 * This file implements the Node_Factory class, which reads the descriptors of the shape and light assets and builds the
 * nodes that they describe, converting their lengths from metres to the units of the scene.
 */

#include "Node_Factory.h"

#include "../light/Directional_Light.h"
#include "../light/Point_Light.h"
#include "../light/Spotlight.h"
#include "../model/CUBE.h"
#include "../model/PLANE.h"
#include "../utils/string/String_Utils.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <tuple>
#include <utility>

#include <nlohmann/json.hpp>

namespace
{
	// Reads an array of numbers of one of the given lengths into a vector of 4 components (the others are 0). Returns
	// false if the value is not such an array or a number is not finite
	bool read_numbers(const nlohmann::json& value, std::initializer_list<std::size_t> lengths, glm::vec4& numbers, std::size_t& length)
	{
		if (!value.is_array() || std::find(lengths.begin(), lengths.end(), value.size()) == lengths.end())
			return false;
		numbers = glm::vec4{ 0.0f };
		for (std::size_t i{}; i < value.size(); ++i)
		{
			if (!value[i].is_number() || !std::isfinite(value[i].get<float>()))
				return false;
			numbers[static_cast<glm::length_t>(i)] = value[i].get<float>();
		}
		length = value.size();
		return true;
	}

	// Reads a number that must be finite and greater than 0. Returns false otherwise
	bool read_positive_number(const nlohmann::json& value, GLfloat& number)
	{
		if (!value.is_number() || !std::isfinite(value.get<float>()) || value.get<float>() <= 0.0f)
			return false;
		number = value.get<float>();
		return true;
	}

	// Reads the required "name" field of a descriptor. Returns false (and an error message) if it is missing or not a
	// non-empty string
	bool read_name(const nlohmann::json& json, std::string& name, std::string& error)
	{
		if (!json.contains("name") || !json["name"].is_string() || json["name"].get<std::string>().empty())
		{
			error = "The field \"name\" is required and must be a non-empty string";
			return false;
		}
		name = json["name"].get<std::string>();
		return true;
	}

	// Reads a whole text file. Returns false if it cannot be opened
	bool read_text_file(const std::filesystem::path& file, std::string& text)
	{
		std::ifstream stream{ file };
		if (!stream)
			return false;
		std::stringstream buffer;
		buffer << stream.rdbuf();
		text = buffer.str();
		return true;
	}

	// Reads and parses a descriptor file with the given parser, adding the name of the file to the error message
	template<typename Descriptor, typename Parser>
	std::optional<Descriptor> load_descriptor(const std::filesystem::path& file, std::string& error, Parser parse)
	{
		std::string text;
		if (!read_text_file(file, text))
		{
			error = "Cannot read the file \"" + file.string() + "\"";
			return std::nullopt;
		}
		auto descriptor = parse(text, error);
		if (!descriptor)
			error = "\"" + file.string() + "\": " + error;
		return descriptor;
	}

	// Size of the built-in shapes, in units of their vertex data (both are 2 units across, from -1 to 1)
	constexpr GLfloat PRIMITIVE_SIZE{ 2.0f };
}

// Public Static Methods
// ---------------------
std::optional<Shape_Descriptor> Node_Factory::parse_shape_descriptor(const std::string& text, std::string& error)
{
	// parse the text without exceptions (a discarded value means that the text is not valid JSON)
	const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
	if (json.is_discarded() || !json.is_object())
	{
		error = "The descriptor is not a valid JSON object";
		return std::nullopt;
	}

	Shape_Descriptor descriptor;
	if (!read_name(json, descriptor.name, error))
		return std::nullopt;

	// required field: the built-in shape
	const std::string shape = json.contains("shape") && json["shape"].is_string() ? json["shape"].get<std::string>() : "";
	if (shape == "cube")
		descriptor.primitive = Shape_Primitive::CUBE;
	else if (shape == "plane")
		descriptor.primitive = Shape_Primitive::PLANE;
	else
	{
		error = "The field \"shape\" is required and must be \"cube\" or \"plane\"";
		return std::nullopt;
	}
	const bool is_plane = descriptor.primitive == Shape_Primitive::PLANE;

	// optional field: the size, in metres (the width, height, and depth of a cube; the width and depth of a plane)
	if (json.contains("size"))
	{
		glm::vec4   numbers;
		std::size_t length{};
		if (!read_numbers(json["size"], { is_plane ? std::size_t{ 2 } : std::size_t{ 3 } }, numbers, length) || numbers.x <= 0.0f ||
			numbers.y <= 0.0f || (!is_plane && numbers.z <= 0.0f))
		{
			error = is_plane ? "The field \"size\" of a plane must be an array of 2 numbers greater than 0 (its width and depth)"
							 : "The field \"size\" of a cube must be an array of 3 numbers greater than 0 (its width, height, and depth)";
			return std::nullopt;
		}
		descriptor.size = is_plane ? glm::vec3{ numbers.x, 0.0f, numbers.y } : glm::vec3{ numbers };
	}

	// optional field: the color (RGB, or RGBA whose alpha is the opacity), each component from 0 to 1
	if (json.contains("color"))
	{
		glm::vec4   numbers;
		std::size_t length{};
		if (!read_numbers(json["color"], { 3, 4 }, numbers, length) || glm::any(glm::lessThan(numbers, glm::vec4{ 0.0f })) ||
			glm::any(glm::greaterThan(numbers, glm::vec4{ 1.0f })))
		{
			error = "The field \"color\" must be an array of 3 or 4 numbers from 0 to 1";
			return std::nullopt;
		}
		descriptor.color = glm::vec4{ glm::vec3{ numbers }, length == 4 ? numbers.w : 1.0f };
	}

	// optional field: whether both faces are drawn (a plane is seen from both sides unless it says otherwise)
	descriptor.is_two_sided = is_plane;
	if (json.contains("two_sided"))
	{
		if (!json["two_sided"].is_boolean())
		{
			error = "The field \"two_sided\" must be a boolean";
			return std::nullopt;
		}
		descriptor.is_two_sided = json["two_sided"].get<bool>();
	}

	return descriptor;
}

std::optional<Light_Descriptor> Node_Factory::parse_light_descriptor(const std::string& text, std::string& error)
{
	// parse the text without exceptions (a discarded value means that the text is not valid JSON)
	const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
	if (json.is_discarded() || !json.is_object())
	{
		error = "The descriptor is not a valid JSON object";
		return std::nullopt;
	}

	Light_Descriptor descriptor;
	if (!read_name(json, descriptor.name, error))
		return std::nullopt;

	// required field: the type of light
	const std::string type = json.contains("type") && json["type"].is_string() ? json["type"].get<std::string>() : "";
	if (type == "directional")
		descriptor.type = Light_Type::DIRECTIONAL_LIGHT;
	else if (type == "point")
		descriptor.type = Light_Type::POINT_LIGHT;
	else if (type == "spot")
		descriptor.type = Light_Type::SPOTLIGHT;
	else
	{
		error = "The field \"type\" is required and must be \"directional\", \"point\", or \"spot\"";
		return std::nullopt;
	}
	const bool has_direction = descriptor.type != Light_Type::POINT_LIGHT;
	const bool has_range     = descriptor.type != Light_Type::DIRECTIONAL_LIGHT;
	const bool is_spotlight  = descriptor.type == Light_Type::SPOTLIGHT;

	// a field that the type of light does not have is a mistake in the descriptor, so it is refused rather than ignored
	for (const auto& [field, applies, kinds] : { std::tuple{ "direction", has_direction, "directional lights and spotlights" },
												 std::tuple{ "range", has_range, "point lights and spotlights" },
												 std::tuple{ "inner_cutoff", is_spotlight, "spotlights" },
												 std::tuple{ "outer_cutoff", is_spotlight, "spotlights" } })
	{
		if (json.contains(field) && !applies)
		{
			error = std::string("The field \"") + field + "\" is only for " + kinds;
			return std::nullopt;
		}
	}

	// optional fields: the colors, each an array of 3 numbers that are not negative (they may exceed 1 for a bright light)
	for (const auto& [field, color] : { std::pair{ "color", &descriptor.diffuse },
										std::pair{ "ambient", &descriptor.ambient },
										std::pair{ "specular", &descriptor.specular } })
	{
		if (!json.contains(field))
			continue;
		glm::vec4   numbers;
		std::size_t length{};
		if (!read_numbers(json[field], { 3 }, numbers, length) || glm::any(glm::lessThan(numbers, glm::vec4{ 0.0f })))
		{
			error = std::string("The field \"") + field + "\" must be an array of 3 numbers that are not negative";
			return std::nullopt;
		}
		*color = glm::vec3{ numbers };
	}

	// optional field: the direction (any length but 0; it is normalized)
	if (json.contains("direction"))
	{
		glm::vec4   numbers;
		std::size_t length{};
		if (!read_numbers(json["direction"], { 3 }, numbers, length) || glm::length(glm::vec3{ numbers }) == 0.0f)
		{
			error = "The field \"direction\" must be an array of 3 numbers that are not all 0";
			return std::nullopt;
		}
		descriptor.direction = glm::normalize(glm::vec3{ numbers });
	}

	// optional fields: the range and the size of the gizmo, in metres
	for (const auto& [field, value] : { std::pair{ "range", &descriptor.range }, std::pair{ "gizmo_size", &descriptor.gizmo_size } })
	{
		if (!json.contains(field))
			continue;
		GLfloat number{};
		if (!read_positive_number(json[field], number))
		{
			error = std::string("The field \"") + field + "\" must be a number greater than 0 (in metres)";
			return std::nullopt;
		}
		*value = number;
	}

	// optional fields: the cut-off angles of a spotlight, in degrees (the inner one is not larger than the outer one)
	for (const auto& [field, angle] :
		 { std::pair{ "inner_cutoff", &descriptor.inner_cutoff }, std::pair{ "outer_cutoff", &descriptor.outer_cutoff } })
	{
		if (!json.contains(field))
			continue;
		GLfloat number{};
		if (!read_positive_number(json[field], number) || number > MAX_CUTOFF_DEGREES)
		{
			error = std::string("The field \"") + field + "\" must be an angle greater than 0 and at most " +
				std::to_string(static_cast<int>(MAX_CUTOFF_DEGREES)) + " degrees";
			return std::nullopt;
		}
		*angle = number;
	}
	if (descriptor.inner_cutoff > descriptor.outer_cutoff)
	{
		error = "The field \"inner_cutoff\" must not be larger than \"outer_cutoff\"";
		return std::nullopt;
	}

	return descriptor;
}

std::optional<Shape_Descriptor> Node_Factory::load_shape_descriptor(const std::filesystem::path& file, std::string& error)
{
	return load_descriptor<Shape_Descriptor>(file, error, parse_shape_descriptor);
}

std::optional<Light_Descriptor> Node_Factory::load_light_descriptor(const std::filesystem::path& file, std::string& error)
{
	return load_descriptor<Light_Descriptor>(file, error, parse_light_descriptor);
}

glm::vec3 Node_Factory::get_shape_scale(const Shape_Descriptor& descriptor, GLfloat units_per_metre)
{
	if (!descriptor.size)
		return glm::vec3{ 1.0f }; // the built-in shape as it is, as the Creation window of the App made it

	glm::vec3 scale = *descriptor.size * units_per_metre / PRIMITIVE_SIZE;
	// a plane has no height: its vertical scale stays 1 (it does not change the shape)
	if (descriptor.primitive == Shape_Primitive::PLANE)
		scale.y = 1.0f;
	return scale;
}

std::shared_ptr<Shape_Model>
Node_Factory::create_shape(const Shape_Descriptor& descriptor, const glm::vec3& position, GLfloat units_per_metre)
{
	const bool is_plane = descriptor.primitive == Shape_Primitive::PLANE;
	auto       shape    = std::make_shared<Shape_Model>(
		descriptor.name,
		is_plane ? plane_vertices_vector : cube_vertices_vector,
		is_plane ? plane_indices_vector : cube_indices_vector,
		descriptor.color,
		position,
		glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f },
		get_shape_scale(descriptor, units_per_metre));
	shape->set_is_two_sided(descriptor.is_two_sided);

	// the identifier of the node is part of its name, as for every node that the user creates
	shape->set_name(String_Utils::generate_id_prefixed_name(shape));
	return shape;
}

std::shared_ptr<Light> Node_Factory::create_light(const Light_Descriptor& descriptor, const glm::vec3& position, GLfloat units_per_metre)
{
	std::shared_ptr<Light> light;
	switch (descriptor.type)
	{
		case Light_Type::DIRECTIONAL_LIGHT:
		{
			auto directional_light =
				std::make_shared<Directional_Light>(descriptor.name, descriptor.ambient, descriptor.diffuse, descriptor.specular, position);
			if (descriptor.direction)
				directional_light->set_direction_only(*descriptor.direction);
			light = directional_light;
			break;
		}
		case Light_Type::SPOTLIGHT:
		{
			auto spotlight =
				std::make_shared<Spotlight>(descriptor.name, descriptor.ambient, descriptor.diffuse, descriptor.specular, position);
			if (descriptor.direction)
				spotlight->set_direction_only(*descriptor.direction);
			// the spotlight keeps the cosines of its cut-off angles
			spotlight->set_inner_cutoff(glm::cos(glm::radians(descriptor.inner_cutoff)));
			spotlight->set_outer_cutoff(glm::cos(glm::radians(descriptor.outer_cutoff)));
			light = spotlight;
			break;
		}
		default:
			light = std::make_shared<Point_Light>(descriptor.name, descriptor.ambient, descriptor.diffuse, descriptor.specular, position);
			break;
	}

	// the attenuation: the range of the descriptor, or the engine's default terms (per metre) in the units of the scene
	auto set_attenuation = [&descriptor, units_per_metre](auto& attenuated_light) {
		if (descriptor.range)
			attenuated_light.set_range(*descriptor.range * units_per_metre);
		else if (units_per_metre != 1.0f)
		{
			attenuated_light.set_linear(attenuated_light.get_linear() / units_per_metre);
			attenuated_light.set_quadratic(attenuated_light.get_quadratic() / (units_per_metre * units_per_metre));
		}
	};
	if (auto point_light = std::dynamic_pointer_cast<Point_Light>(light))
		set_attenuation(*point_light);
	else if (auto spotlight = std::dynamic_pointer_cast<Spotlight>(light))
		set_attenuation(*spotlight);

	// the size of the gizmo: the descriptor's, or the default of the kind of light (which is in metres), in scene units
	if (descriptor.gizmo_size)
		light->set_gizmo_scale(glm::vec3{ *descriptor.gizmo_size * units_per_metre });
	else if (units_per_metre != 1.0f)
		light->set_gizmo_scale(light->get_gizmo_scale() * units_per_metre);

	// the identifier of the node is part of its name (and of its gizmo's, which is named after it when it is created)
	light->set_name(String_Utils::generate_id_prefixed_name(light));
	light->create_gizmo();
	return light;
}
