/*
* AssimpModel.h
* This file defines the AssimpModel class (a derived class of Model),
* which is used to load an Assimp model from a file and draw it.
*/

#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../texture/Texture.h"
#include "../shader/Shader.h"
#include "Model.h"
#include "Mesh.h"

class AssimpModel : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that loads a model from a file
	AssimpModel(std::string const& path);

private:
	// Private Functions
	// -----------------
	// Loads a model with supported Assimp extensions from file and stores the resulting meshes in the meshes vector
	void loadAssimpModel(std::string const& path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node 
	// and repeats this process on its children nodes (if any)
	void processNode(aiNode* node, const aiScene* scene);

	// Processes a mesh and returns a mesh object
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	// Loads the material textures of a mesh
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType textureType);

};