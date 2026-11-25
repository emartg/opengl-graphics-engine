/*
 * NodeFactoryTest.cpp
 * This file tests the reading of the shape and light descriptors by the Node_Factory class (their fields, their
 * defaults, and the reasons for refusing them), the scale of a shape in the units of a scene, and the engine's own
 * shape and light assets. Building the nodes needs an OpenGL context, so it is checked in the App instead.
 */

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include <gtest/gtest.h>

#include "core/asset/Node_Factory.h"

namespace fs = std::filesystem;

namespace
{
	// Returns true if two vectors are equal within a small tolerance
	template<typename Vector>
	bool is_near(const Vector& a, const Vector& b)
	{
		return glm::all(glm::epsilonEqual(a, b, 1e-5f));
	}
}

TEST(NodeFactoryTest, ParsesEveryFieldOfAShapeDescriptor)
{
	std::string error;
	const auto  descriptor = Node_Factory::parse_shape_descriptor(
		R"({ "name": "Box", "shape": "cube", "size": [1, 2, 4], "color": [0.1, 0.2, 0.3, 0.5], "two_sided": true })",
		error);

	ASSERT_TRUE(descriptor) << error;
	EXPECT_EQ(descriptor->name, "Box");
	EXPECT_EQ(descriptor->primitive, Shape_Primitive::CUBE);
	ASSERT_TRUE(descriptor->size);
	EXPECT_TRUE(is_near(*descriptor->size, glm::vec3{ 1.0f, 2.0f, 4.0f }));
	EXPECT_TRUE(is_near(descriptor->color, glm::vec4{ 0.1f, 0.2f, 0.3f, 0.5f }));
	EXPECT_TRUE(descriptor->is_two_sided);
}

TEST(NodeFactoryTest, GivesAShapeItsDefaults)
{
	std::string error;
	const auto  plane = Node_Factory::parse_shape_descriptor(R"({ "name": "Floor", "shape": "plane", "color": [1, 0, 0] })", error);
	ASSERT_TRUE(plane) << error;
	EXPECT_EQ(plane->primitive, Shape_Primitive::PLANE);
	EXPECT_FALSE(plane->size);                                               // the built-in size
	EXPECT_TRUE(is_near(plane->color, glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f })); // RGB is opaque
	EXPECT_TRUE(plane->is_two_sided);                                        // a plane is seen from both sides

	const auto cube = Node_Factory::parse_shape_descriptor(R"({ "name": "Cube", "shape": "cube" })", error);
	ASSERT_TRUE(cube) << error;
	EXPECT_FALSE(cube->is_two_sided);
	EXPECT_TRUE(is_near(cube->color, glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f }));
}

TEST(NodeFactoryTest, ReadsTheWidthAndDepthOfAPlane)
{
	std::string error;
	const auto  plane = Node_Factory::parse_shape_descriptor(R"({ "name": "Floor", "shape": "plane", "size": [6, 4] })", error);
	ASSERT_TRUE(plane) << error;
	ASSERT_TRUE(plane->size);
	EXPECT_TRUE(is_near(*plane->size, glm::vec3{ 6.0f, 0.0f, 4.0f }));

	// three numbers are the size of a cube, not of a plane
	EXPECT_FALSE(Node_Factory::parse_shape_descriptor(R"({ "name": "Floor", "shape": "plane", "size": [6, 1, 4] })", error));
	EXPECT_NE(error.find("width and depth"), std::string::npos);
}

TEST(NodeFactoryTest, RefusesInvalidShapeDescriptors)
{
	const std::vector<std::pair<std::string, std::string>> cases{
		{ "not json", "not a valid JSON object" },
		{ R"([ "name" ])", "not a valid JSON object" },
		{ R"({ "shape": "cube" })", "\"name\"" },
		{ R"({ "name": "", "shape": "cube" })", "\"name\"" },
		{ R"({ "name": "A" })", "\"shape\"" },
		{ R"({ "name": "A", "shape": "sphere" })", "\"shape\"" },
		{ R"({ "name": "A", "shape": "cube", "size": [1, 1] })", "\"size\"" },
		{ R"({ "name": "A", "shape": "cube", "size": [1, 0, 1] })", "\"size\"" },
		{ R"({ "name": "A", "shape": "cube", "size": [1, -1, 1] })", "\"size\"" },
		{ R"({ "name": "A", "shape": "cube", "size": "big" })", "\"size\"" },
		{ R"({ "name": "A", "shape": "cube", "color": [1, 0] })", "\"color\"" },
		{ R"({ "name": "A", "shape": "cube", "color": [1, 0, 2] })", "\"color\"" },
		{ R"({ "name": "A", "shape": "cube", "color": [1, 0, -0.5] })", "\"color\"" },
		{ R"({ "name": "A", "shape": "cube", "two_sided": 1 })", "\"two_sided\"" },
	};
	for (const auto& [text, reason] : cases)
	{
		std::string error;
		EXPECT_FALSE(Node_Factory::parse_shape_descriptor(text, error)) << text;
		EXPECT_NE(error.find(reason), std::string::npos) << text << ": " << error;
	}
}

TEST(NodeFactoryTest, ParsesEveryFieldOfALightDescriptor)
{
	std::string error;
	const auto  descriptor = Node_Factory::parse_light_descriptor(
		R"({ "name": "Lamp", "type": "spot", "color": [2, 1, 0.5], "ambient": [0, 0, 0], "specular": [0.5, 0.5, 0.5],
		     "direction": [0, -2, 0], "range": 10, "inner_cutoff": 20, "outer_cutoff": 30, "gizmo_size": 0.25 })",
		error);

	ASSERT_TRUE(descriptor) << error;
	EXPECT_EQ(descriptor->name, "Lamp");
	EXPECT_EQ(descriptor->type, Light_Type::SPOTLIGHT);
	EXPECT_TRUE(is_near(descriptor->diffuse, glm::vec3{ 2.0f, 1.0f, 0.5f })); // brighter than 1 is allowed
	EXPECT_TRUE(is_near(descriptor->ambient, glm::vec3{ 0.0f }));
	EXPECT_TRUE(is_near(descriptor->specular, glm::vec3{ 0.5f }));
	ASSERT_TRUE(descriptor->direction);
	EXPECT_TRUE(is_near(*descriptor->direction, glm::vec3{ 0.0f, -1.0f, 0.0f })); // normalized
	ASSERT_TRUE(descriptor->range);
	EXPECT_FLOAT_EQ(*descriptor->range, 10.0f);
	EXPECT_FLOAT_EQ(descriptor->inner_cutoff, 20.0f);
	EXPECT_FLOAT_EQ(descriptor->outer_cutoff, 30.0f);
	ASSERT_TRUE(descriptor->gizmo_size);
	EXPECT_FLOAT_EQ(*descriptor->gizmo_size, 0.25f);
}

TEST(NodeFactoryTest, GivesALightTheDefaultsOfTheEngine)
{
	std::string error;
	const auto  descriptor = Node_Factory::parse_light_descriptor(R"({ "name": "Bulb", "type": "point" })", error);

	ASSERT_TRUE(descriptor) << error;
	EXPECT_EQ(descriptor->type, Light_Type::POINT_LIGHT);
	EXPECT_TRUE(is_near(descriptor->ambient, glm::vec3{ 0.1f }));
	EXPECT_TRUE(is_near(descriptor->diffuse, glm::vec3{ 0.8f }));
	EXPECT_TRUE(is_near(descriptor->specular, glm::vec3{ 1.0f }));
	EXPECT_FALSE(descriptor->direction);  // the default of the kind of light
	EXPECT_FALSE(descriptor->range);      // the engine's attenuation
	EXPECT_FALSE(descriptor->gizmo_size); // the default of the kind of light

	const auto sun = Node_Factory::parse_light_descriptor(R"({ "name": "Sun", "type": "directional" })", error);
	ASSERT_TRUE(sun) << error;
	EXPECT_EQ(sun->type, Light_Type::DIRECTIONAL_LIGHT);
}

TEST(NodeFactoryTest, RefusesFieldsThatTheTypeOfLightDoesNotHave)
{
	const std::vector<std::string> cases{
		R"({ "name": "Sun", "type": "directional", "range": 10 })",
		R"({ "name": "Sun", "type": "directional", "inner_cutoff": 10 })",
		R"({ "name": "Bulb", "type": "point", "direction": [0, -1, 0] })",
		R"({ "name": "Bulb", "type": "point", "outer_cutoff": 10 })",
	};
	for (const auto& text : cases)
	{
		std::string error;
		EXPECT_FALSE(Node_Factory::parse_light_descriptor(text, error)) << text;
		EXPECT_NE(error.find("is only for"), std::string::npos) << text << ": " << error;
	}
}

TEST(NodeFactoryTest, RefusesInvalidLightDescriptors)
{
	const std::vector<std::pair<std::string, std::string>> cases{
		{ "{", "not a valid JSON object" },
		{ R"({ "type": "point" })", "\"name\"" },
		{ R"({ "name": "A" })", "\"type\"" },
		{ R"({ "name": "A", "type": "area" })", "\"type\"" },
		{ R"({ "name": "A", "type": "point", "color": [1, 1] })", "\"color\"" },
		{ R"({ "name": "A", "type": "point", "ambient": [1, -1, 1] })", "\"ambient\"" },
		{ R"({ "name": "A", "type": "point", "specular": "white" })", "\"specular\"" },
		{ R"({ "name": "A", "type": "spot", "direction": [0, 0, 0] })", "\"direction\"" },
		{ R"({ "name": "A", "type": "point", "range": 0 })", "\"range\"" },
		{ R"({ "name": "A", "type": "point", "range": -5 })", "\"range\"" },
		{ R"({ "name": "A", "type": "point", "gizmo_size": "small" })", "\"gizmo_size\"" },
		{ R"({ "name": "A", "type": "spot", "outer_cutoff": 90 })", "\"outer_cutoff\"" },
		{ R"({ "name": "A", "type": "spot", "inner_cutoff": 0 })", "\"inner_cutoff\"" },
		{ R"({ "name": "A", "type": "spot", "inner_cutoff": 20 })", "must not be larger" }, // the default outer angle is 15
		{ R"({ "name": "A", "type": "spot", "inner_cutoff": 30, "outer_cutoff": 20 })", "must not be larger" },
	};
	for (const auto& [text, reason] : cases)
	{
		std::string error;
		EXPECT_FALSE(Node_Factory::parse_light_descriptor(text, error)) << text;
		EXPECT_NE(error.find(reason), std::string::npos) << text << ": " << error;
	}
}

TEST(NodeFactoryTest, ScalesAShapeToItsSizeInTheUnitsOfTheScene)
{
	Shape_Descriptor cube;
	cube.primitive = Shape_Primitive::CUBE;
	EXPECT_TRUE(is_near(Node_Factory::get_shape_scale(cube, 1.0f), glm::vec3{ 1.0f }));   // the built-in cube, 2 units across
	EXPECT_TRUE(is_near(Node_Factory::get_shape_scale(cube, 100.0f), glm::vec3{ 1.0f })); // without a size, as it is

	cube.size = glm::vec3{ 1.0f, 2.0f, 4.0f }; // in metres
	EXPECT_TRUE(is_near(Node_Factory::get_shape_scale(cube, 1.0f), glm::vec3{ 0.5f, 1.0f, 2.0f }));
	EXPECT_TRUE(is_near(Node_Factory::get_shape_scale(cube, 100.0f), glm::vec3{ 50.0f, 100.0f, 200.0f })); // in centimetres

	Shape_Descriptor plane;
	plane.primitive = Shape_Primitive::PLANE;
	plane.size      = glm::vec3{ 6.0f, 0.0f, 4.0f };
	EXPECT_TRUE(is_near(Node_Factory::get_shape_scale(plane, 1.0f), glm::vec3{ 3.0f, 1.0f, 2.0f })); // no height to scale
}

TEST(NodeFactoryTest, ReadsTheShapeAndLightAssetsOfTheEngine)
{
	const fs::path assets_dir{ ENGINE_UNIT_TESTS_ASSETS_DIR };
	std::string    error;

	const auto cube = Node_Factory::load_shape_descriptor(assets_dir / "shapes" / "cube.shape.json", error);
	ASSERT_TRUE(cube) << error;
	EXPECT_EQ(cube->name, "Cube Shape");
	EXPECT_EQ(cube->primitive, Shape_Primitive::CUBE);
	const auto plane = Node_Factory::load_shape_descriptor(assets_dir / "shapes" / "plane.shape.json", error);
	ASSERT_TRUE(plane) << error;
	EXPECT_EQ(plane->name, "Plane Shape");
	EXPECT_EQ(plane->primitive, Shape_Primitive::PLANE);

	const std::vector<std::pair<std::string, Light_Type>> lights{ { "directional_light", Light_Type::DIRECTIONAL_LIGHT },
																  { "point_light", Light_Type::POINT_LIGHT },
																  { "spotlight", Light_Type::SPOTLIGHT } };
	for (const auto& [file_name, type] : lights)
	{
		const auto light = Node_Factory::load_light_descriptor(assets_dir / "lights" / (file_name + ".light.json"), error);
		ASSERT_TRUE(light) << error;
		EXPECT_EQ(light->type, type) << file_name;
	}

	// every descriptor of these folders is valid, not only the ones above
	for (const auto& folder : { "shapes", "lights" })
		for (const auto& entry : fs::directory_iterator(assets_dir / folder))
		{
			const bool is_shape = std::string{ folder } == "shapes";
			const bool is_valid = is_shape ? Node_Factory::load_shape_descriptor(entry.path(), error).has_value()
										   : Node_Factory::load_light_descriptor(entry.path(), error).has_value();
			EXPECT_TRUE(is_valid) << error;
		}
}

TEST(NodeFactoryTest, NamesTheFileThatCannotBeRead)
{
	std::string error;
	EXPECT_FALSE(Node_Factory::load_light_descriptor(fs::temp_directory_path() / "node_factory_test_missing.light.json", error));
	EXPECT_NE(error.find("node_factory_test_missing.light.json"), std::string::npos) << error;
}
