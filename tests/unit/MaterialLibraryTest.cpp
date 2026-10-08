/*
 * MaterialLibraryTest.cpp
 * This file contains the unit tests of the Material_Library class that do not require an OpenGL context:
 * the parsing of material descriptors (fields, parameter types, and textures), the search of descriptor files,
 * and the validity of the descriptors of the engine's own materials.
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "core/material/Material_Library.h"
#include "core/shader/Shader_Library.h"

namespace fs = std::filesystem;

namespace
{
	// Reads a whole text file
	std::string read_file(const fs::path& path)
	{
		std::ifstream     stream{ path };
		std::stringstream text;
		text << stream.rdbuf();
		return text.str();
	}
}

// Descriptor Parsing
// ------------------
TEST(MaterialLibraryTest, ParsesADescriptorWithOnlyTheRequiredFields)
{
	std::string error;
	const auto  descriptor = Material_Library::parse_descriptor(R"({ "name": "Test", "shader": "Test Shader" })", fs::path{}, error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_EQ(descriptor->name, "Test");
	EXPECT_EQ(descriptor->shader_name, "Test Shader");
	EXPECT_TRUE(descriptor->parameters.empty());
	EXPECT_FALSE(descriptor->supports_instancing);
}

TEST(MaterialLibraryTest, ParsesTheInstancingFlag)
{
	std::string error;
	const auto  descriptor =
		Material_Library::parse_descriptor(R"({ "name": "Test", "shader": "S", "instancing": true })", fs::path{}, error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_TRUE(descriptor->supports_instancing);
}

TEST(MaterialLibraryTest, ParsesEachParameterTypeFromItsJsonValue)
{
	std::string error;
	const auto  descriptor = Material_Library::parse_descriptor(
		R"({ "name": "Test", "shader": "S", "parameters": {
			"shininess": 32, "roughness": 0.5, "enabled": true,
			"offset": [1, 2], "tint": [0.1, 0.2, 0.3], "color": [1, 0, 0, 0.5] } })",
		fs::path{},
		error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	const auto& parameters = descriptor->parameters;
	ASSERT_EQ(parameters.size(), 6u);
	// numbers are floats (also when they are written without a decimal point)
	EXPECT_EQ(std::get<float>(parameters.at("shininess")), 32.0f);
	EXPECT_EQ(std::get<float>(parameters.at("roughness")), 0.5f);
	EXPECT_EQ(std::get<bool>(parameters.at("enabled")), true);
	EXPECT_EQ(std::get<glm::vec2>(parameters.at("offset")), glm::vec2(1.0f, 2.0f));
	EXPECT_EQ(std::get<glm::vec3>(parameters.at("tint")), glm::vec3(0.1f, 0.2f, 0.3f));
	EXPECT_EQ(std::get<glm::vec4>(parameters.at("color")), glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
}

TEST(MaterialLibraryTest, RejectsInvalidJsonAndValuesThatAreNotObjects)
{
	std::string error;
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": )", fs::path{}, error).has_value());
	EXPECT_FALSE(Material_Library::parse_descriptor(R"([ "name", "shader" ])", fs::path{}, error).has_value());
	EXPECT_FALSE(error.empty());
}

TEST(MaterialLibraryTest, RejectsADescriptorWithoutARequiredField)
{
	std::string error;
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "shader": "S" })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("name"), std::string::npos);
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": "Test" })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("shader"), std::string::npos);
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": "", "shader": "S" })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("name"), std::string::npos);
}

TEST(MaterialLibraryTest, RejectsFieldsOfInvalidTypes)
{
	std::string error;
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "instancing": 1 })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("instancing"), std::string::npos);
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "parameters": [] })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("parameters"), std::string::npos);
}

TEST(MaterialLibraryTest, RejectsParametersOfInvalidTypes)
{
	for (const char* value : { R"("text")", "[1]", "[1, 2, 3, 4, 5]", R"([1, "2"])", "{}", "null" })
	{
		std::string       error;
		const std::string text = std::string(R"({ "name": "T", "shader": "S", "parameters": { "bad": )") + value + " } }";
		EXPECT_FALSE(Material_Library::parse_descriptor(text, fs::path{}, error).has_value()) << value;
		EXPECT_NE(error.find("bad"), std::string::npos) << error;
	}
}

TEST(MaterialLibraryTest, ParsesTheTexturesAndResolvesThemRelativeToTheBaseDirectory)
{
	std::string error;
	const auto  descriptor = Material_Library::parse_descriptor(
		R"({ "name": "T", "shader": "S", "textures": { "albedo_map": "../textures/wood.png", "opacity_map": "mask.png" } })",
		fs::path{ "materials" },
		error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	ASSERT_EQ(descriptor->texture_paths.size(), 2u);
	EXPECT_EQ(descriptor->texture_paths.at("albedo_map"), fs::path("materials") / "../textures/wood.png");
	EXPECT_EQ(descriptor->texture_paths.at("opacity_map"), fs::path("materials") / "mask.png");
}

TEST(MaterialLibraryTest, RejectsTexturesOfInvalidTypes)
{
	std::string error;
	EXPECT_FALSE(Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "textures": [] })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("textures"), std::string::npos);
	EXPECT_FALSE(
		Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "textures": { "albedo_map": 1 } })", fs::path{}, error)
			.has_value());
	EXPECT_NE(error.find("albedo_map"), std::string::npos);
	EXPECT_FALSE(
		Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "textures": { "albedo_map": "" } })", fs::path{}, error)
			.has_value());
	EXPECT_NE(error.find("albedo_map"), std::string::npos);
}

TEST(MaterialLibraryTest, RejectsParameterNamesAndTextureSlotsThatAreNotGlslIdentifiers)
{
	std::string error;
	EXPECT_FALSE(
		Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "parameters": { "base-shininess": 1 } })", fs::path{}, error)
			.has_value());
	EXPECT_NE(error.find("base-shininess"), std::string::npos);
	EXPECT_FALSE(
		Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "textures": { "1map": "a.png" } })", fs::path{}, error)
			.has_value());
	EXPECT_NE(error.find("1map"), std::string::npos);
}

// Descriptor Files
// ----------------
TEST(MaterialLibraryTest, FindsOnlyTheDescriptorFilesOfADirectorySorted)
{
	const fs::path dir = fs::temp_directory_path() / "engine_unit_tests_MaterialFindsOnlyTheDescriptorFilesOfADirectorySorted";
	fs::remove_all(dir);
	fs::create_directories(dir / "nested");
	for (const char* file : { "b.material.json", "a.material.json", "c.shader.json", "nested/d.material.json" })
		std::ofstream{ dir / file } << "{}";

	const auto files = Material_Library::find_descriptor_files(dir);

	ASSERT_EQ(files.size(), 2u);
	EXPECT_EQ(files[0].filename(), "a.material.json");
	EXPECT_EQ(files[1].filename(), "b.material.json");

	std::error_code error;
	fs::remove_all(dir, error);
}

// Engine Materials
// ----------------
TEST(MaterialLibraryTest, TheEngineMaterialDescriptorsAreValidAndUseExistingShaders)
{
	// texture slots of the engine's shaders, by shader name
	std::map<std::string, std::vector<std::string>> shader_texture_slots;
	for (const auto& file : Shader_Library::find_descriptor_files(fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "shaders"))
	{
		std::string error;
		const auto  descriptor = Shader_Library::parse_descriptor(read_file(file), file.parent_path(), error);
		ASSERT_TRUE(descriptor.has_value()) << file << ": " << error;
		shader_texture_slots[descriptor->name] = descriptor->texture_slots;
	}

	// every material must be valid, have a unique name, and use one of the engine's shaders
	const auto files = Material_Library::find_descriptor_files(fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "materials");
	ASSERT_FALSE(files.empty());
	std::set<std::string> material_names;
	for (const auto& file : files)
	{
		std::string error;
		const auto  descriptor = Material_Library::parse_descriptor(read_file(file), file.parent_path(), error);
		ASSERT_TRUE(descriptor.has_value()) << file << ": " << error;
		EXPECT_TRUE(material_names.insert(descriptor->name).second) << "Duplicate material name: " << descriptor->name;
		ASSERT_TRUE(shader_texture_slots.contains(descriptor->shader_name)) << descriptor->name << " uses an unknown shader";

		// every texture must exist and be for a texture slot of the material's shader
		const auto& slots = shader_texture_slots.at(descriptor->shader_name);
		for (const auto& [slot, path] : descriptor->texture_paths)
		{
			EXPECT_NE(std::find(slots.begin(), slots.end(), slot), slots.end()) << descriptor->name << ": unknown texture slot " << slot;
			EXPECT_TRUE(fs::exists(path)) << descriptor->name << ": " << path;
		}
	}

	// the default material, used by the meshes without a material of their own, must exist
	EXPECT_TRUE(material_names.contains(Material_Library::DEFAULT_MATERIAL));
}
