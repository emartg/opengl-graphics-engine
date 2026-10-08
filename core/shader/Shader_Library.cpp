/*
 * Shader_Library.cpp
 * This file implements the Shader_Library class, which manages the shader programs of the engine as assets:
 * each program is described by a descriptor file (<name>.shader.json) with its name and the files of its
 * stages, so that shaders are added or changed without modifying the engine's code. The library loads every
 * descriptor of a directory, compiles the programs, finds them by name, and reloads them at runtime.
 */

#include "Shader_Library.h"

#include "Shader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>

// Destructor
// ----------
Shader_Library::~Shader_Library()
{
	clear();
}

// Public Static Methods
// ---------------------
std::optional<Shader_Descriptor>
Shader_Library::parse_descriptor(const std::string& text, const std::filesystem::path& base_dir, std::string& error)
{
	// parse the text without exceptions (a discarded value means that the text is not valid JSON)
	const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);
	if (json.is_discarded() || !json.is_object())
	{
		error = "The descriptor is not a valid JSON object";
		return std::nullopt;
	}

	// reads a string field (an empty string if it is missing and optional), reporting a missing required
	// field or a field of another type
	auto read_string = [&json, &error](const char* field, bool required, std::string& value) {
		if (!json.contains(field))
		{
			if (required)
				error = std::string("Missing required field \"") + field + "\"";
			return !required;
		}
		if (!json[field].is_string() || json[field].get<std::string>().empty())
		{
			error = std::string("The field \"") + field + "\" must be a non-empty string";
			return false;
		}
		value = json[field].get<std::string>();
		return true;
	};

	std::string name, vertex_file, geometry_file, fragment_file;
	if (!read_string("name", true, name) || !read_string("vertex", true, vertex_file) || !read_string("geometry", false, geometry_file) ||
		!read_string("fragment", true, fragment_file))
		return std::nullopt;

	// resolve the stage files relative to the directory of the descriptor
	Shader_Descriptor descriptor;
	descriptor.name          = name;
	descriptor.vertex_path   = base_dir / vertex_file;
	descriptor.geometry_path = geometry_file.empty() ? std::filesystem::path{} : base_dir / geometry_file;
	descriptor.fragment_path = base_dir / fragment_file;
	return descriptor;
}

std::vector<std::filesystem::path> Shader_Library::find_descriptor_files(const std::filesystem::path& dir)
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
bool Shader_Library::load_directory(const std::filesystem::path& dir)
{
	const std::vector<std::filesystem::path> files = find_descriptor_files(dir);
	if (files.empty())
	{
		std::cerr << "[ERROR::SHADER_LIBRARY::load_directory] No shader descriptors (*" << DESCRIPTOR_EXTENSION << ") found in "
				  << dir.string() << std::endl;
		return false;
	}

	bool        all_loaded   = true;
	std::size_t loaded_count = 0; // number of shaders loaded from this directory
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
			std::cerr << "[ERROR::SHADER_LIBRARY::load_directory] Invalid shader descriptor " << file.string() << ": " << error
					  << std::endl;
			all_loaded = false;
			continue;
		}

		// shader names identify the shaders, so they must be unique
		if (get(descriptor->name))
		{
			std::cerr << "[ERROR::SHADER_LIBRARY::load_directory] A shader named '" << descriptor->name << "' already exists, "
					  << "so the descriptor " << file.string() << " is ignored" << std::endl;
			all_loaded = false;
			continue;
		}

		// create and compile the shader, and add it to the library
		auto shader = std::make_shared<Shader>(
			descriptor->name,
			descriptor->vertex_path.string(),
			descriptor->geometry_path.string(),
			descriptor->fragment_path.string());
		if (!shader->compile())
		{
			std::cerr << "[ERROR::SHADER_LIBRARY::load_directory] Failed to compile the shader '" << descriptor->name << "' ("
					  << file.string() << ")" << std::endl;
			all_loaded = false;
			continue;
		}
		shaders.push_back(std::move(shader));
		++loaded_count;
	}

	std::cout << "[INFO::SHADER_LIBRARY::load_directory] Loaded " << loaded_count << " of " << files.size() << " shaders from "
			  << dir.string() << std::endl;
	return all_loaded;
}

bool Shader_Library::reload(const std::string& name)
{
	const auto shader = get(name);
	if (!shader)
	{
		std::cerr << "[ERROR::SHADER_LIBRARY::reload] Unknown shader '" << name << "'" << std::endl;
		return false;
	}
	return shader->compile();
}

std::size_t Shader_Library::reload_all()
{
	std::size_t failed_count = 0;
	for (const auto& shader : shaders)
		if (!shader->compile())
			++failed_count;
	return failed_count;
}

void Shader_Library::clear()
{
	for (const auto& shader : shaders) shader->deallocate_resources();
	shaders.clear();
}

std::shared_ptr<Shader> Shader_Library::get(const std::string& name) const
{
	auto it = std::find_if(shaders.begin(), shaders.end(), [&name](const auto& shader) { return shader->get_name() == name; });
	return it != shaders.end() ? *it : nullptr;
}
