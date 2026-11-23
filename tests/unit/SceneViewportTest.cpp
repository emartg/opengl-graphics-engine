/*
 * SceneViewportTest.cpp
 * This file contains the unit tests of the Scene_Viewport struct, the rectangle of the framebuffer where the scene is drawn: how
 * it is kept inside the framebuffer (also when it is empty, partly or wholly outside, or when the framebuffer is empty), which
 * points it contains, and its bottom edge in the coordinates of OpenGL.
 */

#include <gtest/gtest.h>

#include "core/renderer/Scene_Viewport.h"

TEST(SceneViewportTest, ARectangleInsideTheFramebufferStaysAsItIs)
{
	const Scene_Viewport viewport{ 400, 23, 800, 977 };

	EXPECT_EQ(viewport.clipped_to(1600, 1000), viewport);
}

TEST(SceneViewportTest, TheWholeFramebufferStaysAsItIs)
{
	const Scene_Viewport viewport{ 0, 0, 1600, 1000 };

	EXPECT_EQ(viewport.clipped_to(1600, 1000), viewport);
}

TEST(SceneViewportTest, ThePartsOutsideTheFramebufferAreCutOff)
{
	// beyond the right and bottom edges
	EXPECT_EQ((Scene_Viewport{ 1500, 900, 300, 300 }.clipped_to(1600, 1000)), (Scene_Viewport{ 1500, 900, 100, 100 }));
	// before the left and top edges
	EXPECT_EQ((Scene_Viewport{ -50, -20, 150, 120 }.clipped_to(1600, 1000)), (Scene_Viewport{ 0, 0, 100, 100 }));
}

TEST(SceneViewportTest, AnEmptyRectangleBecomesOnePixel)
{
	EXPECT_EQ((Scene_Viewport{ 400, 23, 0, 0 }.clipped_to(1600, 1000)), (Scene_Viewport{ 400, 23, 1, 1 }));
}

TEST(SceneViewportTest, ARectangleOutsideTheFramebufferBecomesOnePixelAtItsNearestEdge)
{
	EXPECT_EQ((Scene_Viewport{ 2000, 1200, 100, 100 }.clipped_to(1600, 1000)), (Scene_Viewport{ 1599, 999, 1, 1 }));
	EXPECT_EQ((Scene_Viewport{ -300, -300, 100, 100 }.clipped_to(1600, 1000)), (Scene_Viewport{ 0, 0, 1, 1 }));
}

TEST(SceneViewportTest, AnEmptyFramebufferGivesAnEmptyRectangle)
{
	EXPECT_EQ((Scene_Viewport{ 400, 23, 800, 977 }.clipped_to(0, 0)), Scene_Viewport{});
	EXPECT_EQ((Scene_Viewport{ 400, 23, 800, 977 }.clipped_to(1600, 0)), Scene_Viewport{});
}

TEST(SceneViewportTest, ItContainsItsLeftAndTopEdgesButNotItsRightAndBottomEdges)
{
	const Scene_Viewport viewport{ 400, 23, 800, 977 };

	EXPECT_TRUE(viewport.contains(400.0, 23.0));
	EXPECT_TRUE(viewport.contains(1199.5, 999.5));
	EXPECT_FALSE(viewport.contains(1200.0, 500.0));
	EXPECT_FALSE(viewport.contains(800.0, 1000.0));
	EXPECT_FALSE(viewport.contains(399.9, 500.0));
	EXPECT_FALSE(viewport.contains(800.0, 22.9));
}

TEST(SceneViewportTest, TheBottomEdgeInOpenGlCountsTheRowsFromTheBottom)
{
	// a viewport below a menu bar of 23 pixels, down to the bottom of the framebuffer, starts at row 0 of OpenGL
	EXPECT_EQ((Scene_Viewport{ 400, 23, 800, 977 }.get_opengl_y(1000)), 0);
	// a viewport at the top of the framebuffer starts at its height minus the height of the viewport
	EXPECT_EQ((Scene_Viewport{ 0, 0, 100, 250 }.get_opengl_y(1000)), 750);
}
