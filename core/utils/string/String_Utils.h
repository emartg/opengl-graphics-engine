/*
 * String_Utils.h
 * This file defines the String_Utils class, which provides static utility methods
 * whose functionality revolves around string manipulation and formatting.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>
#include <string>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Node;

class String_Utils
{
public:
	// Public Static Methods
	// ---------------------
	// Converts the input string to uppercase and returns the result
	static std::string to_uppercase(const std::string& input_string);

	// Converts the input string to uppercase in-place and returns the result
	static std::string to_uppercase_from_copy(std::string input_string);

	// Converts the input string to lowercase and returns the result
	static std::string to_lowercase(const std::string& input_string);

	// Converts the input string to lowercase in-place and returns the result
	static std::string to_lowercase_from_copy(std::string input_string);

	// Returns the file extension (including the dot) from the given filename
	static std::string get_file_extension(const std::string& filename);

	// Returns the filename with extension from the given filepath
	static std::string get_filename_with_extension(const std::string& filepath);

	// Returns the filename without extension from the given filepath
	static std::string get_filename_without_extension(const std::string& filepath);

	// Returns the filename without extension or path from the given filepath
	static std::string get_filename_without_extension_or_path(const std::string& filepath);

	// Returns the directory path from the given filepath
	static std::string get_directory_path(const std::string& filepath);

	// Builds "{id: N} <existing name>" and returns it
	static std::string generate_id_prefixed_name(Node& node);

	// Convenience overload for shared_ptr usage
	static std::string generate_id_prefixed_name(const std::shared_ptr<Node>& node);

	// Converts a filename to a clean display name:
	// - Removes extension
	// - Replaces underscores with spaces
	// - Capitalizes the first letter of each word (title case)
	static std::string to_clean_display_name(const std::string& filename);
};