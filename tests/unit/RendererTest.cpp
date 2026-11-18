/*
 * RendererTest.cpp
 * This file contains the unit tests of the Renderer class that need no OpenGL context: the size of the faces of the cube map
 * that an equirectangular (HDR) skybox is converted to, which follows the width of the image. The conversion itself needs a
 * context, so it is covered by the smoke tests of the App, whose scenes use HDR skyboxes.
 */

#include <gtest/gtest.h>

#include "core/renderer/Renderer.h"

TEST(RendererTest, TheFacesOfASkyboxAreHalfTheWidthOfItsImage)
{
	// the HDR skyboxes of the engine are 4096 x 2048: faces of 2048, where they used to be 4096
	EXPECT_EQ(Renderer::get_skybox_face_size(4096, 16384), 2048u);
	EXPECT_EQ(Renderer::get_skybox_face_size(2048, 16384), 1024u);
	EXPECT_EQ(Renderer::get_skybox_face_size(8192, 16384), 4096u);
}

TEST(RendererTest, TheFacesOfASkyboxAreAPowerOfTwo)
{
	// half of 3000 is 1500, rounded up
	EXPECT_EQ(Renderer::get_skybox_face_size(3000, 16384), 2048u);
	EXPECT_EQ(Renderer::get_skybox_face_size(1000, 16384), 512u);
}

TEST(RendererTest, TheFacesOfASkyboxStayWithinTheirLimits)
{
	// a very large image is kept at the largest size, and a very small or invalid width at the smallest
	EXPECT_EQ(Renderer::get_skybox_face_size(32768, 65536), Renderer::MAX_SKYBOX_FACE_SIZE);
	EXPECT_EQ(Renderer::get_skybox_face_size(16, 16384), Renderer::MIN_SKYBOX_FACE_SIZE);
	EXPECT_EQ(Renderer::get_skybox_face_size(0, 16384), Renderer::MIN_SKYBOX_FACE_SIZE);
	EXPECT_EQ(Renderer::get_skybox_face_size(-1, 16384), Renderer::MIN_SKYBOX_FACE_SIZE);

	// the largest cube map of the driver wins, and a maximum of 0 means no such limit
	EXPECT_EQ(Renderer::get_skybox_face_size(8192, 1024), 1024u);
	EXPECT_EQ(Renderer::get_skybox_face_size(4096, 0), 2048u);
}
