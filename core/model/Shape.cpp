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
Shape::Shape(std::vector<GLfloat> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
	: indices(indices), textures(textures)
{
	vertexData = processVertexData(vertices);
	createMesh();
}

Shape::Shape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
			 std::vector<GLuint> indices, std::vector<Texture> textures)
	: indices(indices), textures(textures)
{
	vertexData = processVertexData(vertices, normals, texCoords);
	createMesh();
}

// Private Functions
// -----------------
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
	Mesh mesh{ vertexData, indices, textures };
	meshes.push_back(mesh);
}

