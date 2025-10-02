/*
* SceneManager.h
* This file defines the SceneManager class, which is responsible for managing the scene,
* including the camera and other scene-related objects.
*/


#pragma once

#include <memory> // for smart pointers

#include "AssetManager.h"
#include "../camera/Camera.h"
#include "../texture/Texture.h"

class SceneManager
{
public:
	// Constructor
	// -----------
	SceneManager() = default;

	// Destructor
	// ----------
	~SceneManager() = default;

	// Public Methods
	// --------------
	// Getters
	const std::shared_ptr<Camera>& GetCamera() const { return m_camera; }
	std::shared_ptr<Texture>& GetSkybox() { return m_skybox; }
	// Setters
	void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
	void SetSkybox(std::shared_ptr<Texture> skybox) { m_skybox = std::move(skybox); }

	// Load skybox from 6 individual face file paths
	GLboolean LoadSkybox(const std::vector<std::string>& faces);

private:
	std::shared_ptr<Camera> m_camera;
	std::shared_ptr<Texture> m_skybox;

};