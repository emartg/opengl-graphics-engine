/*
 * ShaderLibraryTest.cpp
 * This file contains the unit tests of the Shader_Library class that do not require an OpenGL context:
 * the parsing of shader descriptors, the search of descriptor files, and the validity of the descriptors
 * of the engine's own shaders.
 */

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/shader/Shader_Library.h"

namespace fs = std::filesystem;

// Descriptor Parsing
// ------------------
TEST(ShaderLibraryTest, ParsesADescriptorAndResolvesItsFilesRelativeToTheBaseDirectory)
{
	std::string error;
	const auto  descriptor = Shader_Library::parse_descriptor(
		R"({ "name": "Test Shader", "vertex": "test.vert.glsl", "fragment": "test.frag.glsl" })",
		fs::path{ "shaders" },
		error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_EQ(descriptor->name, "Test Shader");
	EXPECT_EQ(descriptor->vertex_path, fs::path("shaders") / "test.vert.glsl");
	EXPECT_EQ(descriptor->fragment_path, fs::path("shaders") / "test.frag.glsl");
	EXPECT_TRUE(descriptor->geometry_path.empty()); // the geometry stage is optional
}

TEST(ShaderLibraryTest, ParsesTheOptionalGeometryStage)
{
	std::string error;
	const auto  descriptor = Shader_Library::parse_descriptor(
		R"({ "name": "Test", "vertex": "a.vert", "geometry": "a.geom", "fragment": "a.frag" })",
		fs::path{ "dir" },
		error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_EQ(descriptor->geometry_path, fs::path("dir") / "a.geom");
}

TEST(ShaderLibraryTest, ParsesTheOptionalTextureSlotsInOrder)
{
	std::string error;
	const auto  descriptor = Shader_Library::parse_descriptor(
		R"({ "name": "T", "vertex": "a.vert", "fragment": "a.frag", "textures": [ "albedo_map", "opacity_map" ] })",
		fs::path{},
		error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_EQ(descriptor->texture_slots, (std::vector<std::string>{ "albedo_map", "opacity_map" }));
}

TEST(ShaderLibraryTest, RejectsInvalidRepeatedOrNonIdentifierTextureSlots)
{
	for (const char* slots : { R"("albedo_map")", "[1]", R"([""])", R"(["albedo_map", "albedo_map"])", R"(["base-color"])", R"(["1map"])" })
	{
		std::string       error;
		const std::string text = std::string(R"({ "name": "T", "vertex": "a.vert", "fragment": "a.frag", "textures": )") + slots + " }";
		EXPECT_FALSE(Shader_Library::parse_descriptor(text, fs::path{}, error).has_value()) << slots;
		EXPECT_FALSE(error.empty());
	}
}

TEST(ShaderLibraryTest, RejectsInvalidJson)
{
	std::string error;
	EXPECT_FALSE(Shader_Library::parse_descriptor(R"({ "name": "Test", )", fs::path{}, error).has_value());
	EXPECT_FALSE(error.empty());
}

TEST(ShaderLibraryTest, RejectsAJsonValueThatIsNotAnObject)
{
	std::string error;
	EXPECT_FALSE(Shader_Library::parse_descriptor(R"(["name", "vertex", "fragment"])", fs::path{}, error).has_value());
	EXPECT_FALSE(error.empty());
}

TEST(ShaderLibraryTest, RejectsADescriptorWithoutARequiredField)
{
	for (const char* field : { "name", "vertex", "fragment" })
	{
		// build a descriptor with every required field except one
		std::string text = "{";
		for (const char* other : { "name", "vertex", "fragment" })
			if (std::string(other) != field)
				text += std::string(text.size() > 1 ? ", " : "") + "\"" + other + "\": \"value\"";
		text += "}";

		std::string error;
		EXPECT_FALSE(Shader_Library::parse_descriptor(text, fs::path{}, error).has_value()) << text;
		EXPECT_NE(error.find(field), std::string::npos) << error;
	}
}

TEST(ShaderLibraryTest, RejectsFieldsThatAreNotNonEmptyStrings)
{
	std::string error;
	EXPECT_FALSE(Shader_Library::parse_descriptor(R"({ "name": 1, "vertex": "a", "fragment": "b" })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("name"), std::string::npos);
	EXPECT_FALSE(Shader_Library::parse_descriptor(R"({ "name": "a", "vertex": "", "fragment": "b" })", fs::path{}, error).has_value());
	EXPECT_NE(error.find("vertex"), std::string::npos);
}

// Descriptor Files
// ----------------
TEST(ShaderLibraryTest, FindsOnlyTheDescriptorFilesOfADirectorySorted)
{
	const fs::path dir = fs::temp_directory_path() / "engine_unit_tests_FindsOnlyTheDescriptorFilesOfADirectorySorted";
	fs::remove_all(dir);
	fs::create_directories(dir / "nested");
	for (const char* file : { "b.shader.json", "a.shader.json", "c.vert.glsl", "settings.json", "nested/d.shader.json" })
		std::ofstream{ dir / file } << "{}";

	const auto files = Shader_Library::find_descriptor_files(dir);

	ASSERT_EQ(files.size(), 2u);
	EXPECT_EQ(files[0].filename(), "a.shader.json");
	EXPECT_EQ(files[1].filename(), "b.shader.json");

	std::error_code error;
	fs::remove_all(dir, error);
}

TEST(ShaderLibraryTest, FindsNoDescriptorFilesInAMissingDirectory)
{
	EXPECT_TRUE(Shader_Library::find_descriptor_files(fs::temp_directory_path() / "engine_unit_tests_missing_directory").empty());
}

// Engine Shaders
// --------------
TEST(ShaderLibraryTest, TheEngineShaderDescriptorsAreValidAndTheirFilesExist)
{
	const fs::path shaders_dir = fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "shaders";
	const auto     files       = Shader_Library::find_descriptor_files(shaders_dir);
	ASSERT_FALSE(files.empty());

	std::set<std::string> names;
	for (const auto& file : files)
	{
		std::ifstream     stream{ file };
		std::stringstream text;
		text << stream.rdbuf();

		std::string error;
		const auto  descriptor = Shader_Library::parse_descriptor(text.str(), file.parent_path(), error);
		ASSERT_TRUE(descriptor.has_value()) << file << ": " << error;

		EXPECT_TRUE(names.insert(descriptor->name).second) << "Duplicate shader name: " << descriptor->name;
		EXPECT_TRUE(fs::exists(descriptor->vertex_path)) << descriptor->vertex_path;
		EXPECT_TRUE(fs::exists(descriptor->fragment_path)) << descriptor->fragment_path;
		if (!descriptor->geometry_path.empty())
			EXPECT_TRUE(fs::exists(descriptor->geometry_path)) << descriptor->geometry_path;
	}
}
