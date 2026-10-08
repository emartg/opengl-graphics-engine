/*
 * Shader_Preprocessor.cpp
 * This file implements the Shader_Preprocessor class, which loads the source code of a shader stage
 * from a file and resolves its #include directives, so that several shaders can share common code
 * (e.g., the lighting functions) instead of duplicating it. GLSL has no #include directive of its own.
 */

#include "Shader_Preprocessor.h"

#include <algorithm>
#include <fstream>
#include <system_error>

namespace
{
	// Removes the leading and trailing spaces, tabs, and carriage returns of a line
	std::string trim(const std::string& line)
	{
		const auto first = line.find_first_not_of(" \t\r");
		if (first == std::string::npos)
			return {};
		const auto last = line.find_last_not_of(" \t\r");
		return line.substr(first, last - first + 1);
	}

	// Returns a normalized absolute path, used to recognize a file included more than once through different paths
	std::filesystem::path normalize(const std::filesystem::path& path)
	{
		std::error_code             error;
		const std::filesystem::path normalized = std::filesystem::weakly_canonical(path, error);
		return error ? path.lexically_normal() : normalized;
	}

	// Appends the contents of the file at the given path to the source (resolving its #include directives
	// recursively). Returns false and sets the error message of the source if a file cannot be read
	bool append_file(const std::filesystem::path& path, Shader_Source& source)
	{
		std::ifstream file{ path };
		if (!file.is_open())
		{
			source.error = "Cannot open the file '" + path.string() + "'";
			return false;
		}

		// register the file, whose index is its source string number in the #line directives
		const std::size_t file_index = source.files.size();
		source.files.push_back(normalize(path));

		std::string line;
		std::size_t line_number = 0;
		while (std::getline(file, line))
		{
			++line_number;

			// copy every line that is not an #include directive
			const std::string directive = trim(line);
			if (!directive.starts_with("#include"))
			{
				source.code += line + "\n";
				continue;
			}

			// extract the quoted path of the directive (e.g., #include "include/lighting.glsl")
			const auto open_quote  = directive.find('"');
			const auto close_quote = directive.find('"', open_quote + 1);
			if (open_quote == std::string::npos || close_quote == std::string::npos || close_quote == open_quote + 1)
			{
				source.error = "Malformed #include directive in '" + path.string() + "' (line " + std::to_string(line_number) +
					"): expected #include \"<file>\"";
				return false;
			}
			const std::filesystem::path included_path = path.parent_path() / directive.substr(open_quote + 1, close_quote - open_quote - 1);

			// skip the files that were already included (which also prevents circular includes), replacing the
			// directive with an empty line so that the following lines keep their line numbers
			if (std::find(source.files.begin(), source.files.end(), normalize(included_path)) != source.files.end())
			{
				source.code += "\n";
				continue;
			}

			// insert the included file, numbering its lines from 1 with its own source string number, and then
			// continue numbering the lines of this file after the directive
			source.code += "#line 1 " + std::to_string(source.files.size()) + "\n";
			if (!append_file(included_path, source))
			{
				source.error += " (included from '" + path.string() + "', line " + std::to_string(line_number) + ")";
				return false;
			}
			source.code += "#line " + std::to_string(line_number + 1) + " " + std::to_string(file_index) + "\n";
		}
		return true;
	}
}

// Public Static Methods
// ---------------------
Shader_Source Shader_Preprocessor::load(const std::filesystem::path& path)
{
	Shader_Source source;
	if (!append_file(path, source))
		source.code.clear(); // a source with errors is not compiled
	return source;
}
