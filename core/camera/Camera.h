/*
* Camera.h
* This file defines the Camera class (a derived class of Asset),
* which is used to process input and calculate the corresponding Euler Angles,
* vectors, and Matrices for use in OpenGL.
*/

#pragma once

#include <algorithm>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Asset.h"

// Enumeration that defines several possible options for camera movement. 
// Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera : public Asset
{
public:
	// Constructors
	// ------------
	// Constructor with vectors
	Camera(const std::string& name,
		   const glm::vec3 position = POSITION, const glm::vec3 up = UP,
		   const GLfloat yaw = YAW, const GLfloat pitch = PITCH);
	// Constructor with scalar values
	Camera(const std::string& name,
		   const GLfloat posX, const GLfloat posY, const GLfloat posZ,
		   const GLfloat upX, const GLfloat upY, const GLfloat upZ,
		   const GLfloat yaw, const GLfloat pitch);

	// Public Methods
	// --------------
	// Loads the camera
	void Load() override {}

	// Deallocates all the resources of the camera
	void DeallocateResources() override {}

	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetFront() const { return front; }
	glm::vec3 GetUp() const { return up; }
	glm::vec3 GetRight() const { return right; }
	glm::vec3 GetWorldUp() const { return worldUp; }
	GLfloat GetYaw() const { return yaw; }
	GLfloat GetPitch() const { return pitch; }
	GLfloat GetMovementSpeed() const { return movementSpeed; }
	GLfloat GetMouseSensitivity() const { return mouseSensitivity; }
	GLfloat GetZoom() const { return zoom; }

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() const;

	// Processes input received from any keyboard-like input system.
	// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);
	// Processes input received from a left-click drag event.
	// Expects the offset values in both the x and y directions and the sensitivity of the mouse movement.
	// This method is intended to be used for camera translation
	void ProcessMouseTranslation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity = 0.1f);
	// Processes input received from a mouse right-click drag event.
	// Expects the offset values in both the x and y directions, the sensitivity of the mouse movement, 
	// and whether the user wants to constrain the pitch. 
	// This method is intended to be used for camera rotation
	void ProcessMouseRotation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity = 0.1f, GLboolean constrainPitch = true);
	// Processes input received from a mouse scroll-wheel event.
	// Expects the offset value in the y direction and the sensitivity of the scroll
	void ProcessMouseScroll(GLfloat yoffset, GLfloat sensitivity = 1.0f);

private:
	// Private Attributes
	// ------------------
	// Camera attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	// Euler angles
	GLfloat yaw;
	GLfloat pitch;
	// Camera options
	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// Static Constants
	// ----------------
	// Default camera values
	static constexpr glm::vec3 POSITION = glm::vec3(4.25f, 2.5f, 4.25f);
	static constexpr glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr GLfloat YAW = -135.0f;
	static constexpr GLfloat PITCH = -24.0f;
	static constexpr GLfloat SPEED = 2.5f;
	static constexpr GLfloat SENSITIVITY = 0.1f;
	static constexpr GLfloat ZOOM = 45.0f;

	// Private Methods
	// ---------------
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();

};