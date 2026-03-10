/*
* Scene_Manager.h
* This file defines the Scene_Manager class, which is responsible for managing the scene,
* including the camera and other scene-related objects.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <glad/glad.h> // holds all OpenGL type declarations

// Forward declaration of classes to avoid cyclic includes and allow smart pointers
class Camera;
class Texture;

class Scene_Manager
{
public:
	// Constructor
	// -----------
	Scene_Manager() = default;

	// Destructor
	// ----------
	~Scene_Manager() = default;

	// Public Methods
	// --------------
	// Getters
	const std::shared_ptr<Camera>& get_camera() const { return camera; }
	std::shared_ptr<Texture>& get_skybox() { return skybox; }
	// Setters
	void set_camera(std::shared_ptr<Camera> camera) { this->camera = std::move(camera); }
	void set_skybox(std::shared_ptr<Texture> skybox) { this->skybox = std::move(skybox); }

	// Load skybox from 6 individual face file paths
	GLboolean load_skybox(const std::vector<std::string>& faces);
	// Load skybox from a single equirectangular HDR file path
	GLboolean load_skybox(const std::string& hdr_path);

	// Resets the camera to its default values
	void reset_camera();

	// Clears the skybox, i.e., removes the current skybox texture
	void clear_skybox();

private:
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Texture> skybox;

};