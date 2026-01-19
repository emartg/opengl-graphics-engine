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