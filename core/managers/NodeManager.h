/*
* NodeManager.h
* This file defines the NodeManager class, which is responsible for managing Node objects in the engine.
* It provides methods to add, retrieve, and manage nodes of various types
* and keeps track of the number of nodes of each type.
* Also, when the NodeManager is destroyed, it deallocates resources for all nodes.
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
class PointLight;
class Spotlight;
class DirectionalLight;
class Model;
class AssimpModel;
class Shape;

// Forward declaration of enum to avoid cyclic includes
enum class NodeType;

class NodeManager
{
public:
	// Constructor
	// -----------
	NodeManager() = default;

	// Destructor
	// ----------
	~NodeManager();

	// Public Methods
	// --------------
	// Getters
	const std::vector<std::shared_ptr<Node>>& GetNodes(NodeType nodeType) const;
	const std::vector<std::shared_ptr<Node>>& GetNodes(const std::string& nodeType) const;

	// Gets a node by its index within its type category (throws out_of_range if index invalid)
	const std::shared_ptr<Node>& GetNodeByIndex(NodeType nodeType, GLuint index) const;
	const std::shared_ptr<Node>& GetNodeByIndex(const std::string& nodeType, GLuint index) const;

	// Gets a node by its unique id (returns nullptr if not found)
	std::shared_ptr<Node> GetNodeById(std::uint32_t id) const;

	// Adds a node to the node manager
	void AddNode(std::shared_ptr<Node> node);

	// Removes a node from the node manager by its unique id
	void RemoveNodeById(std::uint32_t id);

	// Getters for the number of nodes of different types
	const GLuint GetNNodes() const;
	const GLuint GetNCameras() const;
	const GLuint GetNLights() const;
	const GLuint GetNPointLights() const;
	const GLuint GetNSpotlights() const;
	const GLuint GetNDirectionalLights() const;
	const GLuint GetNModels() const;
	const GLuint GetNAssimpModels() const;
	const GLuint GetNShapes() const;

private:
	// Private Attributes
	// ------------------
	// Unordered map that holds vectors of nodes, categorized by their type
	std::unordered_map<std::string, std::vector<std::shared_ptr<Node>>> m_nodes;

	// Private Methods
	// ---------------
	// Converts NodeType enum to string for map lookup
	std::string nodeTypeToString(NodeType type) const;

};