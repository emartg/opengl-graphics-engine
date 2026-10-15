/*
 * AssetReferencesTest.cpp
 * This file contains the unit tests of the references between assets that the operations of the Asset_Registry class
 * keep valid (the relative texture paths of the materials when a texture or a material is moved or renamed, alone or
 * inside a folder), of the protection of what is inside the folder of a model, and of the search of the assets that
 * use another one (find_users). They use temporary directories and need no OpenGL context.
 */

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/asset/Asset_Registry.h"
#include "core/material/Material_Library.h"

namespace fs = std::filesystem;

// A fixture with a temporary assets directory, where tests create textures, materials, and models
class AssetReferencesTest : public ::testing::Test
{
protected:
	fs::path       root;
	Asset_Registry registry;
	fs::path       result;
	std::string    error;

	void SetUp() override
	{
		// use the test name, so that tests running in parallel do not share their directories
		const std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

		root = fs::absolute(fs::temp_directory_path() / ("engine_unit_tests_asset_references_" + test_name)).lexically_normal();
		fs::remove_all(root);
		fs::create_directories(root);
	}

	void TearDown() override
	{
		std::error_code ignored; // ignore errors, so that a cleanup failure does not hide the test result
		fs::remove_all(root, ignored);
	}

	void create_file(const std::string& relative_path)
	{
		const fs::path path = root / relative_path;
		fs::create_directories(path.parent_path());
		std::ofstream{ path } << "content";
	}

	// Creates a material file whose textures are files of the directory (slot -> path relative to the directory)
	void create_material(const std::string& relative_path, const std::map<std::string, std::string>& textures)
	{
		Material_Descriptor descriptor;
		descriptor.name        = "Test " + relative_path;
		descriptor.shader_name = "Lit";
		for (const auto& [slot, texture] : textures) descriptor.texture_paths[slot] = root / texture;

		const fs::path file = root / relative_path;
		fs::create_directories(file.parent_path());
		std::ofstream{ file, std::ios::binary } << Material_Library::write_descriptor(descriptor, file.parent_path());
	}

	std::string read_text(const std::string& relative_path) const
	{
		std::ifstream      stream{ root / relative_path, std::ios::binary };
		std::ostringstream buffer;
		buffer << stream.rdbuf();
		return buffer.str();
	}

	// Returns the texture of a slot of a material file, as a path relative to the directory ("" if there is none)
	std::string texture_of(const std::string& material, const std::string& slot) const
	{
		std::string error_text;
		const auto  descriptor = Material_Library::parse_descriptor(read_text(material), (root / material).parent_path(), error_text);
		if (!descriptor || !descriptor->texture_paths.contains(slot))
			return "";
		return descriptor->texture_paths.at(slot).lexically_normal().lexically_relative(root).generic_string();
	}

	void scan() { ASSERT_TRUE(registry.scan(root)); }

	static std::vector<std::string> as_text(const std::vector<fs::path>& paths)
	{
		std::vector<std::string> texts;
		for (const fs::path& path : paths) texts.push_back(path.generic_string());
		return texts;
	}
};

// Moving and Renaming Textures
// ----------------------------
TEST_F(AssetReferencesTest, MovingATextureUpdatesTheMaterialsThatUseIt)
{
	create_file("textures/wood.png");
	create_file("textures/stone.png");
	fs::create_directories(root / "archive");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" }, { "opacity_map", "textures/stone.png" } });
	create_material("materials/pine.material.json", { { "albedo_map", "textures/wood.png" } });
	scan();
	const std::string untouched = read_text("materials/oak.material.json");

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	EXPECT_TRUE(registry.get_warnings().empty());
	EXPECT_EQ(texture_of("materials/oak.material.json", "albedo_map"), "archive/wood.png");
	EXPECT_EQ(texture_of("materials/oak.material.json", "opacity_map"), "textures/stone.png"); // the other texture stays
	EXPECT_EQ(texture_of("materials/pine.material.json", "albedo_map"), "archive/wood.png");
	EXPECT_NE(read_text("materials/oak.material.json"), untouched);
	EXPECT_NE(read_text("materials/oak.material.json").find("../archive/wood.png"), std::string::npos); // relative paths
}

TEST_F(AssetReferencesTest, RenamingATextureUpdatesTheMaterialsThatUseIt)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	scan();

	ASSERT_TRUE(registry.rename("textures/wood.png", "oak_wood", result, error)) << error;
	EXPECT_EQ(texture_of("materials/oak.material.json", "albedo_map"), "textures/oak_wood.png");
}

TEST_F(AssetReferencesTest, MovingOrRenamingAFolderOfTexturesUpdatesTheMaterialsThatUseThem)
{
	create_file("textures/wood/a.png");
	create_file("textures/wood/dark/b.png");
	fs::create_directories(root / "archive");
	create_material(
		"materials/oak.material.json",
		{ { "albedo_map", "textures/wood/a.png" }, { "normal_map", "textures/wood/dark/b.png" } });
	scan();

	ASSERT_TRUE(registry.rename("textures/wood", "oak", result, error)) << error;
	EXPECT_EQ(texture_of("materials/oak.material.json", "albedo_map"), "textures/oak/a.png");
	EXPECT_EQ(texture_of("materials/oak.material.json", "normal_map"), "textures/oak/dark/b.png");

	ASSERT_TRUE(registry.move("textures/oak", "archive", result, error)) << error;
	EXPECT_EQ(texture_of("materials/oak.material.json", "albedo_map"), "archive/oak/a.png");
	EXPECT_EQ(texture_of("materials/oak.material.json", "normal_map"), "archive/oak/dark/b.png");
}

// Moving and Renaming Materials
// -----------------------------
TEST_F(AssetReferencesTest, MovingAMaterialRewritesItsRelativePathsSoThatItKeepsItsTextures)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	fs::create_directories(root / "ui" / "deep");
	scan();
	const std::uint32_t id = registry.find_by_path("materials/oak.material.json")->id;

	ASSERT_TRUE(registry.move("materials/oak.material.json", "ui/deep", result, error)) << error;
	EXPECT_EQ(texture_of("ui/deep/oak.material.json", "albedo_map"), "textures/wood.png"); // still the same texture
	EXPECT_NE(read_text("ui/deep/oak.material.json").find("../../textures/wood.png"), std::string::npos);
	EXPECT_EQ(registry.find_by_path("ui/deep/oak.material.json")->id, id);
}

TEST_F(AssetReferencesTest, RenamingAMaterialLeavesItsFileAsItWas)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	scan();
	const std::string before = read_text("materials/oak.material.json");

	ASSERT_TRUE(registry.rename("materials/oak.material.json", "pine", result, error)) << error;
	EXPECT_EQ(read_text("materials/pine.material.json"), before); // the folder is the same, so the paths are too
}

TEST_F(AssetReferencesTest, MovingAFolderWithAMaterialAndItsTextureKeepsThemTogether)
{
	create_file("pack/textures/wood.png");
	create_material("pack/oak.material.json", { { "albedo_map", "pack/textures/wood.png" } });
	fs::create_directories(root / "archive");
	scan();

	ASSERT_TRUE(registry.move("pack", "archive", result, error)) << error;
	EXPECT_EQ(texture_of("archive/pack/oak.material.json", "albedo_map"), "archive/pack/textures/wood.png");
	EXPECT_TRUE(fs::exists(root / "archive" / "pack" / "textures" / "wood.png"));
}

TEST_F(AssetReferencesTest, MovingAFolderWithAMaterialKeepsItsTexturesThatAreOutsideTheFolder)
{
	create_file("textures/wood.png");
	create_material("pack/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	fs::create_directories(root / "archive");
	scan();

	ASSERT_TRUE(registry.move("pack", "archive", result, error)) << error;
	EXPECT_EQ(texture_of("archive/pack/oak.material.json", "albedo_map"), "textures/wood.png");
	EXPECT_NE(read_text("archive/pack/oak.material.json").find("../../textures/wood.png"), std::string::npos);
}

TEST_F(AssetReferencesTest, LeavesTheMaterialsThatAreNotAffectedAsTheyWere)
{
	create_file("textures/wood.png");
	create_file("textures/stone.png");
	create_file("other/x.png");
	fs::create_directories(root / "archive");
	create_material("materials/stone.material.json", { { "albedo_map", "textures/stone.png" } });
	create_material("materials/plain.material.json", {});
	scan();
	const std::string stone = read_text("materials/stone.material.json");
	const std::string plain = read_text("materials/plain.material.json");

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	ASSERT_TRUE(registry.move("other/x.png", "archive", result, error)) << error;
	EXPECT_EQ(read_text("materials/stone.material.json"), stone);
	EXPECT_EQ(read_text("materials/plain.material.json"), plain);
}

TEST_F(AssetReferencesTest, ReportsTheMaterialsThatCannotBeUpdatedAndStillDoesTheOperation)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	create_file("materials/broken.material.json"); // its text is not a descriptor
	fs::create_directories(root / "archive");
	scan();

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	EXPECT_TRUE(fs::exists(root / "archive" / "wood.png"));
	EXPECT_EQ(texture_of("materials/oak.material.json", "albedo_map"), "archive/wood.png"); // the valid one is updated
	ASSERT_EQ(registry.get_warnings().size(), 1u);
	EXPECT_NE(registry.get_warnings()[0].find("materials/broken.material.json"), std::string::npos) << registry.get_warnings()[0];

	// the warnings are about the last operation only
	ASSERT_TRUE(registry.rename("archive/wood.png", "oak", result, error)) << error;
	EXPECT_EQ(registry.get_warnings().size(), 1u); // the broken file is still there
	fs::remove(root / "materials" / "broken.material.json");
	scan();
	ASSERT_TRUE(registry.rename("archive/oak.png", "pine", result, error)) << error;
	EXPECT_TRUE(registry.get_warnings().empty());
}

TEST_F(AssetReferencesTest, KeepsTheIdentifiersOfTheMaterialsThatAreUpdated)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	fs::create_directories(root / "archive");
	scan();
	const std::uint32_t id = registry.find_by_path("materials/oak.material.json")->id;

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	EXPECT_EQ(registry.find_by_path("materials/oak.material.json")->id, id);
}

// The Folder of a Model
// ---------------------
TEST_F(AssetReferencesTest, ProtectsWhatIsInsideTheFolderOfAModelButNotTheFolderItself)
{
	create_file("models/teapot/Teapot.obj");
	create_file("models/teapot/textures/pot.png");
	create_file("models/teapot/textures/deep/more.png");
	fs::create_directories(root / "archive");
	scan();

	for (const char* item :
		 { "models/teapot/Teapot.obj", "models/teapot/textures/pot.png", "models/teapot/textures", "models/teapot/textures/deep/more.png" })
	{
		EXPECT_FALSE(registry.rename(item, "x", result, error)) << item;
		EXPECT_NE(error.find("model"), std::string::npos) << error;
		EXPECT_FALSE(registry.move(item, "archive", result, error)) << item;
		EXPECT_TRUE(fs::exists(root / item)) << item;
	}

	// the folder of the model, and the folders above it, are free to move and rename, with everything inside
	ASSERT_TRUE(registry.rename("models/teapot", "pot", result, error)) << error;
	EXPECT_TRUE(fs::exists(root / "models" / "pot" / "textures" / "pot.png"));
	ASSERT_TRUE(registry.move("models", "archive", result, error)) << error;
	EXPECT_TRUE(fs::exists(root / "archive" / "models" / "pot" / "Teapot.obj"));
}

TEST_F(AssetReferencesTest, AModelThatIsInTheDirectoryItselfDoesNotProtectTheRest)
{
	create_file("loose.obj");
	create_file("textures/wood.png");
	fs::create_directories(root / "archive");
	scan();

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	ASSERT_TRUE(registry.rename("loose.obj", "free", result, error)) << error;
}

TEST_F(AssetReferencesTest, ADeletedItemInsideTheFolderOfAModelIsAllowed)
{
	create_file("models/teapot/Teapot.obj");
	create_file("models/teapot/textures/pot.png");
	scan();

	ASSERT_TRUE(registry.remove("models/teapot/textures/pot.png", error)) << error; // the owner can decide to delete it
	EXPECT_FALSE(fs::exists(root / "models" / "teapot" / "textures" / "pot.png"));
}

// Finding the Users of an Asset
// -----------------------------
TEST_F(AssetReferencesTest, FindsTheMaterialsThatUseATextureOrAFolderOfTextures)
{
	create_file("textures/wood.png");
	create_file("textures/stone/rock.png");
	create_file("textures/unused.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	create_material("materials/pine.material.json", { { "albedo_map", "textures/wood.png" }, { "normal_map", "textures/stone/rock.png" } });
	create_material("materials/rock.material.json", { { "albedo_map", "textures/stone/rock.png" } });
	create_material("materials/plain.material.json", {});
	scan();

	EXPECT_EQ(
		as_text(registry.find_users("textures/wood.png")),
		(std::vector<std::string>{ "materials/oak.material.json", "materials/pine.material.json" }));
	EXPECT_EQ(
		as_text(registry.find_users("textures/stone/rock.png")),
		(std::vector<std::string>{ "materials/pine.material.json", "materials/rock.material.json" }));
	EXPECT_EQ(
		as_text(registry.find_users("textures/stone")),
		(std::vector<std::string>{ "materials/pine.material.json", "materials/rock.material.json" })); // a folder
	EXPECT_EQ(as_text(registry.find_users("textures")).size(), 3u); // every material that has a texture inside it
	EXPECT_TRUE(registry.find_users("textures/unused.png").empty());
	EXPECT_TRUE(registry.find_users("materials/oak.material.json").empty()); // nothing uses a material (the scene is not an asset)
}

TEST_F(AssetReferencesTest, DoesNotCountTheMaterialsThatAreInsideTheItemItself)
{
	create_file("pack/textures/wood.png");
	create_material("pack/oak.material.json", { { "albedo_map", "pack/textures/wood.png" } });
	scan();

	EXPECT_TRUE(registry.find_users("pack").empty());
	EXPECT_EQ(registry.find_users("pack/textures").size(), 1u); // the material is outside that folder, and uses it
}

TEST_F(AssetReferencesTest, FindsTheModelThatMayUseWhatIsInsideItsFolder)
{
	create_file("models/teapot/Teapot.obj");
	create_file("models/teapot/textures/pot.png");
	create_file("models/teapot/Other.obj");
	scan();

	EXPECT_EQ(
		as_text(registry.find_users("models/teapot/textures/pot.png")),
		(std::vector<std::string>{ "models/teapot/Other.obj", "models/teapot/Teapot.obj" }));
	EXPECT_TRUE(registry.find_users("models/teapot").empty()); // the folder of the model goes with its model
}

TEST_F(AssetReferencesTest, FindsNoUsersForAPathThatIsNotAnAssetOrAFolder)
{
	create_file("textures/wood.png");
	scan();

	EXPECT_TRUE(registry.find_users("missing.png").empty());
	EXPECT_TRUE(registry.find_users("").empty());
	EXPECT_TRUE(registry.find_users("..").empty());
}

TEST_F(AssetReferencesTest, FindsTheUsersWhateverTheSpellingOfThePath)
{
	create_file("textures/wood.png");
	create_material("materials/oak.material.json", { { "albedo_map", "textures/wood.png" } });
	scan();

	EXPECT_EQ(registry.find_users("./textures/../textures/wood.png").size(), 1u);
}
