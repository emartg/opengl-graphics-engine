/*
 * MaterialTest.cpp
 * This file contains the unit tests of the Material class that do not require an OpenGL context: the material
 * parameters of a shader, found from the active uniforms of its program, and the parameters and textures of a material.
 */

#include <map>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "core/material/Material.h"
#include "core/shader/Shader.h" // OpenGL types (e.g., GL_FLOAT)

// Shader Parameters
// -----------------
TEST(MaterialTest, FindsTheParametersOfAShaderWithTheirZeroValues)
{
	const std::map<std::string, unsigned int> uniforms{
		{ "u_material.shininess", GL_FLOAT }, { "u_material.enabled", GL_BOOL },     { "u_material.offset", GL_FLOAT_VEC2 },
		{ "u_material.tint", GL_FLOAT_VEC3 }, { "u_material.color", GL_FLOAT_VEC4 },
	};

	const Material_Parameters parameters = Material::get_shader_parameters(uniforms, {});

	ASSERT_EQ(parameters.size(), 5u);
	EXPECT_EQ(parameters.at("shininess"), Material_Value{ 0.0f });
	EXPECT_EQ(parameters.at("enabled"), Material_Value{ false });
	EXPECT_EQ(parameters.at("offset"), Material_Value{ glm::vec2{ 0.0f } });
	EXPECT_EQ(parameters.at("tint"), Material_Value{ glm::vec3{ 0.0f } });
	EXPECT_EQ(parameters.at("color"), Material_Value{ glm::vec4{ 0.0f } });
}

TEST(MaterialTest, IgnoresTheUniformsThatAreNotMaterialParameters)
{
	const std::map<std::string, unsigned int> uniforms{
		{ "u_model", GL_FLOAT_MAT4 },               // not a member of u_material
		{ "u_object_albedo", GL_FLOAT_VEC4 },       // not a member of u_material
		{ "u_material.albedo_map", GL_SAMPLER_2D }, // the sampler of a texture slot
		{ "u_material.has_albedo_map", GL_BOOL },   // the flag of a texture slot
		{ "u_material.count", GL_INT },             // a type that a parameter cannot have
		{ "u_material.weights[0]", GL_FLOAT },      // an element of an array member
		{ "u_material.layer.strength", GL_FLOAT },  // a member of a nested struct
		{ "u_material.shininess", GL_FLOAT },       // the only parameter
	};

	const Material_Parameters parameters = Material::get_shader_parameters(uniforms, { "albedo_map" });

	ASSERT_EQ(parameters.size(), 1u);
	EXPECT_TRUE(parameters.contains("shininess"));
}

TEST(MaterialTest, AShaderThatIsNotCompiledHasNoParameters)
{
	const Shader shader{ "Test Shader", "test.vert.glsl", "test.frag.glsl" };

	EXPECT_TRUE(Material::get_shader_parameters(shader).empty());
}

// Parameters and Textures
// -----------------------
TEST(MaterialTest, SetsAndRemovesParametersAndTextures)
{
	Material material{ "Test", nullptr, { { "shininess", 32.0f } } };

	material.set_parameter("shininess", 64.0f);
	material.set_parameter("enabled", true);
	EXPECT_EQ(material.get_parameters().at("shininess"), Material_Value{ 64.0f });
	material.remove_parameter("shininess");
	material.remove_parameter("missing"); // removing a parameter that the material does not have does nothing
	EXPECT_EQ(material.get_parameters().size(), 1u);
	EXPECT_TRUE(material.get_parameters().contains("enabled"));

	material.set_texture("albedo_map", nullptr);
	EXPECT_TRUE(material.get_textures().contains("albedo_map"));
	material.remove_texture("albedo_map");
	EXPECT_TRUE(material.get_textures().empty());
}
