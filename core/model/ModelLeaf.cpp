/*
* ModelLeaf.cpp
* This file implements the ModelLeaf class (a derived class of ModelComponent),
* which represents an intentionally non-composite model node in the scene graph (semantic leaf).
*/

#include "ModelLeaf.h"

// Public Methods
// --------------
void ModelLeaf::Draw() const { for (const Mesh& mesh : meshes) mesh.Draw(); }
void ModelLeaf::Draw(const Shader& shader) const
{
	// need to cast away the constness of the shader to use it
	auto& nonConstShader = const_cast<Shader&>(shader);
	nonConstShader.Use(); // activate the shader program
	// set the appropiate world model matrix uniform per leaf model before drawing
	nonConstShader.SetMat4("model", GetWorldModelMatrix());

	for (const Mesh& mesh : meshes)
	{
		const_cast<Mesh&>(mesh).BindTextures(nonConstShader); // bind the textures
		mesh.Draw();
	}
}