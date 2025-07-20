/*
* AssimpModel.cpp
* This file implements the AssimpModel class (a derived class of Model),
* which is used to load an Assimp model from a file and draw it.
*/

#include "AssimpModel.h"

// Static Protected Attributes
// ---------------------------
GLuint AssimpModel::nAssimpModels{}; // initialize the number of Assimp models in the scene to 0

// Constructors
// ------------
// Constructor that loads a model from a file
AssimpModel::AssimpModel(const std::string& name,
						 const std::string& path,
						 const glm::vec3 albedo,
						 const glm::vec3 position, const glm::quat rotation,
						 const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, albedo, position, rotation, scale, forward, meshForward,
			ModelType::ASSIMP_MODEL) // set the model type to ASSIMP_MODEL
{
	loadAssimpModel(path); // load the model from the specified path
	nAssimpModels++;
}

// Private Methods
// ---------------
void AssimpModel::loadAssimpModel(std::string const& path)
{
	Assimp::Importer importer;

	// read file via Assimp (the second argument of ReadFile is a combination of post-processing options)
	const aiScene* scene = importer.ReadFile(path,
											 aiProcess_Triangulate |
											 aiProcess_FlipUVs |
											 aiProcess_GenNormals);

	// check for errors in the importing process
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::Assimp::" << importer.GetErrorString() << std::endl;
		return;
	}
	// store the directory of the model file
	directory = path.substr(0, path.find_last_of('/')) + '/';

	// process the root node (recursively process all of its children)
	processNode(scene->mRootNode, scene);

	// calculate the bounding box of the model based on the vertices of the meshes
	calculateBoundingBox();

	std::cout << "Assimp model loaded successfully at path: " << path << std::endl;
}

void AssimpModel::processNode(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (GLuint i{}; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(processMesh(mesh, scene));
	}

	// recursively process each of the children's nodes (if any)
	for (GLuint i{}; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);

}

Mesh AssimpModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// vector to store vertices, indices and textures
	std::vector<Vertex> vertices; // each vertex contains position, normal and texture coordinates
	std::vector<GLuint> indices; // each index corresponds to a vertex in the vertices vector
	std::vector<Texture> textures; // each texture corresponds to a material texture of the mesh

	// process vertices
	for (GLuint i{}; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates (if available)
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else // if the mesh doesn't contain texture coordinates, set them to (0, 0)
		{
			vertex.TexCoords = glm::vec2(0.0f);
		}

		// add the vertex to the vertices vector
		vertices.push_back(vertex);
	}

	// process indices
	for (GLuint i{}; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j{}; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process material
	if (mesh->mMaterialIndex >= 0) // does the mesh contain material data?
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// load diffuse maps and add them to the textures vector
		std::vector<Texture> diffuseMaps = loadMaterialTextures(
			material, aiTextureType_DIFFUSE, TextureType::DIFFUSE
		);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// load specular maps and add them to the textures vector
		std::vector<Texture> specularMaps = loadMaterialTextures(
			material, aiTextureType_SPECULAR, TextureType::SPECULAR
		);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

std::vector<Texture> AssimpModel::loadMaterialTextures(aiMaterial* mat,
													   aiTextureType type,
													   TextureType textureType)
{
	// a vector to store already loaded textures to avoid loading the same texture multiple times
	std::vector<Texture> textures;

	// iterate over all textures of the specified type in the material
	// and add them to the textures vector if they haven't been loaded before
	for (GLuint i{}; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		std::string texturePath = directory + str.C_Str(); // construct the full path to the texture file

		// check if the texture was loaded before and if so, continue to the next iteration
		GLboolean skip{ false };
		for (GLuint j{}; j < loadedTextures.size(); j++)
		{
			if (std::strcmp(loadedTextures[j].GetPath().data(), texturePath.c_str()) == 0)
			{
				// if the texture has already been loaded, add it to the textures vector
				textures.push_back(loadedTextures[j]);
				skip = true; // a texture with the same filepath already loaded, so no need to load it again
				break;
			}
		}

		// if the texture hasn't been loaded already, load it
		if (!skip)
		{
			Texture texture{ str.C_Str(), texturePath.c_str(), textureType };
			textures.push_back(texture);
			// to ensure we won't load the same texture again, store it in the loaded textures
			loadedTextures.push_back(texture);
		}
	}

	return textures;
}

void AssimpModel::calculateBoundingBox()
{
	if (meshes.empty()) return; // if there are no meshes, return early

	// initialize the bounding box minimum and maximum points to extreme values
	m_boundingBoxMin = glm::vec3(std::numeric_limits<float>::max());
	m_boundingBoxMax = glm::vec3(std::numeric_limits<float>::min());

	// iterate over all meshes and their vertices to calculate the bounding box values
	for (const auto& mesh : meshes)
	{
		for (const auto& vertex : mesh.GetVertices())
		{
			m_boundingBoxMin.x = std::min(m_boundingBoxMin.x, vertex.Position.x);
			m_boundingBoxMin.y = std::min(m_boundingBoxMin.y, vertex.Position.y);
			m_boundingBoxMin.z = std::min(m_boundingBoxMin.z, vertex.Position.z);

			m_boundingBoxMax.x = std::max(m_boundingBoxMax.x, vertex.Position.x);
			m_boundingBoxMax.y = std::max(m_boundingBoxMax.y, vertex.Position.y);
			m_boundingBoxMax.z = std::max(m_boundingBoxMax.z, vertex.Position.z);
		}
	}
}