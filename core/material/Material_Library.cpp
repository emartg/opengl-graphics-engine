/*
 * Material_Library.cpp
 * This file implements the Material_Library class, which manages the materials of the engine as assets: each
 * material is described by a descriptor file (<name>.material.json) with its name, the name of its shader
 * (from the shader library), the values of its parameters, and its textures, so that materials are added or
 * changed without modifying the engine's code. The library loads every descriptor of a directory, holds the
 * materials created by the engine (e.g., those of imported models), and finds the materials by name.
 */

#include "Material_Library.h"

#include "../shader/Shader.h"
#include "../shader/Shader_Library.h"
#include "../texture/Texture.h"
#include "../utils/string/String_Utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

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
		// read and parse the descriptor
		std::ifstream     stream{ file };
		std::stringstream text;
		text << stream.rdbuf();
		std::string error;
		const auto  descriptor = parse_descriptor(text.str(), file.parent_path(), error);
		if (!descriptor)
		{
			std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] Invalid material descriptor " << file.string() << ": " << error
					  << std::endl;
			all_loaded = false;
			continue;
		}

		// material names identify the materials, so they must be unique
		if (get(descriptor->name))
		{
			std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] A material named '" << descriptor->name << "' already exists, "
					  << "so the descriptor " << file.string() << " is ignored" << std::endl;
			all_loaded = false;
			continue;
		}

		// find the shader of the material
		auto shader = shader_library.get(descriptor->shader_name);
		if (!shader)
		{
			std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] The material '" << descriptor->name << "' (" << file.string()
					  << ") uses the shader '" << descriptor->shader_name << "', which is not in the shader library" << std::endl;
			all_loaded = false;
			continue;
		}

		// create the material, with its textures (only for the slots of its shader)
		auto material = std::make_shared<Material>(descriptor->name, shader, descriptor->parameters, descriptor->supports_instancing);
		bool has_valid_textures = true;
		for (const auto& [slot, path] : descriptor->texture_paths)
		{
			const auto& slots = shader->get_texture_slots();
			if (std::find(slots.begin(), slots.end(), slot) == slots.end())
			{
				std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] The material '" << descriptor->name
						  << "' has a texture for the slot '" << slot << "', which the shader '" << descriptor->shader_name
						  << "' does not have" << std::endl;
				has_valid_textures = false;
				continue;
			}
			auto texture = std::make_shared<Texture>(path.filename().string(), path.string(), Texture_Type::UNDEFINED);
			if (texture->get_texture_id() == 0)
			{
				std::cerr << "[ERROR::MATERIAL_LIBRARY::load_directory] Failed to load the texture " << path.string()
						  << " of the material '" << descriptor->name << "'" << std::endl;
				has_valid_textures = false;
				continue;
			}
			material->set_texture(slot, texture);
		}
		if (!has_valid_textures)
		{
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

std::shared_ptr<Material> Material_Library::get(const std::string& name) const
{
	auto it = std::find_if(materials.begin(), materials.end(), [&name](const auto& material) { return material->get_name() == name; });
	return it != materials.end() ? *it : nullptr;
}
