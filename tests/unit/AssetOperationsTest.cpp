/*
 * AssetOperationsTest.cpp
 * This file contains the unit tests of the operations of the Asset_Registry class on the files of an assets directory
 * (creating, renaming, moving, and deleting folders and assets): the validity of names, unique names, the refusal of
 * paths that are not assets or that leave the directory, the identifiers that follow the moved items, and the
 * synchronization of the registry with the files. They use temporary directories and need no OpenGL context.
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/asset/Asset_Registry.h"

namespace fs = std::filesystem;

// Names
// -----
TEST(AssetOperationsTest, AcceptsOrdinaryNamesAndRefusesTheOnesThatFileSystemsDoNotAccept)
{
	for (const char* name : { "wood", "Wood 2", "a.b", "dynamic_glass", "v1.2", "(copy)", "n" })
		EXPECT_TRUE(Asset_Registry::is_valid_name(name)) << name;

	const std::string backslash(1, '\\');
	for (const std::string& name : std::vector<std::string>{ "",
															 ".",
															 "..",
															 ".hidden",
															 " space",
															 "space ",
															 "dot.",
															 "a/b",
															 "a" + backslash + "b",
															 "a<b",
															 "a>b",
															 "a:b",
															 "a\"b",
															 "a|b",
															 "a?b",
															 "a*b",
															 std::string("a") + '\n' + "b",
															 std::string("a") + '\t' })
		EXPECT_FALSE(Asset_Registry::is_valid_name(name)) << "[" << name << "]";

	EXPECT_TRUE(Asset_Registry::is_valid_name(std::string(Asset_Registry::MAX_NAME_LENGTH, 'a')));
	EXPECT_FALSE(Asset_Registry::is_valid_name(std::string(Asset_Registry::MAX_NAME_LENGTH + 1, 'a')));
}

// A fixture with a temporary assets directory that tests fill with files
class AssetOperationsFilesTest : public ::testing::Test
{
protected:
	fs::path       root;
	Asset_Registry registry;
	fs::path       result; // the path that an operation returns
	std::string    error;  // the reason that an operation fails

	void SetUp() override
	{
		// use the test name, so that tests running in parallel do not share their directories
		const std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

		root = fs::temp_directory_path() / ("engine_unit_tests_asset_operations_" + test_name);
		fs::remove_all(root);
		fs::create_directories(root);
	}

	void TearDown() override
	{
		std::error_code ignored; // ignore errors, so that a cleanup failure does not hide the test result
		fs::remove_all(root, ignored);
	}

	// Creates a file (and its folders) inside the directory, and scans the directory again
	void create_file(const std::string& relative_path)
	{
		const fs::path path = root / relative_path;
		fs::create_directories(path.parent_path());
		std::ofstream{ path } << "content";
	}

	void create_cubemap(const std::string& relative_folder)
	{
		for (const char* face : { "posx", "negx", "posy", "negy", "posz", "negz" }) create_file(relative_folder + "/" + face + ".jpg");
	}

	void scan() { ASSERT_TRUE(registry.scan(root)); }

	bool exists(const std::string& relative_path) const { return fs::exists(root / relative_path); }

	// Returns the identifier of the asset at a path (0 if there is none)
	std::uint32_t id_of(const std::string& relative_path) const
	{
		const Asset* asset = registry.find_by_path(relative_path);
		return asset ? asset->id : 0;
	}

	bool has_folder(const std::string& relative_path) const
	{
		const auto& folders = registry.get_folders();
		return std::find(folders.begin(), folders.end(), fs::path{ relative_path }) != folders.end();
	}
};

// Unique Names
// ------------
TEST_F(AssetOperationsFilesTest, KeepsAFreeNameAndNumbersTheOnesThatAreUsed)
{
	create_file("textures/wood.png");
	create_file("textures/wood (2).png");
	create_file("materials/metal.material.json");
	fs::create_directories(root / "textures" / "v1.2");
	scan();

	EXPECT_EQ(registry.get_unique_name("textures", "stone.png"), "stone.png");
	EXPECT_EQ(registry.get_unique_name("textures", "wood.png"), "wood (3).png"); // (2) is used too
	EXPECT_EQ(
		registry.get_unique_name("materials", "metal.material.json"),
		"metal (2).material.json");                                      // the descriptor extension stays whole
	EXPECT_EQ(registry.get_unique_name("textures", "v1.2"), "v1.2 (2)"); // a folder has no extension
	EXPECT_EQ(registry.get_unique_name("", "textures"), "textures (2)");
	EXPECT_EQ(registry.get_unique_name("missing", "x.png"), "x.png");
}

// Creating Folders
// ----------------
TEST_F(AssetOperationsFilesTest, CreatesAFolderInTheDirectoryAndInASubfolder)
{
	fs::create_directories(root / "textures");
	scan();

	ASSERT_TRUE(registry.create_folder("", "models", result, error)) << error;
	EXPECT_EQ(result, fs::path{ "models" });
	EXPECT_TRUE(fs::is_directory(root / "models"));
	EXPECT_TRUE(has_folder("models")); // the registry is up to date

	ASSERT_TRUE(registry.create_folder("textures", "wood", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/wood");
	EXPECT_TRUE(has_folder("textures/wood"));

	ASSERT_TRUE(registry.create_folder("textures/wood", "dark", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/wood/dark");
}

TEST_F(AssetOperationsFilesTest, RefusesToCreateAFolderWithAnInvalidNameAnUsedNameOrAParentThatIsNotAFolder)
{
	create_file("textures/wood.png");
	create_cubemap("skyboxes/meadow");
	scan();

	EXPECT_FALSE(registry.create_folder("", "a/b", result, error));
	EXPECT_NE(error.find("not a valid"), std::string::npos) << error;
	EXPECT_FALSE(registry.create_folder("", ".hidden", result, error));
	EXPECT_FALSE(registry.create_folder("", "", result, error));

	EXPECT_FALSE(registry.create_folder("", "textures", result, error)); // a folder with that name exists
	EXPECT_NE(error.find("already"), std::string::npos) << error;
	EXPECT_FALSE(registry.create_folder("textures", "wood.png", result, error)); // a file with that name exists

	EXPECT_FALSE(registry.create_folder("missing", "x", result, error));           // the parent does not exist
	EXPECT_FALSE(registry.create_folder("textures/wood.png", "x", result, error)); // the parent is a file
	EXPECT_FALSE(registry.create_folder("skyboxes/meadow", "x", result, error));   // the parent is a skybox

	EXPECT_FALSE(exists("textures/a"));
	EXPECT_FALSE(exists("skyboxes/meadow/x"));
}

// Renaming
// --------
TEST_F(AssetOperationsFilesTest, RenamesAnAssetKeepingItsExtensionAndItsIdentifier)
{
	create_file("textures/wood.png");
	create_file("materials/metal.material.json");
	scan();
	const std::uint32_t texture_id  = id_of("textures/wood.png");
	const std::uint32_t material_id = id_of("materials/metal.material.json");

	ASSERT_TRUE(registry.rename("textures/wood.png", "oak", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/oak.png");
	EXPECT_TRUE(exists("textures/oak.png"));
	EXPECT_FALSE(exists("textures/wood.png"));
	EXPECT_EQ(registry.find_by_path("textures/wood.png"), nullptr);
	ASSERT_NE(registry.find_by_path("textures/oak.png"), nullptr);
	EXPECT_EQ(registry.find_by_path("textures/oak.png")->name, "oak");
	EXPECT_EQ(id_of("textures/oak.png"), texture_id); // the asset keeps its identifier

	ASSERT_TRUE(registry.rename("materials/metal.material.json", "steel", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "materials/steel.material.json"); // the descriptor extension stays whole
	EXPECT_EQ(id_of("materials/steel.material.json"), material_id);
	EXPECT_EQ(registry.find_by_path("materials/steel.material.json")->type, Asset_Type::MATERIAL);
}

TEST_F(AssetOperationsFilesTest, RenamesAFolderWithItsContentAndTheIdentifiersInsideIt)
{
	create_file("textures/wood/a.png");
	create_file("textures/wood/dark/b.png");
	create_cubemap("textures/wood/sky");
	scan();
	const std::uint32_t id_a   = id_of("textures/wood/a.png");
	const std::uint32_t id_b   = id_of("textures/wood/dark/b.png");
	const std::uint32_t id_sky = id_of("textures/wood/sky");

	ASSERT_TRUE(registry.rename("textures/wood", "oak", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/oak");
	EXPECT_FALSE(exists("textures/wood"));
	EXPECT_TRUE(has_folder("textures/oak"));
	EXPECT_TRUE(has_folder("textures/oak/dark"));
	EXPECT_EQ(id_of("textures/oak/a.png"), id_a);
	EXPECT_EQ(id_of("textures/oak/dark/b.png"), id_b);
	EXPECT_EQ(id_of("textures/oak/sky"), id_sky); // a skybox folder is an asset, and keeps its identifier too
}

TEST_F(AssetOperationsFilesTest, RenamesASkyboxFolderAsAnAsset)
{
	create_cubemap("skyboxes/meadow");
	scan();
	const std::uint32_t id = id_of("skyboxes/meadow");

	ASSERT_TRUE(registry.rename("skyboxes/meadow", "field", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "skyboxes/field");
	EXPECT_EQ(id_of("skyboxes/field"), id);
	EXPECT_EQ(registry.find_by_path("skyboxes/field")->type, Asset_Type::SKYBOX);
}

TEST_F(AssetOperationsFilesTest, TheNewNameOfAnAssetDoesNotChangeItsExtension)
{
	create_file("textures/wood.png");
	scan();

	// the extension is not part of the name: writing it again keeps the old one and makes it part of the name
	ASSERT_TRUE(registry.rename("textures/wood.png", "oak.jpg", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/oak.jpg.png");
}

TEST_F(AssetOperationsFilesTest, RenamingToTheSameNameChangesNothing)
{
	create_file("textures/wood.png");
	scan();
	const std::uint32_t id = id_of("textures/wood.png");

	ASSERT_TRUE(registry.rename("textures/wood.png", "wood", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/wood.png");
	EXPECT_EQ(id_of("textures/wood.png"), id);
}

TEST_F(AssetOperationsFilesTest, RenamesAnItemToTheSameNameWrittenWithAnotherCase)
{
	create_file("textures/wood.png");
	scan();
	const std::uint32_t id = id_of("textures/wood.png");

	ASSERT_TRUE(registry.rename("textures/wood.png", "Wood", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "textures/Wood.png");
	EXPECT_EQ(id_of("textures/Wood.png"), id);
	// the file really has the new case (the file system may ignore the case when it looks a file up)
	bool found = false;
	for (const auto& entry : fs::directory_iterator{ root / "textures" }) found = found || entry.path().filename().string() == "Wood.png";
	EXPECT_TRUE(found);
}

TEST_F(AssetOperationsFilesTest, RefusesToRenameWithAnInvalidNameOrOverAnotherItem)
{
	create_file("textures/wood.png");
	create_file("textures/stone.png");
	fs::create_directories(root / "textures" / "oak");
	scan();

	EXPECT_FALSE(registry.rename("textures/wood.png", "a/b", result, error));
	EXPECT_NE(error.find("not a valid"), std::string::npos) << error;
	EXPECT_FALSE(registry.rename("textures/wood.png", ".hidden", result, error));
	EXPECT_FALSE(registry.rename("textures/wood.png", "", result, error));

	EXPECT_FALSE(registry.rename("textures/wood.png", "stone", result, error)); // another asset has that name
	EXPECT_NE(error.find("already"), std::string::npos) << error;
	EXPECT_TRUE(exists("textures/wood.png"));
	EXPECT_TRUE(exists("textures/stone.png"));
}

TEST_F(AssetOperationsFilesTest, RefusesToRenameAFolderOverAnotherFolder)
{
	fs::create_directories(root / "a");
	fs::create_directories(root / "b");
	scan();

	EXPECT_FALSE(registry.rename("a", "b", result, error));
	EXPECT_TRUE(exists("a"));
	EXPECT_TRUE(exists("b"));
}

// Moving
// ------
TEST_F(AssetOperationsFilesTest, MovesAnAssetIntoAnotherFolderAndTheDirectoryKeepingItsIdentifier)
{
	create_file("textures/wood.png");
	fs::create_directories(root / "archive");
	scan();
	const std::uint32_t id = id_of("textures/wood.png");

	ASSERT_TRUE(registry.move("textures/wood.png", "archive", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "archive/wood.png");
	EXPECT_TRUE(exists("archive/wood.png"));
	EXPECT_FALSE(exists("textures/wood.png"));
	EXPECT_EQ(id_of("archive/wood.png"), id);
	EXPECT_EQ(registry.find_by_path("textures/wood.png"), nullptr);

	ASSERT_TRUE(registry.move("archive/wood.png", "", result, error)) << error; // into the directory itself
	EXPECT_EQ(result.generic_string(), "wood.png");
	EXPECT_EQ(id_of("wood.png"), id);
}

TEST_F(AssetOperationsFilesTest, MovesAFolderWithItsContentAndTheIdentifiersInsideIt)
{
	create_file("a/x/one.png");
	create_file("a/x/deep/two.png");
	create_cubemap("a/x/sky");
	fs::create_directories(root / "b");
	scan();
	const std::uint32_t id_one = id_of("a/x/one.png");
	const std::uint32_t id_two = id_of("a/x/deep/two.png");
	const std::uint32_t id_sky = id_of("a/x/sky");

	ASSERT_TRUE(registry.move("a/x", "b", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "b/x");
	EXPECT_FALSE(exists("a/x"));
	EXPECT_TRUE(has_folder("b/x/deep"));
	EXPECT_EQ(id_of("b/x/one.png"), id_one);
	EXPECT_EQ(id_of("b/x/deep/two.png"), id_two);
	EXPECT_EQ(id_of("b/x/sky"), id_sky);
}

TEST_F(AssetOperationsFilesTest, MovesASkyboxFolderAsAnAsset)
{
	create_cubemap("skyboxes/meadow");
	fs::create_directories(root / "archive");
	scan();
	const std::uint32_t id = id_of("skyboxes/meadow");

	ASSERT_TRUE(registry.move("skyboxes/meadow", "archive", result, error)) << error;
	EXPECT_EQ(id_of("archive/meadow"), id);
}

TEST_F(AssetOperationsFilesTest, RefusesToMoveAFolderIntoItselfOrIntoItsSubfolders)
{
	fs::create_directories(root / "a" / "b" / "c");
	scan();

	EXPECT_FALSE(registry.move("a", "a", result, error));
	EXPECT_NE(error.find("itself"), std::string::npos) << error;
	EXPECT_FALSE(registry.move("a", "a/b", result, error));
	EXPECT_FALSE(registry.move("a", "a/b/c", result, error));
	EXPECT_TRUE(exists("a/b/c"));
}

TEST_F(AssetOperationsFilesTest, RefusesToMoveToAnInvalidDestinationOrOverAnotherItem)
{
	create_file("textures/wood.png");
	create_file("archive/wood.png");
	create_file("notes.txt");
	create_cubemap("skyboxes/meadow");
	scan();

	EXPECT_FALSE(registry.move("textures/wood.png", "archive", result, error)); // another item has that name
	EXPECT_NE(error.find("already"), std::string::npos) << error;
	EXPECT_TRUE(exists("textures/wood.png"));

	EXPECT_FALSE(registry.move("textures/wood.png", "missing", result, error));
	EXPECT_FALSE(registry.move("textures/wood.png", "notes.txt", result, error));       // a file is not a folder
	EXPECT_FALSE(registry.move("textures/wood.png", "skyboxes/meadow", result, error)); // a skybox is not a folder of assets
	EXPECT_FALSE(registry.move("textures/wood.png", "textures", result, error));        // it is already there
	EXPECT_NE(error.find("already in"), std::string::npos) << error;
}

// Deleting
// --------
TEST_F(AssetOperationsFilesTest, DeletesAnAssetAFolderWithItsContentAndASkybox)
{
	create_file("textures/wood.png");
	create_file("textures/stone.png");
	create_file("models/teapot/Teapot.obj");
	create_file("models/teapot/textures/pot.png");
	create_cubemap("skyboxes/meadow");
	scan();

	ASSERT_TRUE(registry.remove("textures/wood.png", error)) << error;
	EXPECT_FALSE(exists("textures/wood.png"));
	EXPECT_TRUE(exists("textures/stone.png")); // only that asset
	EXPECT_EQ(registry.find_by_path("textures/wood.png"), nullptr);

	ASSERT_TRUE(registry.remove("models/teapot", error)) << error;
	EXPECT_FALSE(exists("models/teapot"));
	EXPECT_EQ(registry.find_by_path("models/teapot/Teapot.obj"), nullptr);
	EXPECT_TRUE(has_folder("models")); // its parent stays

	ASSERT_TRUE(registry.remove("skyboxes/meadow", error)) << error;
	EXPECT_FALSE(exists("skyboxes/meadow"));
	EXPECT_EQ(registry.find_by_path("skyboxes/meadow"), nullptr);
}

TEST_F(AssetOperationsFilesTest, AnIdentifierIsNotGivenToAnotherAssetAfterItsAssetIsDeleted)
{
	create_file("a.png");
	scan();
	const std::uint32_t old_id = id_of("a.png");

	ASSERT_TRUE(registry.remove("a.png", error)) << error;
	EXPECT_EQ(registry.find_by_id(old_id), nullptr);
	create_file("b.png");
	scan();
	EXPECT_NE(id_of("b.png"), old_id);
}

// Paths That Are Refused
// ----------------------
TEST_F(AssetOperationsFilesTest, NeverTouchesAnythingThatIsNotAnAssetOrAFolderOfTheAssets)
{
	create_file("textures/wood.png");
	create_file("textures/readme.txt"); // unknown file
	create_file(".cache/thumb.png");    // hidden folder
	create_file("textures/.hidden.png");
	const fs::path outside = root.parent_path() / (root.filename().string() + "_outside");
	fs::create_directories(outside);
	std::ofstream{ outside / "secret.png" } << "secret";
	scan();

	const std::vector<fs::path> refused{ "textures/readme.txt",
										 ".cache",
										 ".cache/thumb.png",
										 "textures/.hidden.png",
										 "missing.png",
										 "",
										 ".",
										 "..",
										 "../" + outside.filename().string(),
										 outside,
										 outside / "secret.png",
										 "textures/../../" + outside.filename().string(),
										 "textures/..",
										 "textures/../.." };
	for (const fs::path& path : refused)
	{
		EXPECT_FALSE(registry.rename(path, "x", result, error)) << path;
		EXPECT_FALSE(registry.move(path, "textures", result, error)) << path;
		EXPECT_FALSE(registry.remove(path, error)) << path;
	}

	// the destination and the parent are checked too
	EXPECT_FALSE(registry.move("textures/wood.png", outside, result, error));
	EXPECT_FALSE(registry.move("textures/wood.png", "..", result, error));
	EXPECT_FALSE(registry.create_folder(outside, "x", result, error));
	EXPECT_FALSE(registry.create_folder("..", "x", result, error));

	EXPECT_TRUE(exists("textures/readme.txt"));
	EXPECT_TRUE(exists(".cache/thumb.png"));
	EXPECT_TRUE(exists("textures/.hidden.png"));
	EXPECT_TRUE(fs::exists(outside / "secret.png"));
	EXPECT_TRUE(exists("textures/wood.png"));

	std::error_code ignored;
	fs::remove_all(outside, ignored);
}

TEST_F(AssetOperationsFilesTest, RefusesEveryOperationWithoutAScannedDirectory)
{
	Asset_Registry empty;
	EXPECT_FALSE(empty.create_folder("", "x", result, error));
	EXPECT_FALSE(empty.rename("a.png", "b", result, error));
	EXPECT_FALSE(empty.move("a.png", "", result, error));
	EXPECT_FALSE(empty.remove("a.png", error));
}

TEST_F(AssetOperationsFilesTest, AcceptsThePathsInAnyOfTheirSpellings)
{
	create_file("textures/wood.png");
	fs::create_directories(root / "archive");
	scan();

	ASSERT_TRUE(registry.move("./textures/../textures/wood.png", "archive/", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "archive/wood.png");
	ASSERT_TRUE(registry.rename(fs::path("archive") / "wood.png", "oak", result, error)) << error;
	EXPECT_EQ(result.generic_string(), "archive/oak.png");
}

TEST_F(AssetOperationsFilesTest, TheRegistryStaysInSyncWithTheFilesAfterEveryOperation)
{
	create_file("a/one.png");
	scan();

	ASSERT_TRUE(registry.create_folder("", "b", result, error)) << error;
	ASSERT_TRUE(registry.move("a/one.png", "b", result, error)) << error;
	ASSERT_TRUE(registry.rename("b/one.png", "two", result, error)) << error;

	Asset_Registry fresh; // a registry that scans the same directory sees the same assets and folders
	ASSERT_TRUE(fresh.scan(root));
	ASSERT_EQ(fresh.get_assets().size(), registry.get_assets().size());
	for (std::size_t i = 0; i < fresh.get_assets().size(); ++i) EXPECT_EQ(fresh.get_assets()[i].path, registry.get_assets()[i].path);
	EXPECT_EQ(fresh.get_folders(), registry.get_folders());
	EXPECT_EQ(registry.get_root(), root);
}
