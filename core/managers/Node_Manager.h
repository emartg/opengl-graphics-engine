/*
* Node_Manager.h
* This file defines the Node_Manager class, which is responsible for managing Node objects in the engine.
* It provides methods to add, retrieve, and manage nodes of various types
* and keeps track of the number of nodes of each type.
* Also, when the Node_Manager is destroyed, it deallocates resources for all nodes.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <glad/glad.h> // holds OpenGL type definitions

// Forward declaration of classes to avoid cyclic includes
class Node;
class Camera;
class Light;
class Point_Light;
class Spotlight;
class Directional_Light;
class Model;
class Assimp_Model;
class Shape_Model;

// Forward declaration of enum to avoid cyclic includes
enum class Node_Type;

class Node_Manager
{
public:
	// Constructor
	// -----------
	Node_Manager() = default;

	// Destructor
	// ----------
	~Node_Manager();

	// Public Methods
	// --------------
	// Getters
	const std::vector<std::shared_ptr<Node>>& get_nodes(Node_Type node_type) const;
	const std::vector<std::shared_ptr<Node>>& get_nodes(const std::string& nodeType) const;

	// Gets a node by its index within its type category (throws out_of_range if index invalid)
	const std::shared_ptr<Node>& get_node_by_idx(Node_Type node_type, GLuint index) const;
	const std::shared_ptr<Node>& get_node_by_idx(const std::string& node_type, GLuint index) const;

	// Gets a node by its unique id (returns nullptr if not found)
	std::shared_ptr<Node> get_node_by_id(std::uint32_t id) const;

	// Adds a node to the node manager
	void add_node(std::shared_ptr<Node> node);

	// Removes a node from the node manager by its unique id
	void remove_node_by_id(std::uint32_t id);

private:
	// Private Attributes
	// ------------------
	// Unordered map that holds vectors of nodes, categorized by their type
	std::unordered_map<std::string, std::vector<std::shared_ptr<Node>>> nodes;

	// Private Methods
	// ---------------
	// Converts Node_Type enum to string for map lookup
	std::string node_type_to_string(Node_Type type) const;

};