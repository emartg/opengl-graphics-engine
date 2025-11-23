/*
* ModelComposite.cpp
* This file implements the ModelComposite class (a derived class of ModelComponent),
* which represents a composite model in the scene graph. It can bear children.
*/

#include "ModelComposite.h"

// Public Methods
// --------------
void ModelComposite::Draw() const { for (const Mesh& mesh : meshes) mesh.Draw(); }
void ModelComposite::Draw(const Shader& shader) const
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

void ModelComposite::AddChild(const std::shared_ptr<ModelComponent>& child)
{
	if (!child || child.get() == this) return; // prevent adding null or self as child
	// avoid adding duplicate children
	if (std::find(children.begin(), children.end(), child) != children.end()) return;
	children.push_back(child); // add the child to the children vector
	// shared_from_this() comes from ModelComponent (enable_shared_from_this<ModelComponent>)
	child->SetParent(std::static_pointer_cast<ModelComponent>(shared_from_this())); // set this as parent
}
void ModelComposite::RemoveChild(const std::shared_ptr<ModelComponent>& child)
{
	if (!child) return; // prevent removing null child
	// remove the child from the children vector
	children.erase(std::remove(children.begin(), children.end(), child), children.end());
	child->SetParent(nullptr); // reset the parent of the child to nullptr
}