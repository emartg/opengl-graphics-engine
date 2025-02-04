/*
* Shape.h
* This file defines the Shape class (a derived class of Model),
* which is used to create a shape from vertex, normal, texture coordinate, index, and texture data;
* and draw it.
*/

#pragma once

#include <vector>

#include "Model.h"

class Shape : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved vertex, normal and texture coordinate data, 
	// index data, and texture data
	Shape(std::vector<GLfloat> vertices, std::vector<GLuint> indices, std::vector<Texture> textures = {});
	// Constructor that creates a shape from separate vertex, normal, and texture coordinate data,
	// index data, and texture data
	Shape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
		  std::vector<GLuint> indices, std::vector<Texture> textures = {});

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex> vertexData;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	// Private Functions
	// -----------------
	// Creates a vector of Vertex objects from interleaved vertex data
	std::vector<Vertex> processVertexData(std::vector<GLfloat> vertices);
	// Interleaves vertex, normal, and texture coordinate data into a single vector of Vertex objects
	std::vector<Vertex> processVertexData(std::vector<GLfloat> vertices, std::vector<GLfloat> normals,
										  std::vector<GLfloat> texCoords);

	// Creates the mesh from the vertex, index and texture data
	void createMesh();
};