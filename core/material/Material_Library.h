/*
 * Material_Library.h
 * This file defines the Material_Library class, which manages the materials of the engine as assets: each
 * material is described by a descriptor file (<name>.material.json) with its name, the name of its shader
 * (from the shader library), and the values of its parameters, so that materials are added or changed
 * without modifying the engine's code. The library loads every descriptor of a directory and finds the
 * materials by name.
 *
 * Example of a descriptor ("instancing" and "parameters" are optional; a parameter value is a number, a
 * boolean, or an array of 2 to 4 numbers, set as the uniform u_material.<parameter name> of the shader):
 * {
 *     "name": "Default Shape",
 *     "shader": "Shape Model Shader",
 *     "instancing": true,
 *     "parameters": { "shininess": 32.0 }
 * }
 */

#pragma once

#include "Material.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Shader_Library;

// Contents of a material descriptor file
struct Material_Descriptor
{
	std::string         name;
	std::string         shader_name; // name of the shader in the shader library
	Material_Parameters parameters;
	bool                supports_instancing{ false };
};

class Material_Library
{
public:
	// Public Constants
	// ----------------
	// extension of the material descriptor files
	static constexpr const char* DESCRIPTOR_EXTENSION{ ".material.json" };
	// names of the materials used by the objects without a material of their own (shapes and imported models)
	static constexpr const char* DEFAULT_SHAPE_MATERIAL{ "Default Shape" };
	static constexpr const char* DEFAULT_MODEL_MATERIAL{ "Default Model" };

	// Public Static Methods
	// ---------------------
	// Parses the text of a material descriptor (JSON). Returns the descriptor, or an empty optional (and an error
	// message) if the text is not valid JSON, lacks a required field ("name" and "shader"), or has a field or
	// a parameter of an invalid type
	static std::optional<Material_Descriptor> parse_descriptor(const std::string& text, std::string& error);

	// Returns the descriptor files of a directory (not its subdirectories), sorted by file name
	static std::vector<std::filesystem::path> find_descriptor_files(const std::filesystem::path& dir);

	// Public Methods
	// --------------
	// Loads the materials of every descriptor file of a directory, with their shaders from the given shader library
	// (it can be called for several directories). Material names must be unique: a descriptor with the name of a
	// material of the library is ignored. Returns true if every material was loaded, false otherwise (the errors
	// are printed, and the materials that failed, e.g., because their shader does not exist, are not added)
	bool load_directory(const std::filesystem::path& dir, const Shader_Library& shader_library);

	// Removes every material from the library
	void clear() { materials.clear(); }

	// Getters
	// returns the material with the given name, or nullptr if there is none
	std::shared_ptr<Material> get(const std::string& name) const;
	// returns every material, in loading order
	const std::vector<std::shared_ptr<Material>>& get_materials() const { return materials; }

private:
	// Private Attributes
	// ------------------
	std::vector<std::shared_ptr<Material>> materials; // materials of the library, in loading order
};
