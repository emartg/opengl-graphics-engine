/*
* Shape.cpp
* This file implements the Shape class (a derived class of Model),
* which is used to create and draw geometric shapes from vertex, normal, texture coordinate,
* index and texture data (if added).
*/

#include "Shape.h"

// Static Private Attributes
// -------------------------
GLuint Shape::nShapes{}; // initialize the number of shapes in the scene to 0

// Constructors
// ------------
Shape::Shape(const std::string& name,
			 const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, NodeType::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, meshForward),
	vertices{ processVertexData(vertices) }, indices{ indices }
{
	createMesh();
	nShapes++;
}

Shape::Shape(const std::string& name,
			 const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
			 const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, NodeType::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, meshForward),
	vertices{ processVertexData(positions, normals, texCoords) }, indices{ indices }
{
	createMesh();
	nShapes++;
}

Shape::Shape(const std::string& name,
			 const GLfloat* vertices, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, NodeType::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, meshForward)
{
	std::vector<GLfloat> vertexData{ vertices, vertices + nVertices * 8 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	this->vertices = processVertexData(vertexData);
	this->indices = indexData;

	createMesh();
	nShapes++;
}

Shape::Shape(const std::string& name,
			 const GLfloat* positions, const GLfloat* normals,
			 const GLfloat* texCoords, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, NodeType::SHAPE_MODEL, // set the model type to SHAPE_MODEL
			albedo, position, rotation, scale, forward, meshForward)
{
	std::vector<GLfloat> positionData{ positions, positions + nVertices * 3 };
	std::vector<GLfloat> normalData{ normals, normals + nVertices * 3 };
	std::vector<GLfloat> texCoordData{ texCoords, texCoords + nVertices * 2 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	this->vertices = processVertexData(positionData, normalData, texCoordData);
	this->indices = indexData;

	createMesh();
	nShapes++;
}

// Public Methods
// --------------
void Shape::AddTextureData(const std::vector<std::shared_ptr<Texture>>& textures)
{
	this->textures = textures;
	meshes.clear();
	createMesh();
}

// Private Methods
// ---------------
std::vector<Vertex> Shape::processVertexData(std::vector<GLfloat> vertices)
{
	std::vector<Vertex> vertexData;
	for (GLuint i{}; i < vertices.size() / 8; ++i)
	{
		Vertex vertex;
		vertex.Position = glm::vec3(vertices[i * 8], vertices[i * 8 + 1], vertices[i * 8 + 2]);
		vertex.Normal = glm::vec3(vertices[i * 8 + 3], vertices[i * 8 + 4], vertices[i * 8 + 5]);
		vertex.TexCoords = glm::vec2(vertices[i * 8 + 6], vertices[i * 8 + 7]);
		vertexData.push_back(vertex);
	}
	return vertexData;
}

std::vector<Vertex> Shape::processVertexData(std::vector<GLfloat> vertices, std::vector<GLfloat> normals,
											 std::vector<GLfloat> texCoords)
{
	std::vector<Vertex> vertexData;
	for (GLuint i{}; i < vertices.size() / 3; i++)
	{
		Vertex vertex;
		vertex.Position = glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		vertex.Normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		vertex.TexCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);
		vertexData.push_back(vertex);
	}
	return vertexData;
}

void Shape::createMesh() { meshes.emplace_back(std::make_shared<Mesh>(vertices, indices, textures)); }

