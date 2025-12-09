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

	for (auto& [nodeType, nodes] : m_nodes)
	{ // iterate through each node of each type in the map
		// try to find the node with the specified id in the vector of nodes of the current type
		auto it = std::remove_if(nodes.begin(), nodes.end(),
								 [id](const std::shared_ptr<Node>& node) { return node->GetId() == id; });

		if (it != nodes.end())
		{ // if a node with the id was found, store the type and erase it
			if (!found)
			{ // only log once for the specific type
				foundNodeType = nodeType;
				std::cout << "[INFO::NODEMANAGER::RemoveNodeById] Removed "
					<< nodeType << " node with ID: " << id << std::endl;
				found = true;
			}
			nodes.erase(it, nodes.end());
			// continue to remove from other collections (e.g., "MODEL")
		}
	}

	if (!found)
	{ // if no node with the specified ID was found, print an error message
		std::cerr << "[ERROR::NODEMANAGER::RemoveNodeById] No node found with ID: " << id << std::endl;
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