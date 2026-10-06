/*
 * String_Utils_Test.cpp
 * This file contains the unit tests of the String_Utils class: case conversion,
 * file path decomposition, and display name generation.
 */

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "core/Node.h"
#include "core/utils/string/String_Utils.h"

// Case Conversion
// ---------------
TEST(String_Utils_Test, to_uppercase_converts_only_letters)
{
	EXPECT_EQ(String_Utils::to_uppercase("Point_Light 2"), "POINT_LIGHT 2");
	EXPECT_EQ(String_Utils::to_uppercase(""), "");
}

TEST(String_Utils_Test, to_lowercase_converts_only_letters)
{
	EXPECT_EQ(String_Utils::to_lowercase("Point_Light 2"), "point_light 2");
	EXPECT_EQ(String_Utils::to_lowercase(""), "");
}

TEST(String_Utils_Test, conversions_from_copy_match_conversions_from_reference)
{
	const std::string input{ "Skybox.Vert.GLSL" };
	EXPECT_EQ(String_Utils::to_uppercase_from_copy(input), String_Utils::to_uppercase(input));
	EXPECT_EQ(String_Utils::to_lowercase_from_copy(input), String_Utils::to_lowercase(input));
}

// File Path Decomposition
// -----------------------
TEST(String_Utils_Test, get_file_extension_returns_the_last_extension_with_the_dot)
{
	EXPECT_EQ(String_Utils::get_file_extension("models/obj/cube.obj"), ".obj");
	EXPECT_EQ(String_Utils::get_file_extension("shaders/skybox.vert.glsl"), ".glsl");
	EXPECT_EQ(String_Utils::get_file_extension("textures/no_extension"), "");
}

TEST(String_Utils_Test, get_filename_with_extension_removes_the_directories)
{
	EXPECT_EQ(String_Utils::get_filename_with_extension("models/obj/cube.obj"), "cube.obj");
	EXPECT_EQ(String_Utils::get_filename_with_extension("cube.obj"), "cube.obj");
}

TEST(String_Utils_Test, get_filename_without_extension_removes_the_directories_and_the_last_extension)
{
	EXPECT_EQ(String_Utils::get_filename_without_extension("models/obj/cube.obj"), "cube");
	EXPECT_EQ(String_Utils::get_filename_without_extension("shaders/skybox.vert.glsl"), "skybox.vert");
	EXPECT_EQ(String_Utils::get_filename_without_extension_or_path("models/obj/cube.obj"), "cube");
}

TEST(String_Utils_Test, get_directory_path_removes_the_filename)
{
	EXPECT_EQ(String_Utils::get_directory_path("models/obj/cube.obj"), "models/obj");
	EXPECT_EQ(String_Utils::get_directory_path("cube.obj"), "");
}

// Display Names
// -------------
TEST(String_Utils_Test, to_clean_display_name_produces_title_case_words)
{
	EXPECT_EQ(String_Utils::to_clean_display_name("models/obj/backpack_model-v2.obj"), "Backpack Model V2");
	EXPECT_EQ(String_Utils::to_clean_display_name("SPOT_LIGHT"), "Spot Light");
	EXPECT_EQ(String_Utils::to_clean_display_name(""), "");
}

TEST(String_Utils_Test, generate_id_prefixed_name_returns_an_empty_string_for_a_null_node)
{
	const std::shared_ptr<Node> node{};
	EXPECT_EQ(String_Utils::generate_id_prefixed_name(node), "");
}
