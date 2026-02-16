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
Model::Model(const std::string& name, const NodeType type,
			 const glm::vec4 albedo, const glm::vec3 position,
			 const glm::quat rotation, const glm::vec3 scale,
			 const glm::vec3 forward, const glm::vec3 meshForward,
			 const GizmoType gizmoType)
	: Node(name, type, albedo, position, rotation, scale, forward, meshForward, gizmoType)
{}

// Public Methods
// --------------
void Model::Draw() const
{
	for (const auto& mesh : meshes)
		if (mesh) { mesh->Draw(); }
}

void Model::Draw(const Shader& shader) const
{
	for (const auto& mesh : meshes)
	{
		if (mesh)
		{
			mesh->BindTextures(const_cast<Shader&>(shader)); // bind the textures
			mesh->Draw();
		}
	}
}