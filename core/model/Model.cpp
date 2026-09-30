/*
 * Model.h
 * This file implements the Model class (a derived class of Node),
 * which is used as a base class for all mesh-bearing nodes in the scene graph,
 * i.e., models that contain meshes to be drawn.
 */

#include "Model.h"

#include "mesh/Mesh.h"
#include "../shader/Shader.h"

// Constructors
// ------------
Model::Model(
    const std::string& name,
    const Node_Type    type,
    const glm::vec4    albedo,
    const glm::vec3    position,
    const glm::quat    rotation,
    const glm::vec3    scale,
    const glm::vec3    forward,
    const glm::vec3    mesh_forward,
    const Gizmo_Type   gizmo_type) :
    Node(name, type, albedo, position, rotation, scale, forward, mesh_forward, gizmo_type)
{}

// Public Methods
// --------------
void Model::draw() const
{
	for (const auto& mesh : meshes)
		if (mesh)
		{
			mesh->draw();
		}
}

void Model::draw(const Shader& shader) const
{
	for (const auto& mesh : meshes)
	{
		if (mesh)
		{
			mesh->bind_textures(const_cast<Shader&>(shader)); // bind the textures
			mesh->draw();
		}
	}
}