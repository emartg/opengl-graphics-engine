/*
* Shape.cpp
* This file implements the Shape class (a derived class of Model),
* which is used to create a shape from vertex, normal, texture coordinate, index, and texture data;
* and draw it.
*/

#pragma once

#include "Shape.h"

// Constructors
// ------------
Shape::Shape(const std::string& name,
			 const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
			 const std::vector<Texture> textures)
	: Model(name), vertices{ processVertexData(vertices) }, indices{ indices }, textures{ textures }
{
	createMesh();
}

Shape::Shape(const std::string& name,
			 const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
			 const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
			 const std::vector<Texture> textures)
	: Model(name), vertices{ processVertexData(positions, normals, texCoords) }, indices{ indices }, textures{ textures }
{
	createMesh();
}

Shape::Shape(const std::string& name,
			 const GLfloat* vertices, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const Texture* textures, const GLuint nTextures)
	: Model(name)
{
	std::vector<GLfloat> vertexData{ vertices, vertices + nVertices * 8 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	std::vector<Texture> textureData{ textures, textures + nTextures };
	this->vertices = processVertexData(vertexData);
	this->indices = indexData;
	this->textures = textureData;

	createMesh();
}

Shape::Shape(const std::string& name,
			 const GLfloat* positions, const GLfloat* normals,
			 const GLfloat* texCoords, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const Texture* textures, const GLuint nTextures)
	: Model(name)
{
	std::vector<GLfloat> positionData{ positions, positions + nVertices * 3 };
	std::vector<GLfloat> normalData{ normals, normals + nVertices * 3 };
	std::vector<GLfloat> texCoordData{ texCoords, texCoords + nVertices * 2 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	std::vector<Texture> textureData{ textures, textures + nTextures };
	this->vertices = processVertexData(positionData, normalData, texCoordData);
	this->indices = indexData;
	this->textures = textureData;

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

void Shape::createMesh()
{
	Mesh mesh{ vertices, indices, textures };
	meshes.push_back(mesh);
}

