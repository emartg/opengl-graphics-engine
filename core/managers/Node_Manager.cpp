/*
 * Node_Manager.cpp
 * This file implements the Node_Manager class, which is responsible for managing Node objects in the engine.
 * It provides methods to add, retrieve, and manage nodes of various types
 * and keeps track of the number of nodes of each type.
 * Also, when the Node_Manager is destroyed, it deallocates resources for all nodes.
 */

#include "Node_Manager.h"

#include "../Node.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/Point_Light.h"
#include "../light/Spotlight.h"
#include "../light/Directional_Light.h"
#include "../model/Model.h"
#include "../model/Assimp_Model.h"
#include "../model/Shape_Model.h"

// Destructor
// ----------
Node_Manager::~Node_Manager()
{
	// for each node type in the map, iterate through the vector of nodes and deallocate resources
	for (auto& [node_type, nodes] : nodes)
		for (auto& node : nodes)
			if (node)                         // check if the node is not null
				node->deallocate_resources(); // deallocate resources for the node
}

// Public Methods
// --------------
const std::vector<std::shared_ptr<Node>>& Node_Manager::get_nodes(Node_Type node_type) const
{
	std::string node_type_string = node_type_to_string(node_type); // convert Node_Type to str for map lookup

	return get_nodes(node_type_string); // delegate to the string-based get_nodes method
}

const std::vector<std::shared_ptr<Node>>& Node_Manager::get_nodes(const std::string& node_type) const
{
	// find the node type in the map
	auto it = nodes.find(node_type);

	// ensure at least one node of the type exists
	if (it != nodes.end())
		return it->second; // return the vector of nodes of the specified type

	// return an empty vector if the node type is not found
	static const std::vector<std::shared_ptr<Node>> empty;
	return empty;
}

const std::shared_ptr<Node>& Node_Manager::get_node_by_idx(Node_Type node_type, GLuint index) const
{
	std::string node_type_string = node_type_to_string(node_type); // convert Node_Type to str for map lookup

	return get_node_by_idx(node_type_string, index); // delegate to the string-based get_node_by_idx method
}

const std::shared_ptr<Node>& Node_Manager::get_node_by_idx(const std::string& node_type, GLuint index) const
{
	// find the node type in the map
	auto it = nodes.find(node_type);

	// ensure at least one node of the type exists and index is within bounds
	if (it != nodes.end() && index < it->second.size())
		return it->second[index]; // return the node of the specified type at the specified index

	// return an empty shared_ptr if the node type is not found or index is out of bounds
	static const std::shared_ptr<Node> empty;
	return empty;
}

std::shared_ptr<Node> Node_Manager::get_node_by_id(std::uint32_t id) const
{
	for (const auto& [node_type, nodes_of_type] : this->nodes)
	{ // iterate through each node type in the map
		// search for the node with the specified id in the vector of nodes of the current type
		auto it = std::find_if(nodes_of_type.begin(), nodes_of_type.end(), [id](const std::shared_ptr<Node>& node) {
			return node->get_id() == id;
		});
		if (it != nodes_of_type.end())
			return *it; // node with the specified id found, return it
	}

	// if no node with the specified id was found, return nullptr
	return nullptr;
}

void Node_Manager::add_node(std::shared_ptr<Node> node)
{
	// convert Node_Type to string for map lookup
	std::string node_type = node_type_to_string(node->get_type());

	// add node to the corresponding vector in the map
	nodes[node_type].emplace_back(node);

	// also, add all model-type nodes to the generic "MODEL" category for hierarchical access
	if (node_type.find("MODEL") != std::string::npos)
	{
		// add the generic MODEL entry as well for all model types
		nodes["MODEL"].emplace_back(node);
	}

	// print the type, id, and name of the added node
	std::cout << "[INFO::NODEMANAGER::add_node] Added " << node_type << " node with id " << node->get_id() << " and name '"
	          << node->get_name() << "'" << std::endl;
}
void Node_Manager::remove_node_by_id(std::uint32_t id)
{
	bool                  found = false;   // flag to track if a node with the specified id was found
	std::string           found_node_type; // to store the type of the found node
	std::shared_ptr<Node> node_to_remove;  // store the node to check for children

	// first, find the node with the specified id in the map
	for (auto& [node_type, nodes] : nodes)
	{ // iterate through each node type in the map
		// search for the node with the specified id in the vector of nodes of the current type
		auto node_it = std::find_if(nodes.begin(), nodes.end(), [id](const std::shared_ptr<Node>& node) { return node->get_id() == id; });
		if (node_it != nodes.end() && !found)
		{ // node with the specified id found, store the iterator, type, and mark as found, and then break
			node_to_remove  = *node_it;
			found_node_type = node_type;
			found           = true;
			break; // found the node, no need to continue searching
		}
	}

	if (!found)
	{ // if no node with the specified id was found, print an error message and return
		std::cerr << "[ERROR::NODEMANAGER::remove_node_by_id] No node found with id " << id << std::endl;
		return;
	}

	// remove the node from its parent's children vector before the recursive deletion,
	// which would otherwise leave dangling references in the parent's children, leading to
	// ghost nodes appearing in the scene graph tree and viewport
	if (node_to_remove)
	{
		auto parent = node_to_remove->get_parent();
		if (parent)
		{ // if the node has a parent, remove it from the parent's children vector and print info
			parent->remove_child(node_to_remove);

			std::cout << "[INFO::NODEMANAGER::remove_node_by_id] Severed node id " << id << " from parent with id " << parent->get_id()
			          << std::endl;
		}
	}

	// if the node to remove has children, remove them first recursively
	if (node_to_remove)
	{
		auto children = node_to_remove->get_children();
		if (!children.empty())
		{ // if the node has children, print info and recursively remove each child, then print confirmation
			std::cout << "[INFO::NODEMANAGER::remove_node_by_id] Node with id " << id << " has " << children.size()
			          << " children. Removing children first..." << std::endl;

			// create a copy of the children vector to avoid iteratior invalidation during removal
			std::vector<std::shared_ptr<Node>> children_copy = children;
			for (const auto& child : children_copy)
				if (child)
					remove_node_by_id(child->get_id()); // recursive call to remove each child by its id

			std::cout << "[INFO::NODEMANAGER::remove_node_by_id] All children of node with id " << id << " have been removed" << std::endl;
		}
	}

	// now actually remove the node from all relevant vectors in the map
	for (auto& [node_type, nodes] : nodes)
	{ // iterate through each node type in the map
		// remove the node with the specified id from the vector
		auto it = std::remove_if(nodes.begin(), nodes.end(), [id](const std::shared_ptr<Node>& node) { return node->get_id() == id; });

		if (it != nodes.end())
		{ // if the node was found and removed, erase the "removed" elements from the vector
			nodes.erase(it, nodes.end());
		}
	}

	// print confirmation message
	std::cout << "[INFO::NODEMANAGER::remove_node_by_id] Removed " << found_node_type << " node with id " << id << std::endl;
}

// Private Methods
// ---------------
std::string Node_Manager::node_type_to_string(Node_Type type) const
{
	std::string node_type_string{};
	switch (type)
	{
		case Node_Type::CAMERA: node_type_string = "CAMERA"; break;
		case Node_Type::LIGHT: node_type_string = "LIGHT"; break;
		case Node_Type::MODEL: node_type_string = "MODEL"; break;
		case Node_Type::COMPOSITE_MODEL: node_type_string = "COMPOSITE_MODEL"; break;
		case Node_Type::COMPOSITE_ASSIMP_MODEL: node_type_string = "COMPOSITE_ASSIMP_MODEL"; break;
		case Node_Type::ASSIMP_MODEL: node_type_string = "ASSIMP_MODEL"; break;
		case Node_Type::COMPOSITE_SHAPE_MODEL: node_type_string = "COMPOSITE_SHAPE_MODEL"; break;
		case Node_Type::SHAPE_MODEL: node_type_string = "SHAPE_MODEL"; break;
		case Node_Type::SHADER: node_type_string = "SHADER"; break;
		case Node_Type::TEXTURE: node_type_string = "TEXTURE"; break;
		default: std::cerr << "Node_Type not defined!" << std::endl; throw std::invalid_argument("Invalid node type");
	}

	return node_type_string;
}