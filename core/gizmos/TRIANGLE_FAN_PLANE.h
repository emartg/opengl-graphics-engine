/*
* TF_PLANE.h
* This file defines the vertex data for a rectangular plane made of 4 triangles
* fanning out from a central vertex.
* It uses an index buffer to share vertices, and its defined both in array and vector form.
* The vertex data is separated into position, normal, and texture coordinate data, but there is also
* an array and a vector with the interleaved vertex data to test the Shape constructors that take
* interleaved vertex data directly.
*/

#pragma once

#include <vector>
#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Number of vertices and indices in the triangle fan plane 
// (5 vertices, 4 triangles * 3 indices = 12 indices)
const GLuint nTriangleFanPlaneVertices = 5;
const GLuint nTriangleFanPlaneIndices = 12;

// Plane dimensions used in shape construction
constexpr float TF_PLANE_WIDTH = 2.0f;
constexpr float TF_PLANE_HEIGHT = 2.0f;

// Forward direction of the plane defined below
constexpr glm::vec3 TF_PLANE_FORWARD{ 0.0f, 1.0f, 0.0f }; // facing up along the Y-axis

// Triangle fan plane vertices
const GLfloat triangleFanPlanePositionsArr[] = {
	-TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,	// 0: bottom-left
	 TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,	// 1: bottom-right
	 TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,	// 2: top-right
	-TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,	// 3: top-left
	 0.0f,					0.0f,	 0.0f					// 4: center
};

// Triangle fan plane normal data (all facing the same direction)
const GLfloat triangleFanPlaneNormalsArr[] = {
	0.0f,	1.0f,	0.0f,	// 0
	0.0f,	1.0f,	0.0f,	// 1
	0.0f,	1.0f,	0.0f,	// 2
	0.0f,	1.0f,	0.0f,	// 3
	0.0f,	1.0f,	0.0f	// 4: center
};

// Triangle fan plane texture coordinate data
const GLfloat triangleFanPlaneTexCoordsArr[] = {
	0.0f,	0.0f,	// 0
	1.0f,	0.0f,	// 1
	1.0f,	1.0f,	// 2
	0.0f,	1.0f,	// 3
	0.5f,	0.5f	// 4: center
};

// Triangle fan plane index data (4 triangles * 3 vertices)
// Winding order is counter-clockwise (CCW) when viewed from outside
const GLuint triangleFanPlaneIndicesArr[] = {
	0,  1,  4,  // bottom triangle
	1,  2,  4,  // right triangle
	2,  3,  4,  // top triangle
	3,  0,  4   // left triangle
};

// Interleaved triangle fan plane vertex data
const GLfloat triangleFanPlaneVerticesArr[] = {
	// position													// normal					// tex coords
	-TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	0.0f, // 0
	 TF_PLANE_WIDTH / 2,	0.0f,	-TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	0.0f, // 1
	 TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		1.0f,	1.0f, // 2
	-TF_PLANE_WIDTH / 2,	0.0f,	 TF_PLANE_HEIGHT / 2,		0.0f,	1.0f,	0.0f,		0.0f,	1.0f, // 3
	 0.0f,					0.0f,	 0.0f,						0.0f,	1.0f,	0.0f,		0.5f,	0.5f  // 4
};

// Triangle fan plane data in vector form
const std::vector<GLfloat> triangleFanPlanePositionsVec{
	std::begin(triangleFanPlanePositionsArr), std::end(triangleFanPlanePositionsArr)
};
const std::vector<GLfloat> triangleFanPlaneNormalsVec{
	std::begin(triangleFanPlaneNormalsArr), std::end(triangleFanPlaneNormalsArr)
};
const std::vector<GLfloat> triangleFanPlaneTexCoordsVec{
	std::begin(triangleFanPlaneTexCoordsArr), std::end(triangleFanPlaneTexCoordsArr)
};
const std::vector<GLuint> triangleFanPlaneIndicesVec{
	std::begin(triangleFanPlaneIndicesArr), std::end(triangleFanPlaneIndicesArr)
};
const std::vector<GLfloat> triangleFanPlaneVerticesVec{
	std::begin(triangleFanPlaneVerticesArr), std::end(triangleFanPlaneVerticesArr)
};

// Triangle fan plane direction line data (the normal direction from the center)
const GLfloat triangleFanPlaneDirectionLineArr[] = {
	// start: center (vertex 4, offset 4*8 = 32)
	triangleFanPlaneVerticesArr[32],
	triangleFanPlaneVerticesArr[33],
	triangleFanPlaneVerticesArr[34],

	// end: center + normal direction
	triangleFanPlaneVerticesArr[32] + triangleFanPlaneVerticesArr[35],
	triangleFanPlaneVerticesArr[33] + triangleFanPlaneVerticesArr[36],
	triangleFanPlaneVerticesArr[34] + triangleFanPlaneVerticesArr[37]
};

// Triangle fan plane direction line data in vector form
const std::vector<GLfloat> triangleFanPlaneDirectionLineVec{
	std::begin(triangleFanPlaneDirectionLineArr), std::end(triangleFanPlaneDirectionLineArr)
};
