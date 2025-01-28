/* Shape.h
This file defines the Shape class (a derived class of Model),
which is used to create a shape from vertex, normal, texture coordinate, and index data; and draw it */

#pragma once

#include <vector>

#include "Model.h"

class Shape : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved vertex, normal and texture coordinate data, and index data
	Shape(std::vector<GLfloat> interleavedVertexData, std::vector<GLuint> indices);
	// Constructor that creates a shape from vertices, normals, texture coordinates, and indices
	Shape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
		  std::vector<GLuint> indices);

private:
	// Private Functions
	// -----------------
	// Interleaves vertex data (position, normal, texture coordinates), 
	// that is, combines the data into a single vector of Vertex objects,
	// where each Vertex object contains the position, normal, and texture coordinates of a vertex
	std::vector<Vertex> interleaveVertexData(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords);

};