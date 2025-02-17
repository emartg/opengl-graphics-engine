/*
* Shape.h
* This file defines the Shape class (a derived class of Model),
* which is used to create a shape from vertex, normal, texture coordinate, index
* and texture data (if added); and draw it.
*/

#pragma once

#include <vector>

#include "Model.h"

class Shape : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data
	Shape(const std::string& name,
		  const std::vector<GLfloat> vertices, const std::vector<GLuint> indices);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data
	Shape(const std::string& name,
		  const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
		  const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices);

	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape(const std::string& name,
		  const GLfloat* vertices, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays, 
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape(const std::string& name,
		  const GLfloat* positions, const GLfloat* normals,
		  const GLfloat* texCoords, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices);

	// Public Methods
	// --------------
	// Adds texture data to the shape
	void AddTextureData(const Texture* textures, const GLuint nTextures);
	void AddTextureData(const std::vector<Texture> textures);

private:
	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	// Private Methods
	// ---------------
	// Creates a vector of Vertex objects from interleaved vertex data
	std::vector<Vertex> processVertexData(std::vector<GLfloat> vertices);
	// Interleaves vertex, normal, and texture coordinate data into a single vector of Vertex objects
	std::vector<Vertex> processVertexData(std::vector<GLfloat> position,
										  std::vector<GLfloat> normals,
										  std::vector<GLfloat> texCoords);

	// Creates the mesh from the vertex, index and texture data
	void createMesh();

};