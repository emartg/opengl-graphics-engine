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
	std::shared_ptr<Texture>&      get_skybox() { return skybox; }
	// Returns the number of world units in a metre of the scene (1 by default: a scene in metres)
	GLfloat get_units_per_metre() const { return units_per_metre; }
	// Returns a length given in metres in the units of the scene (for example the range of a light that a descriptor gives in metres)
	GLfloat to_scene_units(GLfloat metres) const { return metres * units_per_metre; }
	// Returns a length given in the units of the scene in metres
	GLfloat to_metres(GLfloat scene_units) const { return scene_units / units_per_metre; }
	// Setters
	// Sets the camera of the scene. It keeps its own values: a camera set after set_units_per_metre gets the unit of the scene with
	// camera->apply_units(get_units_per_metre())
	void set_camera(std::shared_ptr<Camera> camera) { this->camera = std::move(camera); }
	void set_skybox(std::shared_ptr<Texture> skybox) { this->skybox = std::move(skybox); }
	// Sets the number of world units in a metre of the scene (for example 100 for a scene in centimetres), and returns true. Nothing
	// in the scene is moved or resized: the positions and sizes that the application gives are in its unit already. The camera of the
	// scene, if there is one, takes the defaults of that unit (Camera::apply_units: its clip planes, its speed, and the default
	// position that Reset restores; never its current position), so the application sets the unit first and its own overrides
	// afterwards. Returns false and keeps the current unit when the number is not finite or not greater than 0
	bool set_units_per_metre(GLfloat units_per_metre);

	// Load skybox from 6 individual face file paths
	GLboolean load_skybox(const std::vector<std::string>& faces);
	// Load skybox from a single equirectangular HDR file path
	GLboolean load_skybox(const std::string& hdr_path);

	// Resets the camera to its default values
	void reset_camera();

	// Clears the skybox, i.e., removes the current skybox texture
	void clear_skybox();

private:
	std::shared_ptr<Camera>  camera;
	std::shared_ptr<Texture> skybox;
	GLfloat                  units_per_metre{ 1.0f }; // world units in a metre of the scene
};
