/*
* Shape.h
* This file defines the Shape class (a derived class of Model),
* which is used to create and draw a simple geomatric shape
* from vertex, normal, texture coordinate, index and texture data (if added).
*/

#pragma once

#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Model.h"

// enumeration class that allows to disriminate between different types of gizmos 
// and also indicate that the shape is not a gizmo without a boolean flag
enum class GizmoShapeType { NONE = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class Shape : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data.
	// Several optional arguments can be specified for the shape: the albedo color,
	// the position vector, the rotation quaternion, the scale vector, 
	// the forward vector and the mesh forward vector
	Shape(const std::string& name,
		  const std::vector<GLfloat> vertices, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD
	);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data. 
	// Several optional arguments can be specified for the shape: the albedo color,
	// the position vector, the rotation quaternion, the scale vector, 
	// the forward vector and the mesh forward vector
	Shape(const std::string& name,
		  const std::vector<GLfloat> positions, const std::vector<GLfloat> normals,
		  const std::vector<GLfloat> texCoords, const std::vector<GLuint> indices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD
	);

	// Constructor that creates a shape from interleaved position, normal and texture coordinate data, 
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays,
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays.
	// Several optional arguments can be specified for the shape: the albedo color,
	// the position vector, the rotation quaternion, the scale vector, 
	// the forward vector and the mesh forward vector
	Shape(const std::string& name,
		  const GLfloat* vertices, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD
	);

	// Constructor that creates a shape from separate position, normal, and texture coordinate data,
	// and index data.
	// This variant takes arrays as arguments instead of vectors, in case the data is laid out in arrays, 
	// and thus requires the number of elements (or vectors of 3 elements) conform each of the arrays.
	// Several optional arguments can be specified for the shape: the albedo color,
	// the position vector, the rotation quaternion, the scale vector, 
	// the forward vector and the mesh forward vector
	Shape(const std::string& name,
		  const GLfloat* positions, const GLfloat* normals,
		  const GLfloat* texCoords, const GLuint nVertices,
		  const GLuint* indices, const GLuint nIndices,
		  const glm::vec3 albedo = ALBEDO,
		  const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
		  const glm::vec3 scale = SCALE,
		  const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD
	);

	// Destructor
	// ----------
	~Shape() { nShapes--; } // decrements the number of shapes

	// Public Methods
	// --------------
	// Getters
	const glm::vec3& GetAlbedo() const { return albedo; }
	const glm::vec3& GetPosition() const { return position; }
	const glm::quat& GetRotation() const { return rotation; }
	// Returns the rotation as Euler angles in degrees
	const glm::vec3 GetRotationInEulerAngles() const;
	const glm::vec3& GetScale() const { return scale; }
	// Returns the shape's forward vector in world space, normalized
	const glm::vec3 GetForward() const;
	const glm::vec3& GetMeshForward() const { return meshForward; }
	const GizmoShapeType GetGizmoShapeType() const { return gizmoShapeType; }

	// Setters
	void SetAlbedo(const glm::vec3& albedo) { this->albedo = albedo; }
	void SetPosition(const glm::vec3& position) { this->position = position; }
	// Sets the rotation directly from a quaternion and updates the forward vector accordingly
	void SetRotation(const glm::quat& rotation);
	// Sets the rotation from Euler angles in degrees and updates the forward vector accordingly
	void SetRotationInEulerAngles(const glm::vec3 eulerAnglesDegrees);
	void SetScale(const glm::vec3& scale) { this->scale = scale; }
	// Aligns the shape's forward vector to the specified world forward vector
	void SetForward(const glm::vec3& worldForward);
	void SetMeshForward(const glm::vec3& meshForward) { this->meshForward = meshForward; }
	void SetGizmoShapeType(const GizmoShapeType gizmoShapeType) { this->gizmoShapeType = gizmoShapeType; }

	// Adds texture data to the shape
	void AddTextureData(const Texture* textures, const GLuint nTextures);
	void AddTextureData(const std::vector<Texture> textures);

	// Methods that return the full model matrix, or any of its components separately
	glm::mat4 GetModelMatrix() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;

	// Static Public Methods
	// ---------------------
	static GLuint GetNShapes() { return nShapes; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nShapes; // number of shapes created

	// Private Attributes
	// ------------------
	std::vector<Vertex> vertices; // vertex, normal and texture coordinate data
	std::vector<GLuint> indices; // indices that define the order in which the vertices are drawn
	std::vector<Texture> textures; // texture data (if any) associated with the shape

	glm::vec3 albedo; // current albedo (color)
	glm::vec3 position; // current position vector

	glm::quat rotation; // current orientation as a quaternion

	glm::vec3 scale; // current scale vector

	glm::vec3 forward; // shape's current forward vector in world space
	glm::vec3 meshForward; // shape's forward vector in local (mesh) space

	// type of the gizmo, default is NONE (i.e, the shape wouldn't be a gizmo)
	GizmoShapeType gizmoShapeType{ GizmoShapeType::NONE };

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

	// Private Static Attributes (for default values)
	// ----------------------------------------------
	static constexpr glm::vec3 ALBEDO{ 0.5f }; // default albedo (color) (grey)
	static constexpr glm::vec3 POSITION{ 0.0f }; // default position vector (zero vector)
	static constexpr glm::quat ROTATION{ 1.0f, 0.0f, 0.0f, 0.0f }; // default rotation quaternion (identity quaternion)
	static constexpr glm::vec3 SCALE{ 1.0f }; // default scale vector (unit vector)
	static constexpr glm::vec3 FORWARD{ 0.0f, 0.0f, 1.0f }; // default forward vector (+Z direction)

};