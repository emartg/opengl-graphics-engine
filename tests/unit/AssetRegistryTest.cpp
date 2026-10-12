/*
 * AssetRegistryTest.cpp
 * This file contains the unit tests of the Asset_Registry class, which needs no OpenGL context: the recognition
 * of the type of an asset from the name of its file, the scan of an assets directory (assets, folders, names,
 * hidden files, and skybox folders), the identifiers of the assets, and the queries by path, type, and folder.
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/asset/Asset_Registry.h"
#include "core/material/Material_Library.h"

namespace fs = std::filesystem;

// Type of an Asset From the Name of a File
// ----------------------------------------
TEST(AssetRegistryTest, RecognizesEveryKindOfAssetFileByItsName)
{
	struct Case
	{
		const char* file;
		Asset_Type  type;
	};
	const std::vector<Case> cases{
		{ "metal.material.json", Asset_Type::MATERIAL },
		{ "cube.shape.json", Asset_Type::SHAPE },
		{ "sun.light.json", Asset_Type::LIGHT },
		{ "meadow.skybox.json", Asset_Type::SKYBOX },
		{ "sky.hdr", Asset_Type::SKYBOX },
		{ "teapot.obj", Asset_Type::MODEL },
		{ "teapot.gltf", Asset_Type::MODEL },
		{ "teapot.glb", Asset_Type::MODEL },
		{ "teapot.fbx", Asset_Type::MODEL },
		{ "teapot.dae", Asset_Type::MODEL },
		{ "teapot.3ds", Asset_Type::MODEL },
		{ "teapot.stl", Asset_Type::MODEL },
		{ "teapot.ply", Asset_Type::MODEL },
		{ "bricks.png", Asset_Type::TEXTURE },
		{ "bricks.jpg", Asset_Type::TEXTURE },
		{ "bricks.jpeg", Asset_Type::TEXTURE },
		{ "bricks.bmp", Asset_Type::TEXTURE },
		{ "bricks.tga", Asset_Type::TEXTURE },
	};

	for (const Case& test_case : cases)
	{
		const auto type = Asset_Registry::get_type_from_file(test_case.file);
		ASSERT_TRUE(type.has_value()) << test_case.file;
		EXPECT_EQ(*type, test_case.type) << test_case.file;
	}
}

TEST(AssetRegistryTest, ComparesTheExtensionsWithoutConsideringTheCase)
{
	EXPECT_EQ(Asset_Registry::get_type_from_file("Teapot.OBJ"), Asset_Type::MODEL);
	EXPECT_EQ(Asset_Registry::get_type_from_file("Sky.HDR"), Asset_Type::SKYBOX);
	EXPECT_EQ(Asset_Registry::get_type_from_file("Metal.Material.JSON"), Asset_Type::MATERIAL);
	EXPECT_EQ(Asset_Registry::get_type_from_file("BRICKS.PNG"), Asset_Type::TEXTURE);
}

TEST(AssetRegistryTest, DoesNotRecognizeTheFilesThatAreNotAssets)
{
	for (const char* file : { "readme.txt",
							  "teapot.mtl",
							  "teapot.bin",
							  "data.json",
							  "shader.glsl",
							  "no_extension",
							  ".hidden",
							  "metal.material",
							  "material.json" })
		EXPECT_FALSE(Asset_Registry::get_type_from_file(file).has_value()) << file;
}

TEST(AssetRegistryTest, UsesOnlyTheFileNameToRecognizeAnAssetNotItsFolders)
{
	EXPECT_EQ(Asset_Registry::get_type_from_file("models/obj.d/teapot.png"), Asset_Type::TEXTURE);
	EXPECT_FALSE(Asset_Registry::get_type_from_file("textures.png/readme").has_value());
}

TEST(AssetRegistryTest, UsesTheSameMaterialExtensionAsTheMaterialLibrary)
{
	EXPECT_STREQ(Asset_Registry::MATERIAL_EXTENSION, Material_Library::DESCRIPTOR_EXTENSION);
}

TEST(AssetRegistryTest, NamesEveryTypeOfAsset)
{
	std::set<std::string> names;
	for (const Asset_Type type :
		 { Asset_Type::MATERIAL, Asset_Type::MODEL, Asset_Type::SHAPE, Asset_Type::LIGHT, Asset_Type::SKYBOX, Asset_Type::TEXTURE })
	{
		const std::string name = Asset_Registry::to_string(type);
		EXPECT_FALSE(name.empty());
		EXPECT_NE(name, "Unknown");
		names.insert(name);
	}
	EXPECT_EQ(names.size(), 6u); // every name is different
}

// Scanning a Directory
// --------------------
// A fixture with a temporary directory that tests fill with files
class AssetRegistryScanTest : public ::testing::Test
{
protected:
	fs::path       root;
	Asset_Registry registry;

	void SetUp() override
	{
		// use the test name, so that tests running in parallel do not share their directories
		const std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

		root = fs::temp_directory_path() / ("engine_unit_tests_assets_" + test_name);
		fs::remove_all(root);
		fs::create_directories(root);
	}

	void TearDown() override
	{
		std::error_code error; // ignore errors, so that a cleanup failure does not hide the test result
		fs::remove_all(root, error);
	}

	// Creates a file (and its folders) inside the directory
	void create_file(const std::string& relative_path) const
	{
		const fs::path path = root / relative_path;
		fs::create_directories(path.parent_path());
		std::ofstream{ path } << "content";
	}

	// Creates the six images of a cubemap inside a folder
	void create_cubemap(const std::string& relative_folder, const std::string& extension = ".jpg") const
	{
		for (const char* face : { "posx", "negx", "posy", "negy", "posz", "negz" }) create_file(relative_folder + "/" + face + extension);
	}

	// Returns the paths of all the assets of the registry as text, to compare them
	std::vector<std::string> all_paths() const
	{
		std::vector<std::string> paths;
		for (const Asset& asset : registry.get_assets()) paths.push_back(asset.path.generic_string());
		return paths;
	}

	// Returns the paths of some assets as text, to compare them
	static std::vector<std::string> paths_of(const std::vector<const Asset*>& assets)
	{
		std::vector<std::string> paths;
		for (const Asset* asset : assets) paths.push_back(asset->path.generic_string());
		return paths;
	}
};

TEST_F(AssetRegistryScanTest, FailsAndStaysEmptyWhenThePathIsNotADirectory)
{
	create_file("file.png");
	EXPECT_FALSE(registry.scan(root / "missing"));
	EXPECT_FALSE(registry.scan(root / "file.png"));
	EXPECT_TRUE(registry.get_assets().empty());
	EXPECT_TRUE(registry.get_folders().empty());
	EXPECT_TRUE(registry.get_root().empty());
}

TEST_F(AssetRegistryScanTest, FindsTheAssetsOfEveryTypeWithTheirNamesAndPaths)
{
	create_file("materials/dynamic_glass.material.json");
	create_file("models/teapot/Teapot.OBJ");
	create_file("shapes/cube.shape.json");
	create_file("lights/sun.light.json");
	create_file("skyboxes/sky.hdr");
	create_file("textures/a.b.png");

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(registry.get_root(), root);
	ASSERT_EQ(registry.get_assets().size(), 6u);

	const Asset* material = registry.find_by_path("materials/dynamic_glass.material.json");
	ASSERT_NE(material, nullptr);
	EXPECT_EQ(material->type, Asset_Type::MATERIAL);
	EXPECT_EQ(material->name, "dynamic_glass"); // the descriptor extension is removed as a whole

	const Asset* model = registry.find_by_path("models/teapot/Teapot.OBJ");
	ASSERT_NE(model, nullptr);
	EXPECT_EQ(model->type, Asset_Type::MODEL);
	EXPECT_EQ(model->name, "Teapot");

	EXPECT_EQ(registry.find_by_path("shapes/cube.shape.json")->name, "cube");
	EXPECT_EQ(registry.find_by_path("lights/sun.light.json")->type, Asset_Type::LIGHT);
	EXPECT_EQ(registry.find_by_path("skyboxes/sky.hdr")->type, Asset_Type::SKYBOX);
	EXPECT_EQ(registry.find_by_path("textures/a.b.png")->name, "a.b"); // only the last extension is removed
}

TEST_F(AssetRegistryScanTest, IgnoresUnknownFilesAndHiddenFilesAndFolders)
{
	create_file("textures/wood.png");
	create_file("textures/readme.txt");
	create_file("models/teapot.mtl");
	create_file("textures/.hidden.png");
	create_file(".cache/thumbnail.png");
	create_file("textures/.thumbs/small.png");

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(paths_of(registry.get_assets_of_type(Asset_Type::TEXTURE)), (std::vector<std::string>{ "textures/wood.png" }));
	EXPECT_EQ(registry.get_assets().size(), 1u);
	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "models", "textures" })); // no hidden folders
}

TEST_F(AssetRegistryScanTest, SortsTheAssetsAndTheFoldersByPath)
{
	for (const char* file : { "textures/b.png", "models/z.obj", "textures/a.png", "materials/m.material.json", "textures/sub/c.png" })
		create_file(file);

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(
		paths_of(registry.get_assets_of_type(Asset_Type::TEXTURE)),
		(std::vector<std::string>{ "textures/a.png", "textures/b.png", "textures/sub/c.png" }));

	const std::vector<std::string> paths = all_paths();
	EXPECT_TRUE(std::is_sorted(paths.begin(), paths.end()));

	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "materials", "models", "textures", "textures/sub" }));
}

TEST_F(AssetRegistryScanTest, ListsTheFoldersIncludingTheEmptyOnesButNotTheRoot)
{
	fs::create_directories(root / "empty");
	fs::create_directories(root / "a" / "b" / "c");

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "a", "a/b", "a/b/c", "empty" }));
	EXPECT_TRUE(registry.get_assets().empty());
}

// Skybox Folders
// --------------
TEST_F(AssetRegistryScanTest, RecognizesAFolderWithTheSixFacesOfACubemapAsASkybox)
{
	create_cubemap("skyboxes/meadow");
	create_file("skyboxes/meadow/readme.txt");

	ASSERT_TRUE(registry.scan(root));
	ASSERT_EQ(registry.get_assets().size(), 1u); // the six images are not listed as textures
	const Asset& skybox = registry.get_assets().front();
	EXPECT_EQ(skybox.type, Asset_Type::SKYBOX);
	EXPECT_EQ(skybox.name, "meadow");
	EXPECT_EQ(skybox.path.generic_string(), "skyboxes/meadow");
	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "skyboxes" })); // the skybox is not a folder of assets
}

TEST_F(AssetRegistryScanTest, RecognizesTheFacesWithAnyCaseAndImageExtension)
{
	for (const char* face : { "PosX.PNG", "negx.jpg", "POSY.jpeg", "negy.bmp", "posz.tga", "NegZ.png" })
		create_file(std::string("sky/") + face);
	EXPECT_TRUE(Asset_Registry::is_skybox_folder(root / "sky"));
}

TEST_F(AssetRegistryScanTest, DoesNotRecognizeAnIncompleteSetOfFacesAsASkybox)
{
	create_cubemap("sky");
	fs::remove(root / "sky" / "negz.jpg");
	EXPECT_FALSE(Asset_Registry::is_skybox_folder(root / "sky"));

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(registry.get_assets_of_type(Asset_Type::SKYBOX).size(), 0u);
	EXPECT_EQ(registry.get_assets_of_type(Asset_Type::TEXTURE).size(), 5u); // the images are textures then
	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "sky" }));
}

TEST_F(AssetRegistryScanTest, DoesNotTakeSixImagesWithOtherNamesForASkybox)
{
	for (const char* face : { "1.jpg", "2.jpg", "3.jpg", "4.jpg", "5.jpg", "6.jpg" }) create_file(std::string("sky/") + face);
	EXPECT_FALSE(Asset_Registry::is_skybox_folder(root / "sky"));
	EXPECT_FALSE(Asset_Registry::is_skybox_folder(root / "missing"));
}

// Identifiers
// -----------
TEST_F(AssetRegistryScanTest, GivesEveryAssetAUniqueIdentifierThatFindsIt)
{
	create_file("a.png");
	create_file("b.png");
	create_cubemap("sky");

	ASSERT_TRUE(registry.scan(root));
	std::set<std::uint32_t> ids;
	for (const Asset& asset : registry.get_assets())
	{
		EXPECT_NE(asset.id, 0u);
		EXPECT_TRUE(ids.insert(asset.id).second);
		EXPECT_EQ(registry.find_by_id(asset.id), &asset);
	}
	EXPECT_EQ(registry.find_by_id(0), nullptr);
	EXPECT_EQ(registry.find_by_id(9999), nullptr);
}

TEST_F(AssetRegistryScanTest, KeepsTheIdentifiersOfTheKnownAssetsWhenScanningAgain)
{
	create_file("a.png");
	create_file("b.png");
	ASSERT_TRUE(registry.scan(root));
	const std::uint32_t id_a = registry.find_by_path("a.png")->id;
	const std::uint32_t id_b = registry.find_by_path("b.png")->id;

	create_file("c.png"); // a new asset
	fs::remove(root / "a.png");
	ASSERT_TRUE(registry.scan(root));

	EXPECT_EQ(registry.find_by_path("a.png"), nullptr);
	EXPECT_EQ(registry.find_by_id(id_a), nullptr); // the removed asset is gone
	EXPECT_EQ(registry.find_by_path("b.png")->id, id_b);
	const std::uint32_t id_c = registry.find_by_path("c.png")->id;
	EXPECT_NE(id_c, id_a); // an identifier is never given to another asset
	EXPECT_NE(id_c, id_b);
	EXPECT_GT(id_c, id_b);

	create_file("a.png"); // the same path again
	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(registry.find_by_path("a.png")->id, id_a);
}

TEST_F(AssetRegistryScanTest, KeepsTheIdentifiersAfterClearing)
{
	create_file("a.png");
	ASSERT_TRUE(registry.scan(root));
	const std::uint32_t id = registry.find_by_path("a.png")->id;

	registry.clear();
	EXPECT_TRUE(registry.get_assets().empty());
	EXPECT_TRUE(registry.get_folders().empty());
	EXPECT_TRUE(registry.get_root().empty());

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(registry.find_by_path("a.png")->id, id);
}

// Queries
// -------
TEST_F(AssetRegistryScanTest, FindsAnAssetByPathWhateverTheSpellingOfThePath)
{
	create_file("textures/wood.png");
	ASSERT_TRUE(registry.scan(root));

	EXPECT_NE(registry.find_by_path("textures/wood.png"), nullptr);
	EXPECT_NE(registry.find_by_path("textures/./wood.png"), nullptr);
	EXPECT_NE(registry.find_by_path("textures/../textures/wood.png"), nullptr);
	EXPECT_NE(registry.find_by_path(fs::path("textures") / "wood.png"), nullptr);
	EXPECT_EQ(registry.find_by_path("textures"), nullptr); // a folder is not an asset
	EXPECT_EQ(registry.find_by_path("textures/Missing.png"), nullptr);
	EXPECT_EQ(registry.find_by_path(""), nullptr);
}

TEST_F(AssetRegistryScanTest, FindsTheAssetsOfAType)
{
	create_file("a.png");
	create_file("m.material.json");
	create_file("n.material.json");

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(
		paths_of(registry.get_assets_of_type(Asset_Type::MATERIAL)),
		(std::vector<std::string>{ "m.material.json", "n.material.json" }));
	EXPECT_EQ(registry.get_assets_of_type(Asset_Type::TEXTURE).size(), 1u);
	EXPECT_TRUE(registry.get_assets_of_type(Asset_Type::LIGHT).empty());
}

TEST_F(AssetRegistryScanTest, FindsTheAssetsOfAFolderWithOrWithoutItsSubfolders)
{
	for (const char* file : { "root.png", "textures/a.png", "textures/b.png", "textures/wood/c.png", "texturesque/d.png", "models/e.obj" })
		create_file(file);
	ASSERT_TRUE(registry.scan(root));

	EXPECT_EQ(paths_of(registry.get_assets_in_folder("")), (std::vector<std::string>{ "root.png" }));
	EXPECT_EQ(paths_of(registry.get_assets_in_folder("textures")), (std::vector<std::string>{ "textures/a.png", "textures/b.png" }));
	EXPECT_EQ(
		paths_of(registry.get_assets_in_folder("textures", true)),
		(std::vector<std::string>{ "textures/a.png", "textures/b.png", "textures/wood/c.png" })); // not "texturesque"
	EXPECT_EQ(paths_of(registry.get_assets_in_folder("textures/")), paths_of(registry.get_assets_in_folder("textures")));
	EXPECT_EQ(paths_of(registry.get_assets_in_folder("./textures/wood")), (std::vector<std::string>{ "textures/wood/c.png" }));
	EXPECT_EQ(registry.get_assets_in_folder("", true).size(), registry.get_assets().size());
	EXPECT_TRUE(registry.get_assets_in_folder("missing", true).empty());
}

TEST_F(AssetRegistryScanTest, ReplacesItsContentWhenScanningAnotherDirectory)
{
	create_file("a.png");
	ASSERT_TRUE(registry.scan(root));

	const fs::path other = root / "other";
	fs::create_directories(other);
	std::ofstream{ other / "b.png" } << "content";
	ASSERT_TRUE(registry.scan(other));

	EXPECT_EQ(registry.get_root(), other);
	EXPECT_EQ(paths_of(registry.get_assets_in_folder("")), (std::vector<std::string>{ "b.png" }));
	EXPECT_EQ(registry.find_by_path("a.png"), nullptr);

	EXPECT_FALSE(registry.scan(root / "missing")); // a failed scan leaves the registry empty
	EXPECT_TRUE(registry.get_assets().empty());
}

TEST_F(AssetRegistryScanTest, DoesNotFollowSymbolicLinks)
{
	create_file("real/a.png");
	std::error_code error;
	fs::create_directory_symlink(root / "real", root / "link", error);
	if (error)
		GTEST_SKIP() << "Symbolic links cannot be created here: " << error.message();

	ASSERT_TRUE(registry.scan(root));
	EXPECT_EQ(all_paths(), (std::vector<std::string>{ "real/a.png" })); // not listed again through the link
	EXPECT_EQ(registry.get_folders(), (std::vector<fs::path>{ "real" }));
}

// The Engine's Own Resources
// --------------------------
TEST(AssetRegistryTest, FindsEveryDescriptorOfTheEngineMaterials)
{
	const fs::path materials_dir = fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "materials";

	Asset_Registry registry;
	ASSERT_TRUE(registry.scan(materials_dir));

	const auto files = Material_Library::find_descriptor_files(materials_dir);
	ASSERT_FALSE(files.empty());
	EXPECT_EQ(registry.get_assets_of_type(Asset_Type::MATERIAL).size(), files.size());
	EXPECT_EQ(registry.get_assets().size(), files.size()); // nothing else is there
}

TEST(AssetRegistryTest, FindsTheCubemapSkyboxesOfTheEngineResources)
{
	const fs::path faces_dir = fs::path{ ENGINE_UNIT_TESTS_RESOURCES_DIR } / "textures" / "skyboxes" / "faces";

	Asset_Registry registry;
	ASSERT_TRUE(registry.scan(faces_dir));

	EXPECT_NE(registry.find_by_path("meadow"), nullptr);
	EXPECT_EQ(registry.find_by_path("meadow")->type, Asset_Type::SKYBOX);
	// the folder whose images are numbered instead of named is not a cubemap
	EXPECT_EQ(registry.find_by_path("union_square_INVALID"), nullptr);
	EXPECT_EQ(registry.get_assets_in_folder("union_square_INVALID").size(), 6u);
}
