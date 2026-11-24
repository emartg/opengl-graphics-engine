/*
 * LightTest.cpp
 * This file contains the unit tests of the lights that need no OpenGL context: a light follows its parent (its position and
 * direction are relative to the parent, like the transform of a node, and the renderer lights the scene with them in world
 * space), it can be moved into a hierarchy but cannot be a parent, and a new parent keeps it where it is in the world
 * (Node::rebase_transform, also tested for an ordinary node); the attenuation of a range, and the scale of the gizmos. The gizmos
 * themselves need an OpenGL context, so the lights of these tests have none, and the gizmos are covered by the smoke tests and by
 * driving the App.
 */

#include <array>
#include <limits>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <gtest/gtest.h>

#include "core/Node.h"
#include "core/light/Directional_Light.h"
#include "core/light/Point_Light.h"
#include "core/light/Spotlight.h"

namespace
{
	// A node without geometry, to be the parent of the lights of the tests (drawing it does nothing)
	class Test_Node : public Node
	{
	public:
		explicit Test_Node(const std::string& name) : Node(name, Node_Type::SHAPE_MODEL) {}

		void draw() const override {}
		void draw(const Shader& shader) const override {}
	};

	// Creates a parent at the given position, turned by the given angle (in degrees) about the Y axis, with a uniform scale
	std::shared_ptr<Node> make_parent(const glm::vec3& position, float angle_y, float scale)
	{
		auto parent = std::make_shared<Test_Node>("parent");
		parent->set_position(position);
		parent->set_rotation(glm::angleAxis(glm::radians(angle_y), glm::vec3{ 0.0f, 1.0f, 0.0f }));
		parent->set_scale(glm::vec3{ scale });
		return parent;
	}

	// Expects two vectors to be equal within a small tolerance
	void expect_near(const glm::vec3& actual, const glm::vec3& expected)
	{
		constexpr float TOLERANCE{ 1e-4f };
		EXPECT_NEAR(actual.x, expected.x, TOLERANCE);
		EXPECT_NEAR(actual.y, expected.y, TOLERANCE);
		EXPECT_NEAR(actual.z, expected.z, TOLERANCE);
	}

	constexpr glm::vec3 GREY{ 0.5f };
} // namespace

TEST(LightTest, ALightCanBeMovedIntoAHierarchyButCannotBeAParent)
{
	Point_Light light{ "light" };

	EXPECT_TRUE(light.get_is_movable());
	EXPECT_FALSE(light.get_can_be_parent());
}

TEST(LightTest, ALightWithoutAParentShinesFromItsOwnPositionAndDirection)
{
	Point_Light point_light{ "point", GREY, GREY, GREY, glm::vec3{ 1.0f, 2.0f, 3.0f } };
	Spotlight   spotlight{ "spot", GREY, GREY, GREY, glm::vec3{ 4.0f, 5.0f, 6.0f }, glm::vec3{ 0.0f, 0.0f, -2.0f } };

	expect_near(point_light.get_position_in_world(), glm::vec3{ 1.0f, 2.0f, 3.0f });
	expect_near(spotlight.get_position_in_world(), glm::vec3{ 4.0f, 5.0f, 6.0f });
	expect_near(spotlight.get_direction_in_world(), glm::vec3{ 0.0f, 0.0f, -1.0f }); // normalized
}

TEST(LightTest, APointLightFollowsThePositionTheRotationAndTheScaleOfItsParent)
{
	const auto parent = make_parent(glm::vec3{ 10.0f, 0.0f, 0.0f }, 90.0f, 2.0f);
	const auto light  = std::make_shared<Point_Light>("light", GREY, GREY, GREY, glm::vec3{ 1.0f, 0.0f, 0.0f });
	parent->add_child(light);

	// the position stays relative to the parent, and in the world it is scaled by 2, turned to -Z, and moved by 10 along X
	expect_near(light->get_position(), glm::vec3{ 1.0f, 0.0f, 0.0f });
	expect_near(light->get_position_in_world(), glm::vec3{ 10.0f, 0.0f, -2.0f });
}

TEST(LightTest, ASpotlightAndADirectionalLightTurnWithTheirParentWithAUnitDirection)
{
	const auto parent      = make_parent(glm::vec3{ 0.0f, 3.0f, 0.0f }, 90.0f, 5.0f);
	const auto spotlight   = std::make_shared<Spotlight>("spot", GREY, GREY, GREY, glm::vec3{ 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f });
	const auto directional = std::make_shared<Directional_Light>("sun", GREY, GREY, GREY, glm::vec3{ 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f });
	parent->add_child(spotlight);
	parent->add_child(directional);

	expect_near(spotlight->get_position_in_world(), glm::vec3{ 0.0f, 3.0f, 0.0f });
	// the scale of the parent does not lengthen the direction
	expect_near(spotlight->get_direction_in_world(), glm::vec3{ 0.0f, 0.0f, -1.0f });
	expect_near(directional->get_direction_in_world(), glm::vec3{ 0.0f, 0.0f, -1.0f });
}

TEST(LightTest, ANewParentKeepsAPointLightWhereItIsInTheWorld)
{
	const auto light  = std::make_shared<Point_Light>("light", GREY, GREY, GREY, glm::vec3{ 3.0f, 2.0f, 1.0f });
	const auto parent = make_parent(glm::vec3{ -4.0f, 1.0f, 2.0f }, 30.0f, 0.5f);

	// as the scene graph of an editor does: the light is given its parent, then its transform is rebased
	parent->add_child(light);
	light->rebase_transform(glm::mat4{ 1.0f }, parent->get_world_model_matrix());

	expect_near(light->get_position_in_world(), glm::vec3{ 3.0f, 2.0f, 1.0f });
	// the position is now relative to the parent, and the transform of the light itself is unchanged
	expect_near(light->get_position(), glm::vec3{ glm::inverse(parent->get_world_model_matrix()) * glm::vec4{ 3.0f, 2.0f, 1.0f, 1.0f } });
	expect_near(light->Node::get_position(), glm::vec3{ 0.0f });
}

TEST(LightTest, ANewParentKeepsASpotlightWhereItIsAndPointingTheSameWay)
{
	const auto old_parent = make_parent(glm::vec3{ 1.0f, 0.0f, 0.0f }, 45.0f, 2.0f);
	const auto new_parent = make_parent(glm::vec3{ 0.0f, 0.0f, 5.0f }, -120.0f, 1.0f);
	const auto spotlight  = std::make_shared<Spotlight>(
		"spot",
		GREY,
		GREY,
		GREY,
		glm::vec3{ 0.0f, 2.0f, 1.0f },
		glm::normalize(glm::vec3{ 1.0f, -1.0f, 0.0f }));
	old_parent->add_child(spotlight);
	const glm::vec3 position{ spotlight->get_position_in_world() };
	const glm::vec3 direction{ spotlight->get_direction_in_world() };

	// moved from one parent to another
	old_parent->remove_child(spotlight);
	new_parent->add_child(spotlight);
	spotlight->rebase_transform(old_parent->get_world_model_matrix(), new_parent->get_world_model_matrix());

	expect_near(spotlight->get_position_in_world(), position);
	expect_near(spotlight->get_direction_in_world(), direction);
}

TEST(LightTest, ANewParentKeepsAnOrdinaryNodeWhereItIsInTheWorld)
{
	const auto parent = make_parent(glm::vec3{ 2.0f, -1.0f, 4.0f }, 60.0f, 3.0f);
	const auto node   = std::make_shared<Test_Node>("node");
	node->set_position(glm::vec3{ 1.0f, 1.0f, 1.0f });
	node->set_rotation(glm::angleAxis(glm::radians(20.0f), glm::vec3{ 1.0f, 0.0f, 0.0f }));
	const glm::mat4 world{ node->get_world_model_matrix() };

	parent->add_child(node);
	node->rebase_transform(glm::mat4{ 1.0f }, parent->get_world_model_matrix());

	const glm::mat4 rebased{ node->get_world_model_matrix() };
	for (int column = 0; column < 4; ++column) expect_near(glm::vec3{ rebased[column] }, glm::vec3{ world[column] });
}

TEST(LightTest, TheAttenuationOfARangeFollowsTheUsualTableOfAttenuationTerms)
{
	// rows of the usual table (range, linear, quadratic); the table rounds its terms, so they are compared within a tolerance
	const float table[][3]{ { 7.0f, 0.7f, 1.8f },
							{ 13.0f, 0.35f, 0.44f },
							{ 50.0f, 0.09f, 0.032f },
							{ 100.0f, 0.045f, 0.0075f },
							{ 3250.0f, 0.0014f, 0.000007f } };
	for (const auto& row : table)
	{
		const glm::vec2 attenuation{ Light::get_attenuation_for_range(row[0]) };
		EXPECT_NEAR(attenuation.x, row[1], 0.1f * row[1]) << "range " << row[0];
		EXPECT_NEAR(attenuation.y, row[2], 0.2f * row[2]) << "range " << row[0];
	}
}

TEST(LightTest, TheRangeOfAPointLightAndASpotlightSetsTheirAttenuation)
{
	Point_Light point_light{ "point" };
	Spotlight   spotlight{ "spot" };

	// the default terms are those of a range of about 50, for a scene in metres
	EXPECT_NEAR(point_light.get_range(), 50.0f, 1e-3f);
	EXPECT_NEAR(spotlight.get_range(), 50.0f, 1e-3f);

	// the same light in a scene in centimetres reaches 5 metres with a range of 500
	EXPECT_TRUE(point_light.set_range(500.0f));
	EXPECT_TRUE(spotlight.set_range(500.0f));
	for (const auto& [constant, linear, quadratic, range] :
		 { std::array<float, 4>{ point_light.get_constant(),
								 point_light.get_linear(),
								 point_light.get_quadratic(),
								 point_light.get_range() },
		   std::array<float, 4>{ spotlight.get_constant(), spotlight.get_linear(), spotlight.get_quadratic(), spotlight.get_range() } })
	{
		EXPECT_FLOAT_EQ(constant, 1.0f);
		EXPECT_FLOAT_EQ(linear, 4.5f / 500.0f);
		EXPECT_FLOAT_EQ(quadratic, 75.0f / (500.0f * 500.0f));
		EXPECT_NEAR(range, 500.0f, 1e-2f);
	}
}

TEST(LightTest, AnInvalidRangeKeepsTheAttenuation)
{
	Point_Light point_light{ "point" };
	Spotlight   spotlight{ "spot" };

	for (const float range : { 0.0f, -10.0f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN() })
	{
		EXPECT_FALSE(point_light.set_range(range));
		EXPECT_FALSE(spotlight.set_range(range));
	}
	EXPECT_FLOAT_EQ(point_light.get_linear(), 0.09f);
	EXPECT_FLOAT_EQ(point_light.get_quadratic(), 0.032f);
	EXPECT_FLOAT_EQ(spotlight.get_linear(), 0.09f);
	EXPECT_FLOAT_EQ(spotlight.get_quadratic(), 0.032f);
}

TEST(LightTest, TheScaleOfTheGizmoCanBeSetAndInvalidScalesAreRefused)
{
	Point_Light       point_light{ "point" };
	Spotlight         spotlight{ "spot" };
	Directional_Light directional_light{ "directional" };

	// the defaults, for a scene in metres
	EXPECT_EQ(point_light.get_gizmo_scale(), glm::vec3{ 0.3f });
	EXPECT_EQ(spotlight.get_gizmo_scale(), glm::vec3{ 0.3f });
	EXPECT_EQ(directional_light.get_gizmo_scale(), glm::vec3{ 0.4f });

	// a gizmo of 5 units, for example 5 centimetres in a scene in centimetres (create_gizmo uses it)
	EXPECT_TRUE(point_light.set_gizmo_scale(glm::vec3{ 5.0f }));
	EXPECT_EQ(point_light.get_gizmo_scale(), glm::vec3{ 5.0f });

	EXPECT_FALSE(point_light.set_gizmo_scale(glm::vec3{ 1.0f, 0.0f, 1.0f }));
	EXPECT_FALSE(point_light.set_gizmo_scale(glm::vec3{ -1.0f }));
	EXPECT_FALSE(point_light.set_gizmo_scale(glm::vec3{ std::numeric_limits<float>::quiet_NaN() }));
	EXPECT_EQ(point_light.get_gizmo_scale(), glm::vec3{ 5.0f });
}
