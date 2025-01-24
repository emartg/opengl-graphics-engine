/* Shape.cpp
This file implements the Shape class (a derived class of Model),
which is used to create a shape from vertex, normal, texture coordinate, and index data; and draw it */

#pragma once

#include "Shape.h"

// Constructors
// ------------
Shape::Shape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
			 std::vector<GLuint> indices)
{
	loadShape(vertices, normals, texCoords, indices);
}

// Private Functions
// -----------------
void Shape::loadShape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
					  std::vector<GLuint> indices)
{
	// interleave vertex data
	std::vector<Vertex> interleavedVertexData;
	for (size_t i{}; i < vertices.size() / 3; ++i)
	{
		Vertex vertex;
		vertex.Position = glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
		vertex.Normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
		vertex.TexCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);
		interleavedVertexData.push_back(vertex);
	}

	// create a mesh from the interleaved vertex data
	meshes.push_back(Mesh(interleavedVertexData, indices, {}));
}