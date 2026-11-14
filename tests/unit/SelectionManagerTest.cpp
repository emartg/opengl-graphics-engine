/*
 * SelectionManagerTest.cpp
 * This file contains the unit tests of the rules of the Selection_Manager class that need no OpenGL context: which nodes of a
 * hierarchy the picking and the outline mask draw, and to which ancestor a repeated click climbs, for nodes that can or cannot
 * be selected (Node::set_is_selectable) and that are visible or not.
 */

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/Node.h"
#include "core/managers/Selection_Manager.h"

namespace
{
	// A node without geometry, enough to build the hierarchies of the tests (drawing it does nothing)
	class Test_Node : public Node
	{
	public:
		explicit Test_Node(const std::string& name) : Node(name, Node_Type::SHAPE_MODEL) {}

		void draw() const override {}
		void draw(const Shader& shader) const override {}
	};

	// Creates a node of the given name, as a shared pointer (the parent links of Node need one)
	std::shared_ptr<Node> make_node(const std::string& name)
	{
		return std::make_shared<Test_Node>(name);
	}

	// Returns the names of the nodes, in their order
	std::vector<std::string> names_of(const std::vector<std::shared_ptr<Node>>& nodes)
	{
		std::vector<std::string> names;
		for (const auto& node : nodes) names.push_back(node->get_name());
		return names;
	}
} // namespace

TEST(SelectionManagerTest, ANodeCanBeSelectedByDefault)
{
	const auto node = make_node("node");

	EXPECT_TRUE(node->get_is_selectable());
	node->set_is_selectable(false);
	EXPECT_FALSE(node->get_is_selectable());
}

TEST(SelectionManagerTest, ThePickingDrawsEveryVisibleNodeOfAHierarchy)
{
	const auto root  = make_node("root");
	const auto left  = make_node("left");
	const auto right = make_node("right");
	root->add_child(left);
	root->add_child(right);

	// the depth-first order of the picking: the root, then the children from the last one
	EXPECT_EQ(names_of(Selection_Manager::collect_pickable_nodes(root)), (std::vector<std::string>{ "root", "right", "left" }));
}

TEST(SelectionManagerTest, ThePickingLeavesOutANodeThatCannotBeSelectedButNotItsChildren)
{
	const auto root   = make_node("root");
	const auto helper = make_node("helper");
	const auto inside = make_node("inside");
	root->add_child(helper);
	helper->add_child(inside);
	helper->set_is_selectable(false);

	// a click goes through the helper, and still reaches the node inside it
	EXPECT_EQ(names_of(Selection_Manager::collect_pickable_nodes(root)), (std::vector<std::string>{ "root", "inside" }));
}

TEST(SelectionManagerTest, ThePickingLeavesOutAnInvisibleNodeWithItsDescendants)
{
	const auto root   = make_node("root");
	const auto hidden = make_node("hidden");
	const auto child  = make_node("child");
	root->add_child(hidden);
	hidden->add_child(child);
	hidden->set_is_visible(false);

	EXPECT_EQ(names_of(Selection_Manager::collect_pickable_nodes(root)), (std::vector<std::string>{ "root" }));
}

TEST(SelectionManagerTest, ARootThatCannotBeSelectedIsNotPickedButItsChildrenAre)
{
	const auto volume = make_node("volume");
	const auto marker = make_node("marker");
	volume->add_child(marker);
	volume->set_is_selectable(false);

	EXPECT_EQ(names_of(Selection_Manager::collect_pickable_nodes(volume)), (std::vector<std::string>{ "marker" }));
}

TEST(SelectionManagerTest, TheOutlineLeavesOutTheNodesThatCannotBeSelected)
{
	const auto root   = make_node("root");
	const auto helper = make_node("helper");
	const auto inside = make_node("inside");
	const auto hidden = make_node("hidden");
	root->add_child(helper);
	root->add_child(hidden);
	helper->add_child(inside);
	helper->set_is_selectable(false);
	hidden->set_is_visible(false);

	// the helper is not outlined, the node inside it is; visibility is not a rule of the outline mask (as before)
	EXPECT_EQ(names_of(Selection_Manager::collect_outlined_nodes(root)), (std::vector<std::string>{ "root", "hidden", "inside" }));
}

TEST(SelectionManagerTest, ARepeatedClickClimbsToTheNearestAncestorThatCanBeSelected)
{
	const auto root   = make_node("root");
	const auto helper = make_node("helper");
	const auto leaf   = make_node("leaf");
	root->add_child(helper);
	helper->add_child(leaf);
	helper->set_is_selectable(false);

	EXPECT_EQ(Selection_Manager::find_selectable_ancestor(leaf), root); // the helper is skipped
	EXPECT_EQ(Selection_Manager::find_selectable_ancestor(helper), root);
	EXPECT_EQ(Selection_Manager::find_selectable_ancestor(root), nullptr); // a root has no ancestor
	EXPECT_EQ(Selection_Manager::find_selectable_ancestor(nullptr), nullptr);
}
