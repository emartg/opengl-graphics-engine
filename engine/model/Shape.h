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
	// Constructor that creates a shape from vertices, normals, texture coordinates, and indices
	Shape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
		  std::vector<GLuint> indices);

private:
	// Private Functions
	// -----------------
	// Processes the data from the vectors and creates a Mesh object
	void loadShape(std::vector<GLfloat> vertices, std::vector<GLfloat> normals, std::vector<GLfloat> texCoords,
				   std::vector<GLuint> indices);
};