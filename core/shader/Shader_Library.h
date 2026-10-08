/*
 * Shader_Library.h
 * This file defines the Shader_Library class, which manages the shader programs of the engine as assets:
 * each program is described by a descriptor file (<name>.shader.json) with its name and the files of its
 * stages, so that shaders are added or changed without modifying the engine's code. The library loads every
 * descriptor of a directory, compiles the programs, finds them by name, and reloads them at runtime.
 *
 * Example of a descriptor (the stage files are relative to the descriptor, and the geometry stage is optional):
 * {
 *     "name": "Skybox Shader",
 *     "vertex": "skybox.vert.glsl",
 *     "fragment": "skybox.frag.glsl"
 * }
 * A shader whose materials have textures lists its texture slots (optional): "textures": [ "albedo_map" ]. Each
 * slot is a sampler of the shader's u_material struct, with a boolean u_material.has_<slot> that tells the shader
 * whether the material has a texture for it (see Material::apply)
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Shader;

// Contents of a shader descriptor file, with the paths of the stage files resolved
struct Shader_Descriptor
{
	std::string              name;
	std::filesystem::path    vertex_path;
	std::filesystem::path    geometry_path; // empty if the program has no geometry stage
	std::filesystem::path    fragment_path;
	std::vector<std::string> texture_slots; // names of the texture slots of the shader's materials (if any)
};

class Shader_Library
{
public:
	// Public Constants
	// ----------------
	// extension of the shader descriptor files
	static constexpr const char* DESCRIPTOR_EXTENSION{ ".shader.json" };

	// Constructors
	// ------------
	Shader_Library() = default;

	// Destructor
	// ----------
	// releases the programs of the shaders (it must be destroyed while the OpenGL context still exists)
	~Shader_Library();

	// Public Static Methods
	// ---------------------
	// Parses the text of a shader descriptor (JSON), resolving the stage files relative to the given base
	// directory (the directory of the descriptor). Returns the descriptor, or an empty optional (and an error
	// message) if the text is not valid JSON, lacks a required field ("name", "vertex", and "fragment"), or has
	// a field of an invalid type
	static std::optional<Shader_Descriptor>
	parse_descriptor(const std::string& text, const std::filesystem::path& base_dir, std::string& error);

	// Returns the descriptor files of a directory (not its subdirectories), sorted by file name
	static std::vector<std::filesystem::path> find_descriptor_files(const std::filesystem::path& dir);

	// Public Methods
	// --------------
	// Loads and compiles the shaders of every descriptor file of a directory (it can be called for several
	// directories, e.g., the engine's shaders and an application's own). Shader names must be unique: a descriptor
	// with the name of a shader of the library is ignored. Returns true if every shader was loaded and compiled,
	// false otherwise (the errors are printed, and the shaders that failed are not added)
	bool load_directory(const std::filesystem::path& dir);

	// Recompiles a shader from its files (see Shader::compile, which keeps the previous program if it fails).
	// Returns true if the shader exists and was recompiled successfully, false otherwise
	bool reload(const std::string& name);
	// Recompiles every shader from its files, and returns the number of shaders that failed
	std::size_t reload_all();

	// Releases the programs of every shader and empties the library
	void clear();

	// Getters
	// returns the shader with the given name, or nullptr if there is none
	std::shared_ptr<Shader> get(const std::string& name) const;
	// returns every shader, in loading order
	const std::vector<std::shared_ptr<Shader>>& get_shaders() const { return shaders; }

private:
	// Private Attributes
	// ------------------
	std::vector<std::shared_ptr<Shader>> shaders; // shaders of the library, in loading order
};
