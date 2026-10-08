/*
 * Material_Library.h
 * This file defines the Material_Library class, which manages the materials of the engine as assets: each
 * material is described by a descriptor file (<name>.material.json) with its name, the name of its shader
 * (from the shader library), the values of its parameters, and its textures, so that materials are added or
 * changed without modifying the engine's code. The library loads every descriptor of a directory, holds the
 * materials created by the engine (e.g., those of imported models), and finds the materials by name. It also
 * saves materials to descriptor files and reloads them, duplicates them, and renames them (e.g., for an editor).
 *
 * Example of a descriptor ("instancing", "parameters", "textures", and "environment" are optional; a parameter value
 * is a number, a boolean, or an array of 2 to 4 numbers, set as the uniform u_material.<parameter name> of the shader,
 * and a texture is the path of an image file, relative to the descriptor, for a texture slot of the shader):
 * {
 *     "name": "Default",
 *     "shader": "Lit Shader",
 *     "instancing": true,
 *     "parameters": { "shininess": 32.0 },
 *     "textures": { "albedo_map": "../textures/wood.png" }
 * }
 * The environment map sampled by the shader (as the cubemap u_environment_map) is "skybox" or "dynamic" (captured
 * every frame from each object), with an optional "environment_resolution" for the faces of dynamic cubemaps, e.g.:
 * { "name": "Mirror", "shader": "Reflective Shader", "environment": "dynamic", "environment_resolution": 512 }
 */

#pragma once

#include "Material.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Shader_Library;

// Contents of a material descriptor file
struct Material_Descriptor
{
	std::string                                  name;
	std::string                                  shader_name; // name of the shader in the shader library
	Material_Parameters                          parameters;
	std::map<std::string, std::filesystem::path> texture_paths; // image file of each texture slot
	bool                                         supports_instancing{ false };
	Environment_Mode                             environment_mode{ Environment_Mode::NONE };
	unsigned int                                 environment_resolution{ Material::DEFAULT_ENVIRONMENT_RESOLUTION };
};

class Material_Library
{
public:
	// Public Constants
	// ----------------
	// extension of the material descriptor files
	static constexpr const char* DESCRIPTOR_EXTENSION{ ".material.json" };
	// name of the material used by the meshes without a material of their own (e.g., the shapes)
	static constexpr const char* DEFAULT_MATERIAL{ "Default" };

	// Public Static Methods
	// ---------------------
	// Parses the text of a material descriptor (JSON), resolving the texture files relative to the given base
	// directory (the directory of the descriptor). Returns the descriptor, or an empty optional (and an error
	// message) if the text is not valid JSON, lacks a required field ("name" and "shader"), or has a field,
	// a parameter, or a texture of an invalid type
	static std::optional<Material_Descriptor>
	parse_descriptor(const std::string& text, const std::filesystem::path& base_dir, std::string& error);

	// Returns the descriptor files of a directory (not its subdirectories), sorted by file name
	static std::vector<std::filesystem::path> find_descriptor_files(const std::filesystem::path& dir);

	// Returns the contents of the descriptor of a material, with the image files of its textures. Returns an empty
	// optional (and an error message) if the material has no shader, or a texture without an image file
	static std::optional<Material_Descriptor> to_descriptor(const Material& material, std::string& error);
	// Writes the text (JSON) of a material descriptor, which parse_descriptor reads back. The texture files are written
	// relative to the given base directory (the directory of the descriptor) when possible, and as absolute paths
	// otherwise (e.g., on another drive)
	static std::string write_descriptor(const Material_Descriptor& descriptor, const std::filesystem::path& base_dir);

	// Returns the name of the descriptor file of a material with the given name: the name in lowercase, with each run of
	// characters that are not ASCII letters or digits replaced with an underscore (e.g., "dynamic_glass.material.json"
	// for "Dynamic Glass"), or "material.material.json" if the name has no letters or digits
	static std::string make_descriptor_file_name(const std::string& material_name);
	// Returns the path of a new descriptor file for a material with the given name in a directory, which is not the path
	// of an existing file (a numeric suffix is added to the file name if needed, e.g., "glass_2.material.json")
	static std::filesystem::path make_new_descriptor_path(const std::string& material_name, const std::filesystem::path& dir);

	// Loads a material from a descriptor file, with its shader from the given shader library and its textures, without
	// adding it to the library (the file becomes the file of the material). Returns nullptr (and an error message) if
	// the file is not a valid descriptor, its shader is not in the shader library, or one of its textures is not a slot
	// of the shader or cannot be loaded
	static std::shared_ptr<Material> load_file(const std::filesystem::path& file, const Shader_Library& shader_library, std::string& error);

	// Saves a material to a descriptor file (creating or replacing it), which becomes the file of the material. Returns
	// false (and an error message) if the material has no valid descriptor (see to_descriptor) or the file cannot be written
	static bool save(Material& material, const std::filesystem::path& file, std::string& error);

	// Public Methods
	// --------------
	// Loads the materials of every descriptor file of a directory, with their shaders from the given shader library
	// (it can be called for several directories). Material names must be unique: a descriptor with the name of a
	// material of the library is ignored. Returns true if every material was loaded, false otherwise (the errors
	// are printed, and the materials that failed, e.g., because their shader or a texture does not exist, are not added)
	bool load_directory(const std::filesystem::path& dir, const Shader_Library& shader_library);

	// Adds a material created by the engine or an application (e.g., the material of an imported model). If its name
	// is already in the library, the material is renamed with a numeric suffix (e.g., "Teapot (2)"), so that the
	// names stay unique. Returns the material
	std::shared_ptr<Material> add(std::shared_ptr<Material> material);
	// Adds a copy of a material of the library (with its shader, parameters, textures, and environment map), named after
	// it with a numeric suffix (see add), and without a descriptor file until it is saved. Returns the copy
	std::shared_ptr<Material> duplicate(const Material& material);

	// Renames a material of the library. Returns false (and an error message), keeping its name, if the new name is
	// empty or the name of another material, or if the material is the default material (found by its name)
	bool rename(Material& material, const std::string& new_name, std::string& error) const;

	// Reloads a material from its descriptor file (e.g., to discard its unsaved changes), changing the material itself,
	// so that the objects that use it are drawn with the reloaded material. Returns false (and an error message),
	// keeping the material unchanged, if the material has no file, it cannot be loaded (see load_file), or its name in
	// the file is the name of another material of the library (or not the default name, for the default material)
	bool reload(Material& material, const Shader_Library& shader_library, std::string& error) const;

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
