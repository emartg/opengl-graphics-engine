/*
 * MaterialLibraryTest.cpp
 * This file contains the unit tests of the Material_Library class that do not require an OpenGL context:
 * the parsing of material descriptors (fields, parameter types, and textures), the search of descriptor files,
 * the writing and saving of descriptors, the names of new descriptor files, the duplication and renaming of
 * materials, and the validity of the descriptors of the engine's own materials.
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
#include "core/shader/Shader.h"
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

TEST(MaterialLibraryTest, ParsesTheEnvironmentModeAndResolution)
{
	std::string error;
	const auto  none = Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S" })", fs::path{}, error);
	ASSERT_TRUE(none.has_value()) << error;
	EXPECT_EQ(none->environment_mode, Environment_Mode::NONE);
	EXPECT_EQ(none->environment_resolution, Material::DEFAULT_ENVIRONMENT_RESOLUTION);

	const auto skybox = Material_Library::parse_descriptor(R"({ "name": "T", "shader": "S", "environment": "skybox" })", fs::path{}, error);
	ASSERT_TRUE(skybox.has_value()) << error;
	EXPECT_EQ(skybox->environment_mode, Environment_Mode::SKYBOX);

	const auto dynamic = Material_Library::parse_descriptor(
		R"({ "name": "T", "shader": "S", "environment": "dynamic", "environment_resolution": 256 })",
		fs::path{},
		error);
	ASSERT_TRUE(dynamic.has_value()) << error;
	EXPECT_EQ(dynamic->environment_mode, Environment_Mode::DYNAMIC);
	EXPECT_EQ(dynamic->environment_resolution, 256u);
}

TEST(MaterialLibraryTest, RejectsInvalidEnvironmentModesAndResolutions)
{
	for (const char* fields : { R"("environment": "cubemap")",
								R"("environment": true)",
								R"("environment_resolution": 8)",
								R"("environment_resolution": 8192)",
								R"("environment_resolution": 512.5)",
								R"("environment_resolution": "512")" })
	{
		std::string       error;
		const std::string text = std::string(R"({ "name": "T", "shader": "S", )") + fields + " }";
		EXPECT_FALSE(Material_Library::parse_descriptor(text, fs::path{}, error).has_value()) << fields;
		EXPECT_NE(error.find("environment"), std::string::npos) << error;
	}
}

TEST(MaterialLibraryTest, SetEnvironmentRejectsResolutionsOutOfRange)
{
	Material material{ "T", nullptr };

	EXPECT_TRUE(material.set_environment(Environment_Mode::DYNAMIC, 256));
	EXPECT_FALSE(material.set_environment(Environment_Mode::SKYBOX, 0));
	EXPECT_FALSE(material.set_environment(Environment_Mode::SKYBOX, Material::MAX_ENVIRONMENT_RESOLUTION + 1));

	// a rejected change keeps the previous environment
	EXPECT_EQ(material.get_environment_mode(), Environment_Mode::DYNAMIC);
	EXPECT_EQ(material.get_environment_resolution(), 256u);
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

// Descriptor Writing
// ------------------
TEST(MaterialLibraryTest, WritesADescriptorThatParsesBackToTheSameDescriptor)
{
	const fs::path      base_dir = fs::temp_directory_path() / "engine_unit_tests_materials";
	Material_Descriptor descriptor;
	descriptor.name                   = "Test";
	descriptor.shader_name            = "Test Shader";
	descriptor.supports_instancing    = true;
	descriptor.environment_mode       = Environment_Mode::DYNAMIC;
	descriptor.environment_resolution = 256;
	descriptor.parameters             = {
		{ "ratio", 0.6579f },
		{ "enabled", true },
		{ "offset", glm::vec2{ 1.5f, -2.0f } },
		{ "tint", glm::vec3{ 0.1f, 0.2f, 0.3f } },
		{ "color", glm::vec4{ 1.0f, 0.5f, 0.25f, 0.125f } },
	};
	descriptor.texture_paths = { { "albedo_map", base_dir / ".." / "textures" / "wood.png" } };

	const std::string text = Material_Library::write_descriptor(descriptor, base_dir);
	std::string       error;
	const auto        parsed = Material_Library::parse_descriptor(text, base_dir, error);

	ASSERT_TRUE(parsed.has_value()) << error << "\n" << text;
	EXPECT_EQ(parsed->name, descriptor.name);
	EXPECT_EQ(parsed->shader_name, descriptor.shader_name);
	EXPECT_TRUE(parsed->supports_instancing);
	EXPECT_EQ(parsed->environment_mode, Environment_Mode::DYNAMIC);
	EXPECT_EQ(parsed->environment_resolution, 256u);
	EXPECT_EQ(parsed->parameters, descriptor.parameters); // the floats are read back exactly
	// the texture is written relative to the descriptor's directory, with forward slashes
	EXPECT_NE(text.find("\"../textures/wood.png\""), std::string::npos) << text;
	ASSERT_TRUE(parsed->texture_paths.contains("albedo_map"));
	EXPECT_EQ(
		parsed->texture_paths.at("albedo_map").lexically_normal(),
		(base_dir.parent_path() / "textures" / "wood.png").lexically_normal());
}

TEST(MaterialLibraryTest, WritesTheFloatsWithTheirShortestRepresentation)
{
	Material_Descriptor descriptor;
	descriptor.name        = "Test";
	descriptor.shader_name = "S";
	descriptor.parameters  = { { "ratio", 0.6579f }, { "shininess", 32.0f } };

	const std::string text = Material_Library::write_descriptor(descriptor, fs::path{});

	EXPECT_NE(text.find("\"ratio\": 0.6579,\n"), std::string::npos) << text;
	EXPECT_NE(text.find("\"shininess\": 32.0\n"), std::string::npos) << text;
}

TEST(MaterialLibraryTest, WritesOnlyTheOptionalFieldsThatAreNotDefaults)
{
	Material_Descriptor descriptor;
	descriptor.name        = "Test";
	descriptor.shader_name = "S";

	const std::string text = Material_Library::write_descriptor(descriptor, fs::path{});

	EXPECT_EQ(text, "{\n  \"name\": \"Test\",\n  \"shader\": \"S\"\n}\n");
}

TEST(MaterialLibraryTest, WritesInvalidUtf8AsReplacementCharacters)
{
	Material_Descriptor descriptor;
	descriptor.name        = "Caf\xE9"; // "Café" in Latin-1, which is not valid UTF-8
	descriptor.shader_name = "S";

	const std::string text = Material_Library::write_descriptor(descriptor, fs::path{});
	std::string       error;
	const auto        parsed = Material_Library::parse_descriptor(text, fs::path{}, error);

	ASSERT_TRUE(parsed.has_value()) << error;
	EXPECT_EQ(parsed->name, "Caf\xEF\xBF\xBD"); // U+FFFD in UTF-8
}

TEST(MaterialLibraryTest, ConvertsAMaterialToItsDescriptor)
{
	auto     shader = std::make_shared<Shader>("Test Shader", "test.vert.glsl", "test.frag.glsl");
	Material material{ "Test", shader, { { "shininess", 16.0f } }, true };
	material.set_environment(Environment_Mode::SKYBOX);

	std::string error;
	const auto  descriptor = Material_Library::to_descriptor(material, error);

	ASSERT_TRUE(descriptor.has_value()) << error;
	EXPECT_EQ(descriptor->name, "Test");
	EXPECT_EQ(descriptor->shader_name, "Test Shader");
	EXPECT_EQ(descriptor->parameters, material.get_parameters());
	EXPECT_TRUE(descriptor->supports_instancing);
	EXPECT_EQ(descriptor->environment_mode, Environment_Mode::SKYBOX);
	EXPECT_TRUE(descriptor->texture_paths.empty());

	// a material without a shader has no descriptor
	EXPECT_FALSE(Material_Library::to_descriptor(Material{ "No Shader", nullptr }, error).has_value());
	EXPECT_NE(error.find("shader"), std::string::npos) << error;
}

TEST(MaterialLibraryTest, SavesAMaterialToADescriptorFileThatBecomesItsFile)
{
	const fs::path dir = fs::temp_directory_path() / "engine_unit_tests_MaterialSavesAMaterialToADescriptorFile";
	fs::remove_all(dir);
	fs::create_directories(dir);
	auto     shader = std::make_shared<Shader>("Test Shader", "test.vert.glsl", "test.frag.glsl");
	Material material{ "Test", shader, { { "shininess", 16.0f } } };

	std::string error;
	ASSERT_TRUE(Material_Library::save(material, dir / "test.material.json", error)) << error;

	EXPECT_EQ(material.get_file_path(), dir / "test.material.json");
	const auto parsed = Material_Library::parse_descriptor(read_file(dir / "test.material.json"), dir, error);
	ASSERT_TRUE(parsed.has_value()) << error;
	EXPECT_EQ(parsed->name, "Test");
	EXPECT_EQ(parsed->parameters, material.get_parameters());

	// a file that cannot be written (in a directory that does not exist) is reported, and keeps the material's file
	EXPECT_FALSE(Material_Library::save(material, dir / "missing" / "test.material.json", error));
	EXPECT_EQ(material.get_file_path(), dir / "test.material.json");

	std::error_code remove_error;
	fs::remove_all(dir, remove_error);
}

// Descriptor File Names
// ---------------------
TEST(MaterialLibraryTest, MakesTheDescriptorFileNameFromTheMaterialName)
{
	EXPECT_EQ(Material_Library::make_descriptor_file_name("Default"), "default.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name("Dynamic Glass"), "dynamic_glass.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name("Teapot (2)"), "teapot_2.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name("  a--b  "), "a_b.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name("../x"), "x.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name(""), "material.material.json");
	EXPECT_EQ(Material_Library::make_descriptor_file_name("\xC3\xA9"), "material.material.json"); // only non-ASCII letters
}

TEST(MaterialLibraryTest, MakesANewDescriptorPathThatDoesNotReplaceAnExistingFile)
{
	const fs::path dir = fs::temp_directory_path() / "engine_unit_tests_MaterialMakesANewDescriptorPath";
	fs::remove_all(dir);
	fs::create_directories(dir);

	EXPECT_EQ(Material_Library::make_new_descriptor_path("Glass", dir), dir / "glass.material.json");
	std::ofstream{ dir / "glass.material.json" } << "{}";
	EXPECT_EQ(Material_Library::make_new_descriptor_path("Glass", dir), dir / "glass_2.material.json");
	std::ofstream{ dir / "glass_2.material.json" } << "{}";
	EXPECT_EQ(Material_Library::make_new_descriptor_path("Glass", dir), dir / "glass_3.material.json");

	std::error_code error;
	fs::remove_all(dir, error);
}

// Library Operations
// ------------------
TEST(MaterialLibraryTest, DuplicatesAMaterialWithAUniqueNameAndWithoutItsFile)
{
	Material_Library library;
	auto             original = library.add(std::make_shared<Material>("Glass", nullptr, Material_Parameters{ { "ratio", 0.5f } }, true));
	original->set_environment(Environment_Mode::DYNAMIC, 256);
	original->set_file_path("glass.material.json");

	const auto copy = library.duplicate(*original);

	EXPECT_EQ(copy->get_name(), "Glass (2)");
	EXPECT_NE(copy, original);
	EXPECT_EQ(copy->get_parameters(), original->get_parameters());
	EXPECT_TRUE(copy->get_supports_instancing());
	EXPECT_EQ(copy->get_environment_mode(), Environment_Mode::DYNAMIC);
	EXPECT_EQ(copy->get_environment_resolution(), 256u);
	EXPECT_TRUE(copy->get_file_path().empty());
	EXPECT_EQ(library.get("Glass (2)"), copy);

	// the copy is independent of the original
	copy->set_parameter("ratio", 0.75f);
	EXPECT_EQ(std::get<float>(original->get_parameters().at("ratio")), 0.5f);
}

TEST(MaterialLibraryTest, RenamesAMaterialOnlyToAFreeNonEmptyName)
{
	Material_Library library;
	auto             glass       = library.add(std::make_shared<Material>("Glass", nullptr));
	auto             chrome      = library.add(std::make_shared<Material>("Chrome", nullptr));
	auto             default_one = library.add(std::make_shared<Material>(Material_Library::DEFAULT_MATERIAL, nullptr));
	std::string      error;

	EXPECT_TRUE(library.rename(*glass, "Frosted Glass", error)) << error;
	EXPECT_EQ(glass->get_name(), "Frosted Glass");
	EXPECT_EQ(library.get("Frosted Glass"), glass);
	EXPECT_TRUE(library.rename(*glass, "Frosted Glass", error)); // the same name

	EXPECT_FALSE(library.rename(*glass, "Chrome", error)); // the name of another material
	EXPECT_FALSE(library.rename(*glass, "", error));
	EXPECT_EQ(glass->get_name(), "Frosted Glass");

	// the default material keeps its name, since the renderer finds it by its name
	EXPECT_FALSE(library.rename(*default_one, "Plain", error));
	EXPECT_EQ(default_one->get_name(), Material_Library::DEFAULT_MATERIAL);
}

TEST(MaterialLibraryTest, DoesNotReloadAMaterialWithoutAFile)
{
	Material_Library library;
	Shader_Library   shader_library;
	auto             material = library.add(std::make_shared<Material>("New", nullptr, Material_Parameters{ { "ratio", 0.5f } }));

	std::string error;
	EXPECT_FALSE(library.reload(*material, shader_library, error));
	EXPECT_NE(error.find("no descriptor file"), std::string::npos) << error;
	EXPECT_EQ(material->get_parameters().size(), 1u); // unchanged
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

TEST(MaterialLibraryTest, WritingTheEngineMaterialDescriptorsReproducesTheirFiles)
{
	// saving an engine material without changes must not change its file (e.g., in the Material Editor)
	for (const auto& file : Material_Library::find_descriptor_files(fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "materials"))
	{
		std::string error;
		const auto  descriptor = Material_Library::parse_descriptor(read_file(file), file.parent_path(), error);
		ASSERT_TRUE(descriptor.has_value()) << file << ": " << error;
		EXPECT_EQ(Material_Library::write_descriptor(*descriptor, file.parent_path()), read_file(file)) << file;
	}
}
