/*
* AssimpModel.cpp
* This file implements the AssimpModel class (a derived class of Model),
* which is used to load and draw 3D models from files using the Assimp library.
*/

#include "AssimpModel.h"

#include "mesh/Mesh.h"
#include "../Node.h"
#include "../shader/Shader.h"
#include "../texture/Texture.h"

// Static Protected Attributes
// ---------------------------
GLuint AssimpModel::nAssimpModels{}; // initialize the number of Assimp models in the scene to 0

// Constructors
// ------------
// Constructor that loads a model from a file
AssimpModel::AssimpModel(const std::string& name, const std::string& path,
						 const glm::vec4 albedo, const glm::vec3 position,
						 const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, NodeType::ASSIMP_MODEL, // set the model type to ASSIMP_MODEL
			albedo, position, rotation, scale, forward, meshForward)
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
	const aiScene* scene =
		importer.ReadFile(path,
						  aiProcess_Triangulate |
						  aiProcess_FlipUVs |
						  aiProcess_GenNormals |
						  aiProcess_PreTransformVertices // bake node transformations into vertices
		);

	// check for errors in the importing process
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "[ERROR::ASSIMPMODEL::loadAssimpModel] "
			<< "Assimp failed to load model from:\n\t" << path
			<< "\n\tImporter error: " << importer.GetErrorString() << std::endl;
		return;
	}

	// store the directory of the model file, handling both '/' and '\'
	// as separators for cross-platform compatibility
	size_t slashFwd = path.find_last_of('/');
	size_t slashBwd = path.find_last_of('\\');
	size_t lastSlash = std::string::npos;

	if (slashFwd != std::string::npos && slashBwd != std::string::npos)
		lastSlash = std::max(slashFwd, slashBwd);
	else
		lastSlash = (slashFwd != std::string::npos) ? slashFwd : slashBwd;

	if (lastSlash != std::string::npos)
	{ // if a slash was found, set the directory accordingly
		// preserve the same separator as in the original path string
		char separator = path[lastSlash]; // either '/' or '\'
		directory = path.substr(0, lastSlash + 1); // include the slash
	}
	else
	{
		// if no slash was found, clear the directory 
		// so that textures are loaded from the current working directory
		directory.clear();
	}

	// process the root node (recursively process all of its children)
	processNode(scene->mRootNode, scene);

	// calculate the bounding box of the model based on the vertices of the meshes
	calculateBoundingBox();

	std::cout << "[SUCCESS::ASSIMPMODEL::loadAssimpModel] Model loaded successfully from:\n\t"
		<< path << std::endl;
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

std::shared_ptr<Mesh> AssimpModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// vector to store vertices, indices and textures
	std::vector<Vertex> vertices; // each vertex contains position, normal and texture coordinates
	std::vector<GLuint> indices; // each index corresponds to a vertex in the vertices vector
	// each texture corresponds to a material texture of the mesh
	std::vector<std::shared_ptr<Texture>> textures;

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

	// process material (load all supported types of texture maps)
	if (mesh->mMaterialIndex >= 0)
	{
		// retrieve the material of the mesh
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// lambda function to load and append textures of a specific type
		auto append = [&](aiTextureType aiType, TextureType textureType)
		{
			std::vector<std::shared_ptr<Texture>> maps =
				loadMaterialTextures(material, aiType, textureType);
			textures.insert(textures.end(), maps.begin(), maps.end());
		};

		// load different types of textures and append them to the textures vector
		// (these are the most common types, more can be added if needed)
		append(aiTextureType_DIFFUSE, TextureType::DIFFUSE);
		append(aiTextureType_SPECULAR, TextureType::SPECULAR);
		append(aiTextureType_NORMALS, TextureType::NORMAL);
		append(aiTextureType_HEIGHT, TextureType::HEIGHT);
		append(aiTextureType_AMBIENT, TextureType::AMBIENT);
		append(aiTextureType_EMISSIVE, TextureType::EMISSIVE);
		append(aiTextureType_LIGHTMAP, TextureType::LIGHTMAP);
		append(aiTextureType_DIFFUSE_ROUGHNESS, TextureType::ROUGHNESS);
		append(aiTextureType_METALNESS, TextureType::METALNESS);
		append(aiTextureType_DISPLACEMENT, TextureType::DISPLACEMENT);
		append(aiTextureType_OPACITY, TextureType::OPACITY);
		append(aiTextureType_REFLECTION, TextureType::REFLECTION);
		append(aiTextureType_BASE_COLOR, TextureType::DIFFUSE);
		// unknown texture type (fallback)
		append(aiTextureType_UNKNOWN, TextureType::UNDEFINED);
	}

	// return a mesh object created from the extracted mesh data
	return std::make_shared<Mesh>(vertices, indices, textures);
}

std::vector<std::shared_ptr<Texture>> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type,
																		TextureType textureType)
{
	// vector to store the loaded textures of the specified type from the material,
	// avoiding duplicates
	std::vector<std::shared_ptr<Texture>> loadedTextures;

	// iterate over all textures of the specified type in the material
	// and add them to the textures vector if they haven't been loaded before
	for (GLuint i{}; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// check for embedded textures (those starting with '*'), which are not yet supported,
		// hence why they are not loaded and a warning is issued
		const char* pathCStr = str.C_Str();
		if (pathCStr && pathCStr[0] == '*')
		{
			// embedded texture, warn and skip loading
			std::cerr << "[WARNING::ASSIMPMODEL::loadMaterialTextures] "
				"Embedded textures not supported, skipping texture:\n"
				<< pathCStr << " of type " << Texture::TextureTypeToString(textureType) << std::endl;
			continue;
		}

		std::string texturePath = directory + str.C_Str(); // construct the full path to the texture file

		// check if the texture was loaded before and if so, continue to the next iteration
		GLboolean skip{ false };
		for (GLuint j{}; j < loadedTextures.size(); j++)
		{
			if (std::strcmp(loadedTextures[j]->GetPath().data(), texturePath.c_str()) == 0)
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
			auto texture = std::make_shared<Texture>(str.C_Str(), texturePath, textureType);
			loadedTextures.push_back(texture); // add the texture to the loaded textures vector
		}
	}

	return loadedTextures;
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
		for (const auto& vertex : mesh->GetVertices())
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