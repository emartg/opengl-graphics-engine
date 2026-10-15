/*
 * MeshGeometryTest.cpp
 * This file contains the unit test of the interface of the Mesh_Geometry class that does not require an OpenGL
 * context: the vertices and the indices of a geometry, which it keeps on the CPU, are read-only getters. A geometry
 * uploads its data to the GPU when it is created, so its behavior is covered by the smoke tests of the App.
 */

#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "core/model/mesh/Mesh_Geometry.h"

// The data of a geometry is read through constant references (checked when this file is compiled)
static_assert(std::is_same_v<decltype(std::declval<const Mesh_Geometry&>().get_vertices()), const std::vector<Vertex>&>);
static_assert(std::is_same_v<decltype(std::declval<const Mesh_Geometry&>().get_indices()), const std::vector<GLuint>&>);

TEST(MeshGeometryTest, ExposesTheVerticesAndTheIndicesAsConstReferences)
{
	// the checks above are static, since creating a geometry needs an OpenGL context
	SUCCEED();
}
