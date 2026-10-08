/*
 * String_Utils.cpp
 * This file implements the String_Utils class, which provides static utility methods
 * whose functionality revolves around string manipulation and formatting.
 */

#include "String_Utils.h"

#include "../../Node.h"

// Public Static Methods
// ---------------------
std::string String_Utils::to_uppercase(const std::string& input_string)
{
	std::string result = input_string;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return result;
}

std::string String_Utils::to_uppercase_from_copy(std::string input_string)
{
	return to_uppercase(input_string);
}

std::string String_Utils::to_lowercase(const std::string& input_string)
{
	std::string result = input_string;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return result;
}

std::string String_Utils::to_lowercase_from_copy(std::string input_string)
{
	return to_lowercase(input_string);
}

std::string String_Utils::get_file_extension(const std::string& filename)
{
	std::filesystem::path system_filepath(filename);
	return system_filepath.extension().string();
}

std::string String_Utils::get_filename_with_extension(const std::string& filepath)
{
	std::filesystem::path system_filepath(filepath);
	return system_filepath.filename().string();
}

std::string String_Utils::get_filename_without_extension(const std::string& filepath)
{
	std::filesystem::path system_filepath(filepath);
	return system_filepath.stem().string();
}

std::string String_Utils::get_filename_without_extension_or_path(const std::string& filepath)
{
	std::filesystem::path system_filepath(filepath);
	return system_filepath.stem().filename().string();
}

std::string String_Utils::get_directory_path(const std::string& filepath)
{
	std::filesystem::path system_filepath(filepath);
	return system_filepath.parent_path().string();
}

std::string String_Utils::generate_id_prefixed_name(Node& node)
{
	std::ostringstream oss;                                     // create an output string stream
	oss << "{id: " << node.get_id() << "} " << node.get_name(); // format the string with id and name
	return oss.str();                                           // return the generated string from the output string stream
}

std::string String_Utils::generate_id_prefixed_name(const std::shared_ptr<Node>& node)
{
	if (!node)
		return {};                           // return empty string if node is null
	return generate_id_prefixed_name(*node); // call the reference overload
}

std::string String_Utils::to_clean_display_name(const std::string& filename)
{
	// remove the file extension
	std::string name = get_filename_without_extension(filename);

	// replace underscores and hyphens with spaces (common separators in filenames) to improve readability
	std::replace(name.begin(), name.end(), '_', ' ');
	std::replace(name.begin(), name.end(), '-', ' ');

	// capitalize first letter of each word and convert the rest to lowercase,
	// using a flag to determine when to capitalize the next character,
	// thus ensuring a title case format even if the input is in an inconsistent case format
	bool capitalize_next = true;
	for (char& c : name)
	{
		if (std::isspace(static_cast<unsigned char>(c)))
		{ // if the character is a space, set the flag to capitalize the next character
			capitalize_next = true;
		}
		else if (capitalize_next)
		{ // if the flag is set, capitalize the character and reset the flag
			c               = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			capitalize_next = false;
		}
		else
		{ // otherwise, convert the character to lowercase
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
	}

	return name;
}

bool String_Utils::is_glsl_identifier(const std::string& text)
{
	// the first character must be a letter or an underscore, and the rest letters, digits, or underscores
	if (text.empty() || !(std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_'))
		return false;
	const bool has_valid_characters =
		std::all_of(text.begin(), text.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });

	// the "gl_" prefix and two consecutive underscores are reserved by GLSL
	return has_valid_characters && !text.starts_with("gl_") && text.find("__") == std::string::npos;
}
