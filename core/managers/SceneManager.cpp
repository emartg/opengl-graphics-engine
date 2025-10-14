/*
* SceneManager.cpp
* This file implements the SceneManager class, which is responsible for managing the scene,
* including the camera and other scene-related objects.
*/

#include "SceneManager.h"

// Public Methods
// --------------
GLboolean SceneManager::LoadSkybox(const std::vector<std::string>& faces)
{
	// ensure 6 faces are provided
	if (faces.size() != 6)
	{ // if not, print an error and return false
		std::cerr << "[ERROR::SCENEMANAGER::LoadSkybox] Skybox requires 6 face paths, "
			<< "but " << faces.size() << " were provided." << std::endl;
		return GL_FALSE;
	}

	// create a new Texture object for the skybox
	m_skybox = std::make_shared<Texture>("skybox", faces);
	if (m_skybox->GetTextureId() == 0)
	{ // if the texture ID is 0, loading failed - print an error, reset the skybox pointer, and return false
		std::cerr << "[ERROR::SCENEMANAGER::LoadSkybox] Failed to load skybox texture." << std::endl;
		m_skybox = nullptr; // reset the skybox pointer
		return GL_FALSE;
	}

	// if the skybox is loaded successfully, print a success message and return true
	std::cout << "[SUCCESS::SCENEMANAGER::LoadSkybox] Skybox loaded successfully." << std::endl;
	return GL_TRUE;
}

void SceneManager::ResetCamera()
{
	if (m_camera)
	{ // if a camera is set, reset it to its default values
		m_camera->Reset();
		std::cout << "[INFO::SCENEMANAGER::ResetCamera] Camera reset to default values" << std::endl;
	}
	else
	{ // if no camera is set, print an info message
		std::cout << "[INFO::SCENEMANAGER::ResetCamera] No camera to reset" << std::endl;
	}
}

void SceneManager::ClearSkybox()
{
	if (m_skybox)
	{ // if a skybox is set, deallocate its resources and reset the pointer
		m_skybox->DeallocateResources();
		m_skybox = nullptr;
		std::cout << "[INFO::SCENEMANAGER::ClearSkybox] Skybox cleared" << std::endl;
	}
	else
	{ // if no skybox is set, print an info message
		std::cout << "[INFO::SCENEMANAGER::ClearSkybox] No skybox to clear" << std::endl;
	}
}