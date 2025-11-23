/*
* Shape.h
* This file defines the Shape class (a derived class of ModelComponent),
* which is used to create and draw geometric shapes from vertex, normal, texture coordinate, index
* and texture data (if added).
*/

#pragma once

#include "ModelComponent.h"

class Shape : public ModelComponent
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data.
	Shape(const std::string& name,
		  const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data. 
	Shape(const std::string& name,
		  const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
		  const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD);

	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays.
	Shape(const std::string& name,
		  const GLfloat* vertices, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays, 
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays.
	Shape(const std::string& name,
		  const GLfloat* positions, const GLfloat* normals,
		  const GLfloat* texCoords, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD);

	// Destructor
	// ----------
	~Shape() { nShapes--; }

	// Public Methods
	// --------------
	// Loads the Shape
	void Load() override {}

	// Deallocates all the resources of the Shape, that is, all its meshes
	void DeallocateResources() override { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Draws only its own meshes
	void Draw() const override { for (const Mesh& mesh : meshes) mesh.Draw(); }
	// Draws only its own meshes with the specified shader (binds the textures before drawing)
	void Draw(const Shader& shader) const override
	{
		// need to cast away the constness of the shader to use it
		auto& nonConstShader = const_cast<Shader&>(shader);
		nonConstShader.Use(); // activate the shader program
		// set the appropiate world model matrix uniform per model before drawing
		nonConstShader.SetMat4("model", GetWorldModelMatrix());

		for (const Mesh& mesh : meshes)
		{
			const_cast<Mesh&>(mesh).BindTextures(nonConstShader); // bind the textures
			mesh.Draw();
		}
	}

	// Adds texture data to the shape
	void AddTextureData(const Texture* textures, const GLuint nTextures);
	void AddTextureData(const std::vector<Texture> textures);

	// Static Public Methods
	// ---------------------
	static GLuint GetNShapes() { return nShapes; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nShapes; // number of shapes in the scene

	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices; // vertex, normal and texture coordinate data
	std::vector<GLuint> indices; // indices that define the order in which the vertices are drawn
	std::vector<Texture> textures; // texture data (if any) associated with the shape

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