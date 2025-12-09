/*
* SceneManager.h
* This file defines the SceneManager class, which is responsible for managing the scene,
* including the camera and other scene-related objects.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations

// Forward declaration of classes to avoid cyclic includes and allow smart pointers
class Camera;
class Texture;

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
	// Load skybox from a single equirectangular HDR file path
	GLboolean LoadSkybox(const std::string& hdrPath);

	// Resets the camera to its default values
	void ResetCamera();

	// Clears the skybox, i.e., removes the current skybox texture
	void ClearSkybox();

private:
	std::shared_ptr<Camera> m_camera;
	std::shared_ptr<Texture> m_skybox;

};