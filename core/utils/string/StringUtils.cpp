/*
* StringUtils.cpp
* This file implements the StringUtils class, which provides static utility methods
* whose functionality revolves around string manipulation and formatting.
*/

#include "StringUtils.h"

#include "../../Node.h"

// Public Static Methods
// ---------------------
std::string StringUtils::ToUppercase(const std::string& inputString)
{
	std::string result = inputString;
	std::transform(result.begin(), result.end(), result.begin(),
				   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return result;
}

std::string StringUtils::ToUppercaseFromCopy(std::string inputString)
{
	return ToUppercase(inputString);
}

std::string StringUtils::ToLowercase(const std::string& inputString)
{
	std::string result = inputString;
	std::transform(result.begin(), result.end(), result.begin(),
				   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return result;
}

std::string StringUtils::ToLowercaseFromCopy(std::string inputString)
{
	return ToLowercase(inputString);
}

std::string StringUtils::GetFileExtension(const std::string& filename)
{
	std::filesystem::path filePath(filename);
	return filePath.extension().string();
}

std::string StringUtils::GetFilenameWithExtension(const std::string& filepath)
{
	std::filesystem::path filePath(filepath);
	return filePath.filename().string();
}

std::string StringUtils::GetFilenameWithoutExtension(const std::string& filepath)
{
	std::filesystem::path filePath(filepath);
	return filePath.stem().string();
}

std::string StringUtils::GetFilenameWithoutExtensionOrPath(const std::string& filepath)
{
	std::filesystem::path filePath(filepath);
	return filePath.stem().filename().string();
}

std::string StringUtils::GetDirectoryPath(const std::string& filepath)
{
	std::filesystem::path filePath(filepath);
	return filePath.parent_path().string();
}

std::string StringUtils::GenerateIdPrefixedName(Node& node)
{
	std::ostringstream oss; // create an output string stream
	oss << "{id: " << node.GetId() << "} " << node.GetName(); // format the string with id and name
	return oss.str(); // return the generated string from the output string stream
}

std::string StringUtils::GenerateIdPrefixedName(const std::shared_ptr<Node>& node)
{
	if (!node) return {}; // return empty string if node is null
	return GenerateIdPrefixedName(*node); // call the reference overload
}

std::string StringUtils::ToCleanDisplayName(const std::string& filename)
{
	// remove the file extension
	std::string name = GetFilenameWithoutExtension(filename);

	// replace underscores and hyphens with spaces (common separators in filenames) to improve readability
	std::replace(name.begin(), name.end(), '_', ' ');
	std::replace(name.begin(), name.end(), '-', ' ');

	// capitalize first letter of each word and convert the rest to lowercase,
	// using a flag to determine when to capitalize the next character,
	// thus ensuring a title case format even if the input is in an inconsistent case format
	bool capitalizeNext = true;
	for (char& c : name)
	{
		if (std::isspace(static_cast<unsigned char>(c)))
		{ // if the character is a space, set the flag to capitalize the next character
			capitalizeNext = true;
		}
		else if (capitalizeNext)
		{ // if the flag is set, capitalize the character and reset the flag
			c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			capitalizeNext = false;
		}
		else
		{ // otherwise, convert the character to lowercase
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
	}

	return name;
}