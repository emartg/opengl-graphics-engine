/*
* Shape.h
* This file defines the Shape class (a derived class of Model),
* which is used to create and draw a simple geomatric shape
* from vertex, normal, texture coordinate, index and texture data (if added).
*/

#pragma once

#include <vector>

#include "Model.h"

// enumeration class that allows to disriminate between different types of gizmos 
// and also indicate that the shape is not a gizmo without a boolean flag
enum class GizmoShapeType { DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT, NONE };

class Shape : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data. Two optional arguments can be specified for the shape: the albedo color (default is grey)
	// and the position vector (default is the zero vector)
	Shape(const std::string& name,
		  const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = glm::vec3(0.5f),
		  const glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f)
	);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data. Two optional arguments can be specified for the shape: the albedo color (default is grey)
	// and the position vector (default is the zero vector)
	Shape(const std::string& name,
		  const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
		  const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = glm::vec3(0.5f),
		  const glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f)
	);

	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data. Two optional arguments can be specified for the shape: the albedo color (default is grey)
	// and the position vector (default is the zero vector)
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape(const std::string& name,
		  const GLfloat* vertices, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = glm::vec3(0.5f),
		  const glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f)
	);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data. Two optional arguments can be specified for the shape: the albedo color (default is grey)
	// and the position vector (default is the zero vector)
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays, 
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays
	Shape(const std::string& name,
		  const GLfloat* positions, const GLfloat* normals,
		  const GLfloat* texCoords, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = glm::vec3(0.5f),
		  const glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f)
	);

	// Destructor
	// ----------
	~Shape() { nShapes--; } // decrements the number of shapes

	// Public Methods
	// --------------
	// Getters
	const glm::vec3& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::vec3& GetDefaultDirection() const { return defaultDirection; }
	const glm::vec3& GetDirection() const { return direction; }
	const GizmoShapeType GetGizmoShapeType() const { return gizmoShapeType; }

	// Setters
	void SetAlbedo(const glm::vec3& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	void SetDefaultDirection(const glm::vec3& defaultDirection) { this->defaultDirection = defaultDirection; }
	void SetDirection(const glm::vec3& direction) { this->direction = direction; }
	void SetGizmoShapeType(const GizmoShapeType gizmoShapeType) { this->gizmoShapeType = gizmoShapeType; }

	// Adds texture data to the shape
	void AddTextureData(const Texture* textures, const GLuint nTextures);
	void AddTextureData(const std::vector<Texture> textures);

	// Static Public Methods
	// ---------------------
	static GLuint GetNShapes() { return nShapes; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nShapes; // number of shapes created

	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	glm::vec3 albedo; // color of the shape in case no textures are used
	glm::vec3 position; // position vector for the shape in the scene
	glm::vec3 defaultDirection; // direction used to create the shape
	glm::vec3 direction; // direction vector for the shape in the scene
	//glm::vec3 height; // hex pyramid height (represents the inner cut-off angle for spotlights)
	//GLfloat baseArea{ 0.0f }; // hex pyramid base area (represents the outer cut-off angle for spotlights)

	GizmoShapeType gizmoShapeType{ GizmoShapeType::NONE }; // type of the gizmo, default is NONE (i.e, the shape wouldn't be a gizmo)

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