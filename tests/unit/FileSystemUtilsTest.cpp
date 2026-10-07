/*
 * FileSystemUtilsTest.cpp
 * This file contains the unit tests of the File_System_Utils class: the location of the
 * running executable, and the selection of the first existing directory from a list of candidates.
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/utils/file_system/File_System_Utils.h"

namespace fs = std::filesystem;

// Executable Directory
// --------------------
TEST(FileSystemUtilsTest, get_executable_directory_returns_the_directory_of_the_test_executable)
{
	const fs::path executable_directory = File_System_Utils::get_executable_directory();

	ASSERT_FALSE(executable_directory.empty());
	EXPECT_TRUE(executable_directory.is_absolute());
	EXPECT_TRUE(fs::is_directory(executable_directory));
	EXPECT_TRUE(fs::exists(executable_directory / ENGINE_UNIT_TESTS_EXECUTABLE_NAME));
}

// First Existing Directory
// ------------------------
// Test fixture that creates a temporary directory tree for each test, and removes it afterwards:
// <temporary directory>/<test name>/{first, second, file.txt}
class FindFirstExistingDirectoryTest : public ::testing::Test
{
protected:
	fs::path root_dir;
	fs::path first_dir;
	fs::path second_dir;
	fs::path regular_file;
	fs::path missing_dir;

	void SetUp() override
	{
		// use the test name, so that tests running in parallel do not share their directories
		const std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

		root_dir     = fs::temp_directory_path() / ("engine_unit_tests_" + test_name);
		first_dir    = root_dir / "first";
		second_dir   = root_dir / "second";
		regular_file = root_dir / "file.txt";
		missing_dir  = root_dir / "missing";

		fs::remove_all(root_dir);
		fs::create_directories(first_dir);
		fs::create_directories(second_dir);
		std::ofstream{ regular_file } << "not a directory";
	}

	void TearDown() override
	{
		std::error_code error; // ignore errors, so that a cleanup failure does not hide the test result
		fs::remove_all(root_dir, error);
	}
};

TEST_F(FindFirstExistingDirectoryTest, returns_an_empty_path_when_there_are_no_candidates)
{
	EXPECT_TRUE(File_System_Utils::find_first_existing_directory({}).empty());
}

TEST_F(FindFirstExistingDirectoryTest, returns_an_empty_path_when_no_candidate_exists)
{
	EXPECT_TRUE(File_System_Utils::find_first_existing_directory({ missing_dir }).empty());
}

TEST_F(FindFirstExistingDirectoryTest, returns_the_first_existing_candidate_in_order)
{
	const fs::path result = File_System_Utils::find_first_existing_directory({ missing_dir, second_dir, first_dir });

	ASSERT_FALSE(result.empty());
	EXPECT_TRUE(fs::equivalent(result, second_dir));
}

TEST_F(FindFirstExistingDirectoryTest, skips_empty_paths_and_regular_files)
{
	const fs::path result = File_System_Utils::find_first_existing_directory({ fs::path{}, regular_file, first_dir });

	ASSERT_FALSE(result.empty());
	EXPECT_TRUE(fs::equivalent(result, first_dir));
}

TEST_F(FindFirstExistingDirectoryTest, normalizes_the_returned_path)
{
	// "second/../first" exists, and is returned as the canonical path of "first"
	const fs::path result = File_System_Utils::find_first_existing_directory({ second_dir / ".." / "first" });

	EXPECT_EQ(result, fs::canonical(first_dir));
}
