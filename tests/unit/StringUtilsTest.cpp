/*
 * StringUtilsTest.cpp
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
TEST(StringUtilsTest, ToUppercaseConvertsOnlyLetters)
{
	EXPECT_EQ(String_Utils::to_uppercase("Point_Light 2"), "POINT_LIGHT 2");
	EXPECT_EQ(String_Utils::to_uppercase(""), "");
}

TEST(StringUtilsTest, ToLowercaseConvertsOnlyLetters)
{
	EXPECT_EQ(String_Utils::to_lowercase("Point_Light 2"), "point_light 2");
	EXPECT_EQ(String_Utils::to_lowercase(""), "");
}

TEST(StringUtilsTest, ConversionsFromCopyMatchConversionsFromReference)
{
	const std::string input{ "Skybox.Vert.GLSL" };
	EXPECT_EQ(String_Utils::to_uppercase_from_copy(input), String_Utils::to_uppercase(input));
	EXPECT_EQ(String_Utils::to_lowercase_from_copy(input), String_Utils::to_lowercase(input));
}

// File Path Decomposition
// -----------------------
TEST(StringUtilsTest, GetFileExtensionReturnsTheLastExtensionWithTheDot)
{
	EXPECT_EQ(String_Utils::get_file_extension("models/obj/cube.obj"), ".obj");
	EXPECT_EQ(String_Utils::get_file_extension("shaders/skybox.vert.glsl"), ".glsl");
	EXPECT_EQ(String_Utils::get_file_extension("textures/no_extension"), "");
}

TEST(StringUtilsTest, GetFilenameWithExtensionRemovesTheDirectories)
{
	EXPECT_EQ(String_Utils::get_filename_with_extension("models/obj/cube.obj"), "cube.obj");
	EXPECT_EQ(String_Utils::get_filename_with_extension("cube.obj"), "cube.obj");
}

TEST(StringUtilsTest, GetFilenameWithoutExtensionRemovesTheDirectoriesAndTheLastExtension)
{
	EXPECT_EQ(String_Utils::get_filename_without_extension("models/obj/cube.obj"), "cube");
	EXPECT_EQ(String_Utils::get_filename_without_extension("shaders/skybox.vert.glsl"), "skybox.vert");
	EXPECT_EQ(String_Utils::get_filename_without_extension_or_path("models/obj/cube.obj"), "cube");
}

TEST(StringUtilsTest, GetDirectoryPathRemovesTheFilename)
{
	EXPECT_EQ(String_Utils::get_directory_path("models/obj/cube.obj"), "models/obj");
	EXPECT_EQ(String_Utils::get_directory_path("cube.obj"), "");
}

// Display Names
// -------------
TEST(StringUtilsTest, ToCleanDisplayNameProducesTitleCaseWords)
{
	EXPECT_EQ(String_Utils::to_clean_display_name("models/obj/backpack_model-v2.obj"), "Backpack Model V2");
	EXPECT_EQ(String_Utils::to_clean_display_name("SPOT_LIGHT"), "Spot Light");
	EXPECT_EQ(String_Utils::to_clean_display_name(""), "");
}

TEST(StringUtilsTest, GenerateIdPrefixedNameReturnsAnEmptyStringForANullNode)
{
	const std::shared_ptr<Node> node{};
	EXPECT_EQ(String_Utils::generate_id_prefixed_name(node), "");
}

TEST(StringUtilsTest, IsGlslIdentifierAcceptsLettersDigitsAndUnderscoresNotStartingWithADigit)
{
	for (const char* identifier : { "albedo_map", "_private", "map2", "A", "shininess" })
		EXPECT_TRUE(String_Utils::is_glsl_identifier(identifier)) << identifier;
	for (const char* text : { "", "1map", "base-color", "albedo map", "map.x", "map$" })
		EXPECT_FALSE(String_Utils::is_glsl_identifier(text)) << text;
}

TEST(StringUtilsTest, IsGlslIdentifierRejectsTheReservedPrefixAndDoubleUnderscores)
{
	EXPECT_FALSE(String_Utils::is_glsl_identifier("gl_map"));
	EXPECT_FALSE(String_Utils::is_glsl_identifier("albedo__map"));
	EXPECT_TRUE(String_Utils::is_glsl_identifier("glossiness")); // only the "gl_" prefix is reserved
}
