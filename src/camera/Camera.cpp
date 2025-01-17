/* Camera.cpp
This file implements the Camera class, which is used to process input and calculate
the corresponding Euler Angles, Vectors, and Matrices for use in OpenGL */

#include "Camera.h"

// Constructors
// ------------
// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch)
	: front(glm::vec3(0.0f, 0.0f, -1.0f)), position(position), worldUp(up), yaw(yaw), pitch(pitch),
	movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
{
	updateCameraVectors();
}

// Constructor with scalar values
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch)
	: Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch) {
}

// Public Methods
// --------------
// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

// Processes input received from any keyboard-like input system. 
// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
	GLfloat velocity = movementSpeed * deltaTime;
	switch (direction)
	{
	case FORWARD:
		position += front * velocity;
		break;
	case BACKWARD:
		position -= front * velocity;
		break;
	case LEFT:
		position -= right * velocity;
		break;
	case RIGHT:
		position += right * velocity;
		break;
	}
	// position.y = 0.0f;  // make sure the user stays at the ground level (y = 0, XZ plane)
}

// Processes input received from a left-click drag event.
// Expects the offset values in both the x and y directions and the sensitivity of the mouse movement.
// This method is intended to be used for camera translation
void Camera::ProcessMouseTranslation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	position += right * xoffset;
	position += up * yoffset;
}

// Processes input received from a mouse right-click drag event.
// Expects the offset values in both the x and y directions, the sensitivity of the mouse movement, 
// and whether the user wants to constrain the pitch. 
// This method is intended to be used for camera rotation
void Camera::ProcessMouseRotation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity, GLboolean constrainPitch)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
		pitch = std::clamp(pitch, -89.0f, 89.0f);

	// update front, right and up Vectors using the updated Euler Angles
	updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. 
// Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(GLfloat yoffset, GLfloat sensitivity)
{
	zoom -= yoffset * sensitivity;
	zoom = std::clamp(zoom, 1.0f, 45.0f);
}

// Private Methods
// ---------------
// Utility function for updating the camera's front vector.
// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
	// calculate the new front vector
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	// also re-calculate the right and up vector
	right = glm::normalize(glm::cross(this->front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, this->front));
}