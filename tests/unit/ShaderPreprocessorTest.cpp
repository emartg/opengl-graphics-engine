/*
 * ShaderPreprocessorTest.cpp
 * This file contains the unit tests of the Shader_Preprocessor class: the resolution of the #include
 * directives of shader source files (relative paths, nested and repeated includes, and errors) and
 * the #line directives that keep the line numbers of the original files.
 */

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "core/shader/Shader_Preprocessor.h"

namespace fs = std::filesystem;

// Test fixture that creates a temporary directory for each test, where the shader files are written,
// and removes it afterwards
class ShaderPreprocessorTest : public ::testing::Test
{
protected:
	fs::path root_dir;

	void SetUp() override
	{
		// use the test name, so that tests running in parallel do not share their directories
		const std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

		root_dir = fs::temp_directory_path() / ("engine_unit_tests_" + test_name);
		fs::remove_all(root_dir);
		fs::create_directories(root_dir);
	}

	void TearDown() override
	{
		std::error_code error; // ignore errors, so that a cleanup failure does not hide the test result
		fs::remove_all(root_dir, error);
	}

	// Writes a file with the given contents (relative to the temporary directory), and returns its path
	fs::path write_file(const fs::path& relative_path, const std::string& contents) const
	{
		const fs::path path = root_dir / relative_path;
		fs::create_directories(path.parent_path());
		std::ofstream{ path, std::ios::binary } << contents;
		return path;
	}
};

TEST_F(ShaderPreprocessorTest, ReturnsTheFileUnchangedWithoutIncludes)
{
	const fs::path path = write_file("shader.glsl", "#version 450 core\nvoid main() {}\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	ASSERT_TRUE(source.is_valid()) << source.error;
	EXPECT_EQ(source.code, "#version 450 core\nvoid main() {}\n");
	ASSERT_EQ(source.files.size(), 1u);
}

TEST_F(ShaderPreprocessorTest, ReplacesAnIncludeWithTheIncludedFileBetweenLineDirectives)
{
	write_file("common.glsl", "float f;\n");
	const fs::path path = write_file("shader.glsl", "#version 450 core\n#include \"common.glsl\"\nvoid main() {}\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	ASSERT_TRUE(source.is_valid()) << source.error;
	// the included file is source string 1, and the lines of the shader continue at line 3 of source string 0
	EXPECT_EQ(source.code, "#version 450 core\n#line 1 1\nfloat f;\n#line 3 0\nvoid main() {}\n");
	ASSERT_EQ(source.files.size(), 2u);
	EXPECT_EQ(source.files[1].filename(), "common.glsl");
}

TEST_F(ShaderPreprocessorTest, ResolvesNestedIncludesRelativeToTheIncludingFile)
{
	write_file("include/b.glsl", "float b;\n");
	write_file("include/a.glsl", "#include \"b.glsl\"\nfloat a;\n"); // relative to include/, not to the shader
	const fs::path path = write_file("shader.glsl", "#version 450 core\n#include \"include/a.glsl\"\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	ASSERT_TRUE(source.is_valid()) << source.error;
	EXPECT_EQ(source.code, "#version 450 core\n#line 1 1\n#line 1 2\nfloat b;\n#line 2 1\nfloat a;\n#line 3 0\n");
	ASSERT_EQ(source.files.size(), 3u);
	EXPECT_EQ(source.files[2].filename(), "b.glsl");
}

TEST_F(ShaderPreprocessorTest, IncludesEachFileOnlyOnce)
{
	write_file("common.glsl", "float f;\n");
	write_file("other.glsl", "#include \"common.glsl\"\nfloat g;\n");
	const fs::path path = write_file("shader.glsl", "#include \"common.glsl\"\n#include \"./common.glsl\"\n#include \"other.glsl\"\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	ASSERT_TRUE(source.is_valid()) << source.error;
	// the repeated directives become empty lines, so that the following lines keep their line numbers
	EXPECT_EQ(source.code, "#line 1 1\nfloat f;\n#line 2 0\n\n#line 1 2\n\nfloat g;\n#line 4 0\n");
	EXPECT_EQ(source.files.size(), 3u);
}

TEST_F(ShaderPreprocessorTest, StopsCircularIncludes)
{
	write_file("a.glsl", "#include \"b.glsl\"\nfloat a;\n");
	write_file("b.glsl", "#include \"a.glsl\"\nfloat b;\n");
	const fs::path path = write_file("shader.glsl", "#include \"a.glsl\"\n");

	const Shader_Source source = Shader_Preprocessor::load(path);
	// the include of a.glsl in b.glsl becomes an empty line, so that "float b;" is still line 2 of b.glsl
	ASSERT_TRUE(source.is_valid()) << source.error;
	EXPECT_EQ(source.code, "#line 1 1\n#line 1 2\n\nfloat b;\n#line 2 1\nfloat a;\n#line 2 0\n");
}

TEST_F(ShaderPreprocessorTest, AcceptsIndentationAndWindowsLineEndings)
{
	write_file("common.glsl", "float f;\r\n");
	const fs::path path = write_file("shader.glsl", "\t #include \"common.glsl\"\r\nvoid main() {}\r\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	ASSERT_TRUE(source.is_valid()) << source.error;
	EXPECT_EQ(source.files.size(), 2u);
	EXPECT_NE(source.code.find("float f;"), std::string::npos);
	EXPECT_EQ(source.code.find("#include"), std::string::npos);
}

TEST_F(ShaderPreprocessorTest, ReportsAMissingFile)
{
	const Shader_Source source = Shader_Preprocessor::load(root_dir / "missing.glsl");

	EXPECT_FALSE(source.is_valid());
	EXPECT_NE(source.error.find("missing.glsl"), std::string::npos);
	EXPECT_TRUE(source.code.empty());
}

TEST_F(ShaderPreprocessorTest, ReportsAMissingIncludedFileAndWhereItIsIncluded)
{
	const fs::path path = write_file("shader.glsl", "#version 450 core\n#include \"missing.glsl\"\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	EXPECT_FALSE(source.is_valid());
	EXPECT_NE(source.error.find("missing.glsl"), std::string::npos);
	EXPECT_NE(source.error.find("line 2"), std::string::npos);
	EXPECT_TRUE(source.code.empty());
}

TEST_F(ShaderPreprocessorTest, ReportsAMalformedInclude)
{
	const fs::path path = write_file("shader.glsl", "#include <common.glsl>\n");

	const Shader_Source source = Shader_Preprocessor::load(path);

	EXPECT_FALSE(source.is_valid());
	EXPECT_NE(source.error.find("Malformed"), std::string::npos);
}
