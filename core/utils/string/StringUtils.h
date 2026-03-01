/*
* StringUtils.h
* This file defines the StringUtils class, which provides static utility methods
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

class StringUtils
{
public:
	// Public Static Methods
	// ---------------------
	// Converts the input string to uppercase and returns the result
	static std::string ToUppercase(const std::string& inputString);

	// Converts the input string to uppercase in-place and returns the result
	static std::string ToUppercaseFromCopy(std::string inputString);

	// Converts the input string to lowercase and returns the result
	static std::string ToLowercase(const std::string& inputString);

	// Converts the input string to lowercase in-place and returns the result
	static std::string ToLowercaseFromCopy(std::string inputString);

	// Returns the file extension (including the dot) from the given filename
	static std::string GetFileExtension(const std::string& filename);

	// Returns the filename with extension from the given filepath
	static std::string GetFilenameWithExtension(const std::string& filepath);

	// Returns the filename without extension from the given filepath
	static std::string GetFilenameWithoutExtension(const std::string& filepath);

	// Returns the filename without extension or path from the given filepath
	static std::string GetFilenameWithoutExtensionOrPath(const std::string& filepath);

	// Returns the directory path from the given filepath
	static std::string GetDirectoryPath(const std::string& filepath);

	// Builds "{id: N} <existing name>" and returns it
	static std::string GenerateIdPrefixedName(Node& node);

	// Convenience overload for shared_ptr usage
	static std::string GenerateIdPrefixedName(const std::shared_ptr<Node>& node);

	// Converts a filename to a clean display name:
	// - Removes extension
	// - Replaces underscores with spaces
	// - Capitalizes the first letter of each word (title case)
	static std::string ToCleanDisplayName(const std::string& filename);
};