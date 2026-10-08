/*
 * Material_Library.cpp
 * This file implements the Material_Library class, which manages the materials of the engine as assets: each
 * material is described by a descriptor file (<name>.material.json) with its name, the name of its shader
 * (from the shader library), the values of its parameters, and its textures, so that materials are added or
 * changed without modifying the engine's code. The library loads every descriptor of a directory, holds the
 * materials created by the engine (e.g., those of imported models), and finds the materials by name. It also
 * saves materials to descriptor files and reloads them, duplicates them, and renames them (e.g., for an editor).
 */

#include "Material_Library.h"

#include "../shader/Shader.h"
#include "../shader/Shader_Library.h"
#include "../texture/Texture.h"
#include "../utils/string/String_Utils.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <variant>

#include <nlohmann/json.hpp>

namespace
{
	// Converts a JSON value to a material parameter value: a number is a float, a boolean is a bool, and an array
	// of 2, 3, or 4 numbers is a vector. Returns an empty optional if the value has another type
	std::optional<Material_Value> to_material_value(const nlohmann::json& value)
	{
		if (value.is_boolean())
			return Material_Value{ value.get<bool>() };
		if (value.is_number())
			return Material_Value{ value.get<float>() };
		if (value.is_array() && value.size() >= 2 && value.size() <= 4 &&
			std::all_of(value.begin(), value.end(), [](const nlohmann::json& element) { return element.is_number(); }))
		{
			const auto component = [&value](std::size_t i) {
				return value[i].get<float>();
			};
			switch (value.size())
			{
				case 2: return Material_Value{ glm::vec2{ component(0), component(1) } };
				case 3: return Material_Value{ glm::vec3{ component(0), component(1), component(2) } };
				default: return Material_Value{ glm::vec4{ component(0), component(1), component(2), component(3) } };
			}
		}
		return std::nullopt;
	}

	// Converts a float to the double with the shortest decimal representation that reads back as the same float, so
	// that the descriptors are written with the values as they were typed (e.g., 0.6579 instead of 0.657899975776672)
	double to_json_number(float value)
	{
		char       buffer[64];
		const auto written = std::to_chars(buffer, buffer + sizeof(buffer), value);
		double     number  = static_cast<double>(value);
		if (written.ec == std::errc{})
			std::from_chars(buffer, written.ptr, number);
		return number;
	}

	// Converts a material parameter value to a JSON value (the inverse of to_material_value)
	nlohmann::ordered_json to_json_value(const Material_Value& value)
	{
		return std::visit(
			[](const auto& typed_value) -> nlohmann::ordered_json {
				using Type = std::decay_t<decltype(typed_value)>;
				if constexpr (std::is_same_v<Type, bool>)
					return typed_value;
				else if constexpr (std::is_same_v<Type, float>)
					return to_json_number(typed_value);
				else
				{ // a vector, as an array of its components
					nlohmann::ordered_json array = nlohmann::ordered_json::array();
					for (glm::length_t i = 0; i < Type::length(); ++i) array.push_back(to_json_number(typed_value[i]));
					return array;
				}
			},
			value);
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
}

// Public Static Methods
// ---------------------
std::optional<Material_Descriptor>
Material_Library::parse_descriptor(const std::string& text, const std::filesystem::path& base_dir, std::string& error)
{
	// parse the text without exceptions (a discarded value means that the text is not valid JSON)
	const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
	if (json.is_discarded() || !json.is_object())
	{
		error = "The descriptor is not a valid JSON object";
		return std::nullopt;
	}

	Material_Descriptor descriptor;

	// required fields: the name of the material and the name of its shader
	for (const auto& [field, value] : { std::pair{ "name", &descriptor.name }, std::pair{ "shader", &descriptor.shader_name } })
	{
		if (!json.contains(field))
		{
			error = std::string("Missing required field \"") + field + "\"";
			return std::nullopt;
		}
		if (!json[field].is_string() || json[field].get<std::string>().empty())
		{
			error = std::string("The field \"") + field + "\" must be a non-empty string";
			return std::nullopt;
		}
		*value = json[field].get<std::string>();
	}

	// optional field: whether the shader supports instanced rendering
	if (json.contains("instancing"))
	{
		if (!json["instancing"].is_boolean())
		{
			error = "The field \"instancing\" must be a boolean";
			return std::nullopt;
		}
		descriptor.supports_instancing = json["instancing"].get<bool>();
	}

	// optional field: the values of the material parameters
	if (json.contains("parameters"))
	{
		if (!json["parameters"].is_object())
		{
			error = "The field \"parameters\" must be an object";
			return std::nullopt;
		}
		for (const auto& [parameter_name, json_value] : json["parameters"].items())
		{
			// the parameters become members of the shader's u_material struct, so their names must be GLSL identifiers
			if (!String_Utils::is_glsl_identifier(parameter_name))
			{
				error = "The parameter name \"" + parameter_name + "\" is not a valid GLSL identifier";
				return std::nullopt;
			}
			const auto value = to_material_value(json_value);
			if (!value)
			{
				error = "The parameter \"" + parameter_name + "\" must be a number, a boolean, or an array of 2 to 4 numbers";
				return std::nullopt;
			}
			descriptor.parameters[parameter_name] = *value;
		}
	}

	// optional fields: the environment map sampled by the shader, and the resolution of the dynamic cubemaps
	if (json.contains("environment"))
	{
		const std::string mode = json["environment"].is_string() ? json["environment"].get<std::string>() : "";
		if (mode == "skybox")
			descriptor.environment_mode = Environment_Mode::SKYBOX;
		else if (mode == "dynamic")
			descriptor.environment_mode = Environment_Mode::DYNAMIC;
		else
		{
			error = "The field \"environment\" must be \"skybox\" or \"dynamic\"";
			return std::nullopt;
		}
	}
	if (json.contains("environment_resolution"))
	{
		const auto& resolution = json["environment_resolution"];
		if (!resolution.is_number_integer() || resolution.get<long long>() < Material::MIN_ENVIRONMENT_RESOLUTION ||
			resolution.get<long long>() > Material::MAX_ENVIRONMENT_RESOLUTION)
		{
			error = "The field \"environment_resolution\" must be an integer between " +
				std::to_string(Material::MIN_ENVIRONMENT_RESOLUTION) + " and " + std::to_string(Material::MAX_ENVIRONMENT_RESOLUTION);
			return std::nullopt;
		}
		descriptor.environment_resolution = resolution.get<unsigned int>();
	}

	// optional field: the image files of the textures, by texture slot (relative to the descriptor)
	if (json.contains("textures"))
	{
		if (!json["textures"].is_object())
		{
			error = "The field \"textures\" must be an object";
			return std::nullopt;
		}
		for (const auto& [slot, json_path] : json["textures"].items())
		{
			if (!String_Utils::is_glsl_identifier(slot))
			{
				error = "The texture slot \"" + slot + "\" is not a valid GLSL identifier";
				return std::nullopt;
			}
			if (!json_path.is_string() || json_path.get<std::string>().empty())
			{
				error = "The texture \"" + slot + "\" must be a non-empty string (the path of an image file)";
				return std::nullopt;
			}
			descriptor.texture_paths[slot] = base_dir / json_path.get<std::string>();
		}
	}

	return descriptor;
}

std::vector<std::filesystem::path> Material_Library::find_descriptor_files(const std::filesystem::path& dir)
{
	std::vector<std::filesystem::path> files;
	std::error_code                    error; // an inaccessible directory has no descriptors
	for (const auto& entry : std::filesystem::directory_iterator(dir, error))
	{
		const std::string file_name = entry.path().filename().string();
		if (entry.is_regular_file(error) && file_name.ends_with(DESCRIPTOR_EXTENSION))
			files.push_back(entry.path());
	}
	// sort the files, since the order of a directory iteration is unspecified
	std::sort(files.begin(), files.end());
	return files;
}

std::optional<Material_Descriptor> Material_Library::to_descriptor(const Material& material, std::string& error)
{
	if (!material.get_shader())
	{
		error = "The material has no shader";
		return std::nullopt;
	}

	Material_Descriptor descriptor;
	descriptor.name                   = material.get_name();
	descriptor.shader_name            = material.get_shader()->get_name();
	descriptor.parameters             = material.get_parameters();
	descriptor.supports_instancing    = material.get_supports_instancing();
	descriptor.environment_mode       = material.get_environment_mode();
	descriptor.environment_resolution = material.get_environment_resolution();
	for (const auto& [slot, texture] : material.get_textures())
	{
		if (!texture)
			continue; // a slot without a texture
		if (texture->get_texture_path().empty())
		{
			error = "The texture of the slot \"" + slot + "\" has no image file";
			return std::nullopt;
		}
		descriptor.texture_paths[slot] = texture->get_texture_path();
	}
	return descriptor;
}

std::string Material_Library::write_descriptor(const Material_Descriptor& descriptor, const std::filesystem::path& base_dir)
{
	// the fields are written in the order of the engine's descriptors (an ordered JSON object keeps their order)
	nlohmann::ordered_json json;
	json["name"]   = descriptor.name;
	json["shader"] = descriptor.shader_name;
	if (descriptor.supports_instancing)
		json["instancing"] = true;
	if (descriptor.environment_mode != Environment_Mode::NONE)
		json["environment"] = descriptor.environment_mode == Environment_Mode::SKYBOX ? "skybox" : "dynamic";
	if (descriptor.environment_mode == Environment_Mode::DYNAMIC ||
		descriptor.environment_resolution != Material::DEFAULT_ENVIRONMENT_RESOLUTION)
		json["environment_resolution"] = descriptor.environment_resolution;
	if (!descriptor.parameters.empty())
	{
		json["parameters"] = nlohmann::ordered_json::object();
		for (const auto& [parameter_name, value] : descriptor.parameters) json["parameters"][parameter_name] = to_json_value(value);
	}
	if (!descriptor.texture_paths.empty())
	{
		json["textures"] = nlohmann::ordered_json::object();
		for (const auto& [slot, path] : descriptor.texture_paths)
		{
			// a path relative to the descriptor (with forward slashes, on every platform), or an absolute path
			std::error_code             error;
			const std::filesystem::path relative_path = std::filesystem::relative(path, base_dir, error);
			json["textures"][slot] =
				!error && !relative_path.empty() ? relative_path.generic_string() : std::filesystem::absolute(path, error).generic_string();
		}
	}
	// invalid UTF-8 (e.g., in the name of a material imported from an old model file) is replaced with U+FFFD, since
	// JSON text must be valid UTF-8 (the default handler would throw an exception)
	return json.dump(2, ' ', false, nlohmann::ordered_json::error_handler_t::replace) + "\n";
}

std::string Material_Library::make_descriptor_file_name(const std::string& material_name)
{
	std::string stem;
	for (const char character : material_name)
	{
		const auto code = static_cast<unsigned char>(character);
		if (code < 128 && std::isalnum(code))
			stem += static_cast<char>(std::tolower(code));
		else if (!stem.empty() && stem.back() != '_')
			stem += '_'; // a run of other characters (e.g., spaces) becomes one underscore
	}
	if (!stem.empty() && stem.back() == '_')
		stem.pop_back();
	if (stem.empty())
		stem = "material";
	return stem + DESCRIPTOR_EXTENSION;
}

std::filesystem::path Material_Library::make_new_descriptor_path(const std::string& material_name, const std::filesystem::path& dir)
{
	const std::string     file_name = make_descriptor_file_name(material_name);
	const std::string     stem      = file_name.substr(0, file_name.size() - std::string_view{ DESCRIPTOR_EXTENSION }.size());
	std::filesystem::path path      = dir / file_name;
	std::error_code       error;
	for (int suffix = 2; std::filesystem::exists(path, error); ++suffix)
		path = dir / (stem + "_" + std::to_string(suffix) + DESCRIPTOR_EXTENSION);
	return path;
}

std::shared_ptr<Material>
Material_Library::load_file(const std::filesystem::path& file, const Shader_Library& shader_library, std::string& error)
{
	// read and parse the descriptor
	std::string text;
	if (!read_text_file(file, text))
	{
		error = "The file cannot be read";
		return nullptr;
	}
	const auto descriptor = parse_descriptor(text, file.parent_path(), error);
	if (!descriptor)
		return nullptr;

	// find the shader of the material
	auto shader = shader_library.get(descriptor->shader_name);
	if (!shader)
	{
		error =
			"The material '" + descriptor->name + "' uses the shader '" + descriptor->shader_name + "', which is not in the shader library";
		return nullptr;
	}

	// create the material, with its textures (only for the slots of its shader)
	auto material = std::make_shared<Material>(descriptor->name, shader, descriptor->parameters, descriptor->supports_instancing);
	material->set_environment(descriptor->environment_mode, descriptor->environment_resolution);
	material->set_file_path(file);
	for (const auto& [slot, path] : descriptor->texture_paths)
	{
		const auto& slots = shader->get_texture_slots();
		if (std::find(slots.begin(), slots.end(), slot) == slots.end())
		{
			error = "The material '" + descriptor->name + "' has a texture for the slot '" + slot + "', which the shader '" +
				descriptor->shader_name + "' does not have";
			return nullptr;
		}
		auto texture = std::make_shared<Texture>(path.filename().string(), path.string(), Texture_Type::UNDEFINED);
		if (texture->get_texture_id() == 0)
		{
			error = "Failed to load the texture " + path.string() + " of the material '" + descriptor->name + "'";
			return nullptr;
		}
		material->set_texture(slot, texture);
	}
	return material;
}

bool Material_Library::save(Material& material, const std::filesystem::path& file, std::string& error)
{
	const auto descriptor = to_descriptor(material, error);
	if (!descriptor)
		return false;

	// write the descriptor, with the textures relative to its directory
	const std::string text = write_descriptor(*descriptor, file.parent_path());
	std::ofstream     stream{ file, std::ios::binary | std::ios::trunc }; // binary: the same line endings on every platform
	if (!stream)
	{
		error = "The file " + file.string() + " cannot be opened for writing";
		return false;
	}
	stream << text;
	stream.close();
	if (!stream)
	{
		error = "The file " + file.string() + " could not be written";
		return false;
	}

	material.set_file_path(file);
	std::cout << "[INFO::MATERIAL_LIBRARY::save] Saved the material '" << material.get_name() << "' to " << file.string() << std::endl;
	return true;
}

// Public Methods
// --------------
bool Material_Library::load_directory(const std::filesystem::path& dir, const Shader_Library& shader_library)
{
	const std::vector<std::filesystem::path> files = find_descriptor_files(dir);
	if (files.empty())
	{
		std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] No material descriptors (*" << DESCRIPTOR_EXTENSION << ") found in "
				  << dir.string() << std::endl;
		return false;
	}

	bool        all_loaded   = true;
	std::size_t loaded_count = 0; // number of materials loaded from this directory
	for (const auto& file : files)
	{
		// load the material (with its shader and textures)
		std::string error;
		auto        material = load_file(file, shader_library, error);
		if (!material)
		{
			std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] Failed to load the material descriptor " << file.string() << ": "
					  << error << std::endl;
			all_loaded = false;
			continue;
		}

		// material names identify the materials, so they must be unique
		if (get(material->get_name()))
		{
			std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] A material named '" << material->get_name() << "' already exists, "
					  << "so the descriptor " << file.string() << " is ignored" << std::endl;
			all_loaded = false;
			continue;
		}

		// add the material to the library
		materials.push_back(std::move(material));
		++loaded_count;
	}

	std::cout << "[INFO::MATERIAL_LIBRARY::load_directory] Loaded " << loaded_count << " of " << files.size() << " materials from "
			  << dir.string() << std::endl;
	return all_loaded;
}

std::shared_ptr<Material> Material_Library::add(std::shared_ptr<Material> material)
{
	// find a unique name, adding a numeric suffix to the material's name if it is already in use
	const std::string base_name = material->get_name();
	std::string       name      = base_name;
	for (int suffix = 2; get(name); ++suffix) name = base_name + " (" + std::to_string(suffix) + ")";
	material->set_name(name);

	materials.push_back(material);
	return material;
}

std::shared_ptr<Material> Material_Library::duplicate(const Material& material)
{
	auto copy = std::make_shared<Material>(material);
	copy->set_file_path({}); // the copy has no file until it is saved (saving it must not replace the original's file)
	return add(std::move(copy));
}

bool Material_Library::rename(Material& material, const std::string& new_name, std::string& error) const
{
	if (new_name == material.get_name())
		return true;
	if (material.get_name() == DEFAULT_MATERIAL)
	{
		error = std::string("The material '") + DEFAULT_MATERIAL + "' cannot be renamed, since the renderer finds it by its name";
		return false;
	}
	if (new_name.empty())
	{
		error = "The name of a material cannot be empty";
		return false;
	}
	if (get(new_name))
	{
		error = "A material named '" + new_name + "' already exists";
		return false;
	}
	material.set_name(new_name);
	return true;
}

bool Material_Library::reload(Material& material, const Shader_Library& shader_library, std::string& error) const
{
	if (material.get_file_path().empty())
	{
		error = "The material '" + material.get_name() + "' has no descriptor file";
		return false;
	}
	const auto reloaded = load_file(material.get_file_path(), shader_library, error);
	if (!reloaded)
		return false;

	// the reloaded name must keep the names of the library unique (and the default material must keep its name)
	if (material.get_name() == DEFAULT_MATERIAL && reloaded->get_name() != DEFAULT_MATERIAL)
	{
		error = std::string("The descriptor of the material '") + DEFAULT_MATERIAL + "' must keep its name";
		return false;
	}
	const auto other = get(reloaded->get_name());
	if (other && other.get() != &material)
	{
		error = "The descriptor names the material '" + reloaded->get_name() + "', which is the name of another material";
		return false;
	}

	material = *reloaded;
	return true;
}

std::shared_ptr<Material> Material_Library::get(const std::string& name) const
{
	auto it = std::find_if(materials.begin(), materials.end(), [&name](const auto& material) { return material->get_name() == name; });
	return it != materials.end() ? *it : nullptr;
}
