/*
* SceneManager.h
* This file defines the SceneManager class, which is responsible for managing the scene,
* including the camera and other scene-related objects.
*/


#pragma once

#include <memory> // for smart pointers

#include "AssetManager.h"
#include "../camera/Camera.h"

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
	// Setters
	void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }

private:
	std::shared_ptr<Camera> m_camera;

};