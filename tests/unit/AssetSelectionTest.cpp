/*
 * AssetSelectionTest.cpp
 * This file contains the unit tests of the Asset_Selection class, which needs no OpenGL context: selecting one item,
 * toggling items, selecting a range from the anchor (in any direction, and without a valid anchor), clearing,
 * removing items (and what is inside a folder), following the items that are moved or renamed, and keeping only the
 * items that still exist.
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/asset/Asset_Selection.h"

using Paths = std::vector<std::string>;

// The list that a window shows, in the order that the user sees it
static const Paths LIST{ "a.png", "b.png", "c.png", "d.png", "e.png" };

TEST(AssetSelectionTest, StartsEmpty)
{
	Asset_Selection selection;
	EXPECT_TRUE(selection.is_empty());
	EXPECT_TRUE(selection.get_paths().empty());
	EXPECT_TRUE(selection.get_anchor().empty());
	EXPECT_FALSE(selection.is_selected("a.png"));
}

TEST(AssetSelectionTest, SelectingAnItemReplacesTheSelectionAndMakesItTheAnchor)
{
	Asset_Selection selection;
	selection.select("a.png");
	selection.select("b.png");

	EXPECT_EQ(selection.get_paths(), Paths{ "b.png" });
	EXPECT_EQ(selection.get_anchor(), "b.png");
	EXPECT_TRUE(selection.is_selected("b.png"));
	EXPECT_FALSE(selection.is_selected("a.png"));
	EXPECT_FALSE(selection.is_empty());
}

TEST(AssetSelectionTest, TogglingAddsAndRemovesItemsInTheOrderOfTheClicks)
{
	Asset_Selection selection;
	selection.toggle("c.png");
	selection.toggle("a.png");
	selection.toggle("e.png");
	EXPECT_EQ(selection.get_paths(), (Paths{ "c.png", "a.png", "e.png" }));
	EXPECT_EQ(selection.get_anchor(), "e.png");

	selection.toggle("a.png"); // a selected item is removed
	EXPECT_EQ(selection.get_paths(), (Paths{ "c.png", "e.png" }));
	EXPECT_EQ(selection.get_anchor(), "a.png"); // the last clicked item is the anchor even if it was removed
}

TEST(AssetSelectionTest, SelectsARangeFromTheAnchorToAnItemInBothDirections)
{
	Asset_Selection selection;
	selection.select("b.png");
	selection.select_range(LIST, "d.png");
	EXPECT_EQ(selection.get_paths(), (Paths{ "b.png", "c.png", "d.png" }));
	EXPECT_EQ(selection.get_anchor(), "b.png"); // the anchor stays, so the range can be changed

	selection.select_range(LIST, "a.png"); // back and beyond the anchor
	EXPECT_EQ(selection.get_paths(), (Paths{ "a.png", "b.png" }));

	selection.select_range(LIST, "b.png"); // the anchor itself
	EXPECT_EQ(selection.get_paths(), Paths{ "b.png" });
}

TEST(AssetSelectionTest, ARangeReplacesTheItemsThatWereSelectedBefore)
{
	Asset_Selection selection;
	selection.select("a.png");
	selection.toggle("e.png"); // anchor: e.png
	selection.select_range(LIST, "c.png");
	EXPECT_EQ(selection.get_paths(), (Paths{ "c.png", "d.png", "e.png" }));
}

TEST(AssetSelectionTest, ARangeWithoutAValidAnchorSelectsOnlyTheItem)
{
	Asset_Selection selection;
	selection.select_range(LIST, "c.png"); // no anchor
	EXPECT_EQ(selection.get_paths(), Paths{ "c.png" });
	EXPECT_EQ(selection.get_anchor(), "c.png");

	selection.select("x.png"); // an anchor that is not in the list
	selection.select_range(LIST, "d.png");
	EXPECT_EQ(selection.get_paths(), Paths{ "d.png" });

	selection.select("a.png");
	selection.select_range(LIST, "x.png"); // an item that is not in the list
	EXPECT_EQ(selection.get_paths(), Paths{ "x.png" });
}

TEST(AssetSelectionTest, ClearingUnselectsEverythingAndForgetsTheAnchor)
{
	Asset_Selection selection;
	selection.select("a.png");
	selection.toggle("b.png");
	selection.clear();
	EXPECT_TRUE(selection.is_empty());
	EXPECT_TRUE(selection.get_anchor().empty());
}

TEST(AssetSelectionTest, RemovingAFolderRemovesWhatIsInsideItButNotItsNeighbors)
{
	Asset_Selection selection;
	selection.toggle("textures");
	selection.toggle("textures/wood.png");
	selection.toggle("textures/deep/a.png");
	selection.toggle("texturesque/b.png"); // another folder whose name starts the same
	selection.toggle("other.png");

	selection.remove("textures");
	EXPECT_EQ(selection.get_paths(), (Paths{ "texturesque/b.png", "other.png" }));
	EXPECT_EQ(selection.get_anchor(), "other.png");

	selection.remove("other.png");
	EXPECT_EQ(selection.get_paths(), Paths{ "texturesque/b.png" });
	EXPECT_TRUE(selection.get_anchor().empty()); // the anchor was removed
}

TEST(AssetSelectionTest, FollowsTheItemsThatAreMovedOrRenamed)
{
	Asset_Selection selection;
	selection.toggle("textures/wood.png");
	selection.toggle("textures/deep/a.png");
	selection.toggle("textures"); // the anchor
	selection.toggle("texturesque/b.png");
	selection.toggle("textures"); // toggling it again removes it, and it is the anchor
	selection.toggle("textures");

	selection.retarget("textures", "archive/textures");
	EXPECT_EQ(
		selection.get_paths(),
		(Paths{ "archive/textures/wood.png", "archive/textures/deep/a.png", "texturesque/b.png", "archive/textures" }));
	EXPECT_EQ(selection.get_anchor(), "archive/textures");
}

TEST(AssetSelectionTest, KeepsOnlyTheItemsThatStillExist)
{
	Asset_Selection selection;
	selection.toggle("a.png");
	selection.toggle("b.png");
	selection.toggle("c.png"); // the anchor

	selection.keep_if([](const std::string& path) { return path != "b.png" && path != "c.png"; });
	EXPECT_EQ(selection.get_paths(), Paths{ "a.png" });
	EXPECT_TRUE(selection.get_anchor().empty()); // the anchor no longer exists
}

TEST(AssetSelectionTest, TheTopLevelItemsLeaveOutWhatIsInsideAnotherSelectedItem)
{
	Asset_Selection selection;
	selection.toggle("textures/wood.png");
	selection.toggle("textures");
	selection.toggle("texturesque/b.png"); // another folder whose name starts the same
	selection.toggle("textures/deep/a.png");
	selection.toggle("other.png");

	EXPECT_EQ(selection.get_top_level_paths(), (Paths{ "textures", "texturesque/b.png", "other.png" }));
	EXPECT_EQ(selection.get_paths().size(), 5u); // the selection itself is not changed

	selection.clear();
	EXPECT_TRUE(selection.get_top_level_paths().empty());
}
