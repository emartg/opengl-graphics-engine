/*
 * Scene_Manager.cpp
 * This file implements the Scene_Manager class, which is responsible for managing the scene,
 * including the camera and other scene-related objects.
 */

#include "Scene_Manager.h"

#include "Node_Manager.h"
#include "../camera/Camera.h"
#include "../texture/Texture.h"

// Public Methods
// --------------
GLboolean Scene_Manager::load_skybox(const std::vector<std::string>& faces)
{
	// ensure 6 faces are provided
	if (faces.size() != 6)
	{ // if not, print an error and return false
		std::cerr << "[ERROR::SCENEMANAGER::load_skybox] Skybox requires 6 face paths, "
				  << "but " << faces.size() << " were provided" << std::endl;
		return GL_FALSE;
	}

	// create a new Texture object for the skybox
	skybox = std::make_shared<Texture>("skybox", faces);
	if (skybox->get_texture_id() == 0)
	{ // if the texture id is 0, loading failed - print an error, reset the skybox pointer, and return false
		std::cerr << "[ERROR::SCENEMANAGER::load_skybox] Failed to load skybox texture" << std::endl;
		skybox = nullptr; // reset the skybox pointer
		return GL_FALSE;
	}

	// if the skybox is loaded successfully, print a success message and return true
	std::cout << "[SUCCESS::SCENEMANAGER::load_skybox] Skybox loaded successfully" << std::endl;
	return GL_TRUE;
}

GLboolean Scene_Manager::load_skybox(const std::string& hdr_path)
{
	// create a new Texture object for the skybox from the HDR equirectangular image
	skybox = std::make_shared<Texture>("skybox", hdr_path, true);
	if (skybox->get_texture_id() == 0)
	{ // if the texture id is 0, loading failed - print an error, reset the skybox pointer, and return falseX
		std::cerr << "[ERROR::SCENEMANAGER::load_skybox] Failed to load HDR skybox texture from: " << hdr_path << std::endl;
		skybox = nullptr; // reset the skybox pointer
		return GL_FALSE;
	}

	// if the skybox is loaded successfully, print a success message and return true
	std::cout << "[SUCCESS::SCENEMANAGER::load_skybox] HDR skybox loaded successfully" << std::endl;
	return GL_TRUE;
}

void Scene_Manager::reset_camera()
{
	if (camera)
	{ // if a camera is set, reset it to its default values
		camera->Reset();
		std::cout << "[INFO::SCENEMANAGER::reset_camera] Camera reset to default values" << std::endl;
	}
	else
	{ // if no camera is set, print an info message
		std::cout << "[INFO::SCENEMANAGER::reset_camera] No camera to reset" << std::endl;
	}
}

void Scene_Manager::clear_skybox()
{
	if (skybox)
	{ // if a skybox is set, deallocate its resources and reset the pointer
		skybox->deallocate_resources();
		skybox = nullptr;
		std::cout << "[INFO::SCENEMANAGER::clear_skybox] Skybox cleared" << std::endl;
	}
	else
	{ // if no skybox is set, print an info message
		std::cout << "[INFO::SCENEMANAGER::clear_skybox] No skybox to clear" << std::endl;
	}
}
