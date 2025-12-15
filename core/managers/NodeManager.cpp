/*
* NodeManager.cpp
* This file implements the NodeManager class, which is responsible for managing Node objects in the engine.
* It provides methods to add, retrieve, and manage nodes of various types
* and keeps track of the number of nodes of each type.
* Also, when the NodeManager is destroyed, it deallocates resources for all nodes.
*/

#include "NodeManager.h"

#include "../Node.h"
#include "../camera/Camera.h"
#include "../light/Light.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"
#include "../light/DirectionalLight.h"
#include "../model/Model.h"
#include "../model/AssimpModel.h"
#include "../model/Shape.h"

// Destructor
// ----------
NodeManager::~NodeManager()
{
	// for each node type in the map, iterate through the vector of nodes and deallocate resources
	for (auto& [nodeType, nodes] : m_nodes)
		for (auto& node : nodes)
			if (node) // check if the node is not null
				node->DeallocateResources(); // deallocate resources for the node
}

// Public Methods
// --------------
const std::vector<std::shared_ptr<Node>>& NodeManager::GetNodes(NodeType nodeType) const
{
	std::string nodeTypeStr = nodeTypeToString(nodeType); // convert NodeType to str for map lookup

	return GetNodes(nodeTypeStr); // delegate to the string-based GetNodes method
}

const std::vector<std::shared_ptr<Node>>& NodeManager::GetNodes(const std::string& nodeType) const
{
	// find the node type in the map
	auto it = m_nodes.find(nodeType);

	// ensure at least one node of the type exists
	if (it != m_nodes.end())
		return it->second; // return the vector of nodes of the specified type

	// return an empty vector if the node type is not found
	static const std::vector<std::shared_ptr<Node>> empty;
	return empty;
}

const std::shared_ptr<Node>& NodeManager::GetNodeByIndex(NodeType type, GLuint index) const
{
	std::string nodeTypeStr = nodeTypeToString(type); // convert NodeType to str for map lookup

	return GetNodeByIndex(nodeTypeStr, index); // delegate to the string-based GetNodeByIndex method
}

const std::shared_ptr<Node>& NodeManager::GetNodeByIndex(const std::string& nodeType, GLuint index) const
{
	// find the node type in the map
	auto it = m_nodes.find(nodeType);

	// ensure at least one node of the type exists and index is within bounds
	if (it != m_nodes.end() && index < it->second.size())
		return it->second[index]; // return the node of the specified type at the specified index

	// return an empty shared_ptr if the node type is not found or index is out of bounds
	static const std::shared_ptr<Node> empty;
	return empty;
}

std::shared_ptr<Node> NodeManager::GetNodeById(std::uint32_t id) const
{
	for (const auto& [nodeType, nodes] : m_nodes)
	{ // iterate through each node type in the map
		// search for the node with the specified id in the vector of nodes of the current type
		auto it = std::find_if(nodes.begin(), nodes.end(),
							   [id](const std::shared_ptr<Node>& node) { return node->GetId() == id; });
		if (it != nodes.end()) return *it; // node with the specified id found, return it
	}

	// if no node with the specified id was found, return nullptr
	return nullptr;
}

void NodeManager::AddNode(std::shared_ptr<Node> node)
{
	// convert NodeType to string for map lookup
	std::string nodeType = nodeTypeToString(node->GetNodeType());

	// add node to the corresponding vector in the map
	m_nodes[nodeType].emplace_back(node);

	// also, add all model-type nodes to the generic "MODEL" category for hierarchical access
	if (nodeType.find("MODEL") != std::string::npos)
	{
		// add the generic MODEL entry as well for all model types
		m_nodes["MODEL"].emplace_back(node);
	}

	// print the type, id, and name of the added node
	std::cout << "[INFO::NODEMANAGER::AddNode] Added "
		<< nodeType << " node with ID " << node->GetId() << " and name " << node->GetName() << std::endl;
}
void NodeManager::RemoveNodeById(std::uint32_t id)
{
	bool found = false; // flag to track if a node with the specified id was found
	std::string foundNodeType; // to store the type of the found node
	std::shared_ptr<Node> nodeToRemove; // store the node to check for children

	// first, find the node with the specified id in the map
	for (auto& [nodeType, nodes] : m_nodes)
	{ // iterate through each node type in the map
		// search for the node with the specified id in the vector of nodes of the current type
		auto nodeIt = std::find_if(nodes.begin(), nodes.end(),
								   [id](const std::shared_ptr<Node>& node) { return node->GetId() == id; });
		if (nodeIt != nodes.end() && !found)
		{ // node with the specified id found, store the iterator, type, and mark as found, and then break
			nodeToRemove = *nodeIt;
			foundNodeType = nodeType;
			found = true;
			break; // found the node, no need to continue searching
		}
	}

	if (!found)
	{ // if no node with the specified id was found, print an error message and return
		std::cerr << "[ERROR::NODEMANAGER::RemoveNodeById] No node found with ID " << id << std::endl;
		return;
	}

	// if the node has children, remove them first (recursively)
	if (nodeToRemove)
	{
		auto children = nodeToRemove->GetChildren();
		if (!children.empty())
		{
			std::cout << "[INFO::NODEMANAGER::RemoveNodeById] Node with ID " << id
				<< " has " << children.size() << " children.\n\tRemoving children first..." << std::endl;

			for (const auto& child : children) if (child) RemoveNodeById(child->GetId()); // recursive call
		}
	}

	// now remove the node from all categories
	std::cout << "[INFO::NODEMANAGER::RemoveNodeById] Removed "
		<< foundNodeType << " node with ID " << id << " and name " << nodeToRemove->GetName() << std::endl;

	for (auto& [nodeType, nodes] : m_nodes)
	{ // iterate through each node type in the map
		// remove the node with the specified id from the vector
		auto it = std::remove_if(nodes.begin(), nodes.end(),
								 [id](const std::shared_ptr<Node>& node) { return node->GetId() == id; });

		if (it != nodes.end())
		{ // if the node was found and removed, erase the "removed" elements from the vector
			nodes.erase(it, nodes.end());
		}
	}
}

const GLuint NodeManager::GetNNodes() const { return Node::GetNNodes(); }
const GLuint NodeManager::GetNCameras() const { return Camera::GetNCameras(); }
const GLuint NodeManager::GetNLights() const { return Light::GetNLights(); }
const GLuint NodeManager::GetNPointLights() const { return PointLight::GetNPointLights(); }
const GLuint NodeManager::GetNSpotlights() const { return Spotlight::GetNSpotlights(); }
const GLuint NodeManager::GetNDirectionalLights() const { return DirectionalLight::GetNDirectionalLights(); }
const GLuint NodeManager::GetNModels() const { return Model::GetNModels(); }
const GLuint NodeManager::GetNAssimpModels() const { return AssimpModel::GetNAssimpModels(); }
const GLuint NodeManager::GetNShapes() const { return Shape::GetNShapes(); }

// Private Methods
// ---------------
std::string NodeManager::nodeTypeToString(NodeType type) const
{
	std::string nodeTypeStr{};
	switch (type)
	{
		case NodeType::CAMERA:
			nodeTypeStr = "CAMERA";
			break;
		case NodeType::LIGHT:
			nodeTypeStr = "LIGHT";
			break;
		case NodeType::MODEL:
			nodeTypeStr = "MODEL";
			break;
		case NodeType::COMPOSITE_MODEL:
			nodeTypeStr = "COMPOSITE_MODEL";
			break;
		case NodeType::COMPOSITE_ASSIMP_MODEL:
			nodeTypeStr = "COMPOSITE_ASSIMP_MODEL";
			break;
		case NodeType::ASSIMP_MODEL:
			nodeTypeStr = "ASSIMP_MODEL";
			break;
		case NodeType::COMPOSITE_SHAPE_MODEL:
			nodeTypeStr = "COMPOSITE_SHAPE_MODEL";
			break;
		case NodeType::SHAPE_MODEL:
			nodeTypeStr = "SHAPE_MODEL";
			break;
		case NodeType::SHADER:
			nodeTypeStr = "SHADER";
			break;
		case NodeType::TEXTURE:
			nodeTypeStr = "TEXTURE";
			break;
		default:
			std::cerr << "NodeType not defined!" << std::endl;
			throw std::invalid_argument("Invalid node type");
	}

	return nodeTypeStr;
}