/*
 * OutlineParamsTest.cpp
 * This file contains the unit test of the Outline_Params struct of the Selection_Manager: the outline of the selected node
 * is enabled by default, with the default color and thickness.
 */

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include "core/managers/Selection_Manager.h"

TEST(OutlineParamsTest, TheOutlineIsEnabledByDefaultWithTheDefaultColorAndThickness)
{
	const Outline_Params params;

	EXPECT_TRUE(params.enabled);
	EXPECT_EQ(params.thickness, 2u);
	EXPECT_EQ(params.color, glm::vec3(0.0f, 0.95f, 1.0f));
}

TEST(OutlineParamsTest, ACopyCanTurnTheOutlineOffWithoutChangingTheOtherValues)
{
	Outline_Params params;
	params.enabled = false;

	EXPECT_FALSE(params.enabled);
	EXPECT_EQ(params.thickness, Outline_Params{}.thickness);
	EXPECT_EQ(params.color, Outline_Params{}.color);
}
