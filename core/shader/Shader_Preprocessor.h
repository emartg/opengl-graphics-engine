/*
 * Shader_Preprocessor.h
 * This file defines the Shader_Preprocessor class, which loads the source code of a shader stage
 * from a file and resolves its #include directives, so that several shaders can share common code
 * (e.g., the lighting functions) instead of duplicating it. GLSL has no #include directive of its own.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

// Source code of a shader stage after resolving its #include directives
struct Shader_Source
{
	std::string                        code;  // source code, ready to be compiled
	std::vector<std::filesystem::path> files; // files that make up the code (index = source string number)
	std::string                        error; // error message if the source could not be loaded (empty otherwise)

	// Returns true if the source was loaded successfully
	bool is_valid() const { return error.empty(); }
};

class Shader_Preprocessor
{
public:
	// Public Static Methods
	// ---------------------
	// Loads the shader source file at the given path, replacing each #include "<file>" directive (on a line
	// of its own) with the contents of that file, recursively. Included paths are relative to the directory
	// of the file that includes them, and each file is included only once (later directives that include it
	// again, as well as circular includes, are removed).
	// #line directives are inserted around the included code, so that the compiler messages show the line
	// numbers of the original files: the source string number N of a message corresponds to files[N]
	static Shader_Source load(const std::filesystem::path& path);
};
