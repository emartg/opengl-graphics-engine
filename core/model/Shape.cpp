/*
* Shape.cpp
* This file implements the Shape class (a derived class of Model),
* which is used to create and draw a simple geomatric shape
* from vertex, normal, texture coordinate, index and texture data (if added).
*/

#include "Shape.h"

#define GLM_ENABLE_EXPERIMENTAL // enable experimental features in GLM
#include <glm/gtx/quaternion.hpp> // for quaternion operations

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
	: Model(name),
	vertices{ processVertexData(vertices) }, indices{ indices },
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward }
{
	createMesh();
	nShapes++; // increments the number of shapes
}

Shape::Shape(const std::string& name,
			 const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
			 const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name),
	vertices{ processVertexData(positions, normals, texCoords) }, indices{ indices },
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward }
{
	createMesh();
	nShapes++; // increments the number of shapes
}

Shape::Shape(const std::string& name,
			 const GLfloat* vertices, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name),
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward }
{
	std::vector<GLfloat> vertexData{ vertices, vertices + nVertices * 8 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	this->vertices = processVertexData(vertexData);
	this->indices = indexData;

	createMesh();
	nShapes++; // increments the number of shapes
}

Shape::Shape(const std::string& name,
			 const GLfloat* positions, const GLfloat* normals,
			 const GLfloat* texCoords, const GLuint nVertices,
			 const GLuint* indices, const GLuint nIndices,
			 const glm::vec3 albedo,
			 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name),
	albedo{ albedo }, position{ position }, rotation{ rotation }, scale{ scale },
	forward{ forward }, meshForward{ meshForward }
{
	std::vector<GLfloat> positionData{ positions, positions + nVertices * 3 };
	std::vector<GLfloat> normalData{ normals, normals + nVertices * 3 };
	std::vector<GLfloat> texCoordData{ texCoords, texCoords + nVertices * 2 };
	std::vector<GLuint> indexData{ indices, indices + nIndices };
	this->vertices = processVertexData(positionData, normalData, texCoordData);
	this->indices = indexData;

	createMesh();
	nShapes++; // increments the number of shapes
}

// Public Methods
// --------------
const glm::vec3 Shape::GetRotationInEulerAngles() const
{
	// convert the quaternion rotation to Euler angles in radians, then to degrees
	return glm::degrees(glm::eulerAngles(this->rotation));
}
const glm::vec3 Shape::GetForward() const
{
	// return the forward vector in world space, normalized
	return glm::normalize(this->rotation * meshForward);
}

void Shape::SetRotation(const glm::quat& rotation)
{
	// normalize to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotation);
	// update forward vector based on the new rotation
	this->forward = glm::normalize(this->rotation * meshForward);
}
void Shape::SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees)
{
	// convert degrees to radians for glm::quat constructor
	glm::vec3 eulerAnglesRadians = glm::radians(eulerAnglesDegrees);
	// create quaternion from Euler angles
	glm::quat newRotation = glm::quat(eulerAnglesRadians);
	// use the SetRotation method to set the new rotation as a quaternion
	SetRotation(newRotation);
}
void Shape::SetForward(const glm::vec3& worldForward)
{
	// ensure the worldForward vector is normalized
	glm::vec3 normWorldForward = glm::normalize(worldForward);
	// rotate the meshForward vector to align with the worldForward vector
	glm::quat rotationQuat = glm::rotation(meshForward, normWorldForward);
	// normalize the rotation quaternion to ensure a valid rotation quaternion
	this->rotation = glm::normalize(rotationQuat);
	// update the forward vector based on the new rotation
	this->forward = normWorldForward;
}

void Shape::AddTextureData(const Texture* textures, const GLuint nTextures)
{
	for (GLuint i{}; i < nTextures; i++)
		this->textures.push_back(textures[i]);
	meshes.clear();
	createMesh();
}

void Shape::AddTextureData(const std::vector<Texture> textures)
{
	for (const Texture& texture : textures)
		this->textures.push_back(texture);
	meshes.clear();
	createMesh();
}

glm::mat4 Shape::GetModelMatrix() const
{
	glm::mat4 model = glm::mat4{ 1.0f };
	model = glm::translate(model, position); // apply translation
	model *= glm::mat4_cast(rotation); // apply rotation
	model = glm::scale(model, scale); // apply scaling
	return model;
}
glm::mat4 Shape::GetTranslationMatrix() const
{
	glm::mat4 translationMatrix = glm::mat4{ 1.0f };
	translationMatrix = glm::translate(translationMatrix, position);
	return translationMatrix;
}
glm::mat4 Shape::GetRotationMatrix() const
{
	glm::mat4 rotationMatrix = glm::mat4{ 1.0f };
	rotationMatrix *= glm::mat4_cast(rotation);
	return rotationMatrix;
}
glm::mat4 Shape::GetScaleMatrix() const
{
	glm::mat4 scaleMatrix = glm::mat4{ 1.0f };
	scaleMatrix = glm::scale(scaleMatrix, scale);
	return scaleMatrix;
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

void Shape::createMesh() { meshes.emplace_back(vertices, indices, textures); }

