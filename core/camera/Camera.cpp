/*
* Camera.cpp
* This file implements the Camera class (a derived class of Node),
* which is used to process input and calculate the corresponding Euler Angles,
* vectors, and Matrices for use in OpenGL.
*/

#include "Camera.h"

#include "../texture/Texture.h"

// Constructors
// ------------
Camera::Camera(const std::string& name,
			   const glm::vec3 position, const glm::vec3 up,
			   const GLfloat yaw, const GLfloat pitch)
	: Node("Camera", NodeType::CAMERA, ALBEDO, POSITION, ROTATION, SCALE, FORWARD, FORWARD,
		   GizmoType::NONE, false, false),
	position{ position }, front{ FRONT }, worldUp{ up }, yaw{ yaw }, pitch{ pitch },
	movementSpeed{ SPEED }, mouseSensitivity{ SENSITIVITY }, zoom{ ZOOM }
{
	// store the initial values for camera reset
	initialPosition = position;
	initialUp = up;
	initialYaw = yaw;
	initialPitch = pitch;

	// update the camera vectors based on the initial values
	recalculateVectors();
}

Camera::Camera(const std::string& name,
			   const  GLfloat posX, const GLfloat posY, const GLfloat posZ,
			   const GLfloat upX, const GLfloat upY, const GLfloat upZ,
			   const GLfloat yaw, const GLfloat pitch)
	: Camera(name, glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch)
{}

// Public Methods
// --------------
glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
	GLfloat velocity = movementSpeed * deltaTime;
	switch (direction)
	{
		case Camera_Movement::FORWARD:
			position += front * velocity;
			break;
		case Camera_Movement::BACKWARD:
			position -= front * velocity;
			break;
		case Camera_Movement::LEFT:
			position -= right * velocity;
			break;
		case Camera_Movement::RIGHT:
			position += right * velocity;
			break;
	}
	// position.y = 0.0f;  // make sure the user stays at the ground level (y = 0, XZ plane)
}

void Camera::ProcessMouseTranslation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	position += right * xoffset;
	position += up * yoffset;
}

void Camera::ProcessMouseRotation(GLfloat xoffset, GLfloat yoffset,
								  GLfloat sensitivity, GLboolean constrainPitch)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
		pitch = std::clamp(pitch, -89.0f, 89.0f);

	// update front, right and up Vectors using the updated Euler Angles
	recalculateVectors();
}

void Camera::ProcessMouseScroll(GLfloat yoffset, GLfloat sensitivity)
{
	zoom -= yoffset * sensitivity;
	// as the zoom value of the Camera class is used as the field of view (FOV) for the projection matrix,
	// we need to restrict it to a range that prevents the FOV from being too wide or too narrow, 
	// preventing distortion or a flipped viewport
	zoom = std::clamp(zoom, 1.0f, 90.0f); // restrict zoom to a sensible range
}

void Camera::Reset()
{
	position = initialPosition;
	front = FRONT;
	worldUp = initialUp;
	yaw = initialYaw;
	pitch = initialPitch;
	movementSpeed = SPEED;
	mouseSensitivity = SENSITIVITY;
	zoom = ZOOM;
	recalculateVectors();
}

// Private Methods
// ---------------
void Camera::recalculateVectors()
{
	// calculate the new front vector
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	// also re-calculate the right and up vector
	// (normalizing the vectors, because their length gets closer to 0 
	// the more you look up or down which results in slower movement)
	right = glm::normalize(glm::cross(this->front, worldUp));
	up = glm::normalize(glm::cross(right, this->front));
}