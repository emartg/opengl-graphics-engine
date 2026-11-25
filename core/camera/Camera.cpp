/*
 * Camera.cpp
 * This file implements the Camera class (a derived class of Node),
 * which is used to process input and calculate the corresponding Euler Angles,
 * vectors, and Matrices for use in OpenGL.
 */

#include "Camera.h"

#include "../texture/Texture.h"

#include <cmath>

// Constructors
// ------------
Camera::Camera(const std::string& name, const glm::vec3 position, const glm::vec3 up, const GLfloat yaw, const GLfloat pitch) :
	Node("Camera", Node_Type::CAMERA, ALBEDO, POSITION, ROTATION, SCALE, FORWARD, FORWARD, Gizmo_Type::NONE, false, false),
	position{ position },
	front{ FRONT },
	world_up{ up },
	yaw{ yaw },
	pitch{ pitch },
	movement_speed{ SPEED },
	mouse_sensitivity{ SENSITIVITY },
	zoom{ ZOOM }
{
	// store the initial values for camera reset
	initial_position = position;
	initial_up       = up;
	initial_yaw      = yaw;
	initial_pitch    = pitch;

	// update the camera vectors based on the initial values
	recalculate_vectors();

	// a camera is moved with the mouse, never selected: the Scene Graph of an editor lists it greyed out and it is not outlined
	// (an application can still call set_is_selectable(true) on it); it is not moved like a node (the constructor of Node above
	// receives false), and the user cannot delete it, even if an application makes it selectable
	set_is_selectable(false);
	set_is_deletable(false);
}

Camera::Camera(
	const std::string& name,
	const GLfloat      pos_x,
	const GLfloat      pos_y,
	const GLfloat      pos_z,
	const GLfloat      up_x,
	const GLfloat      up_y,
	const GLfloat      up_z,
	const GLfloat      yaw,
	const GLfloat      pitch) :
	Camera(name, glm::vec3(pos_x, pos_y, pos_z), glm::vec3(up_x, up_y, up_z), yaw, pitch)
{}

// Public Methods
// --------------
glm::mat4 Camera::get_view_matrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::get_projection_matrix(GLfloat aspect_ratio) const
{
	return glm::perspective(glm::radians(zoom), aspect_ratio, near_plane, far_plane);
}

Ray Camera::get_ray(GLfloat x, GLfloat y, GLuint width, GLuint height) const
{
	// the point at the far plane under (x, y), seen from the position of the camera
	const glm::vec3 far_point{ unproject(x, y, 1.0f, width, height) };
	return Ray{ position, glm::normalize(far_point - position) };
}

glm::vec3 Camera::unproject(GLfloat x, GLfloat y, GLfloat depth, GLuint width, GLuint height) const
{
	// normalized device coordinates: x from -1 at the left edge to 1 at the right one, y from 1 at the top to -1 at the bottom (the
	// view is measured from its top-left corner), and z from -1 at the near plane to 1 at the far plane
	const GLfloat   w{ width == 0 ? 1.0f : static_cast<GLfloat>(width) };
	const GLfloat   h{ height == 0 ? 1.0f : static_cast<GLfloat>(height) };
	const glm::vec4 ndc{ 2.0f * x / w - 1.0f, 1.0f - 2.0f * y / h, 2.0f * depth - 1.0f, 1.0f };
	const glm::vec4 world{ glm::inverse(get_projection_matrix(width, height) * get_view_matrix()) * ndc };
	return glm::vec3{ world } / world.w;
}

glm::mat4 Camera::get_projection_matrix(GLuint width, GLuint height) const
{
	// a framebuffer of height 0 (a minimized window) has no aspect ratio, and a division by 0 would fill the matrix with infinities
	const GLfloat aspect_ratio{ height == 0 ? 1.0f : static_cast<GLfloat>(width) / static_cast<GLfloat>(height) };
	return get_projection_matrix(aspect_ratio);
}

void Camera::process_keyboard(Camera_Movement direction, GLfloat delta_time)
{
	GLfloat velocity = movement_speed * delta_time;
	switch (direction)
	{
		case Camera_Movement::FORWARD: position += front * velocity; break;
		case Camera_Movement::BACKWARD: position -= front * velocity; break;
		case Camera_Movement::LEFT: position -= right * velocity; break;
		case Camera_Movement::RIGHT: position += right * velocity; break;
	}
}

void Camera::process_mouse_translation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	position += right * xoffset;
	position += up * yoffset;
}

void Camera::process_mouse_rotation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity, GLboolean constrain_pitch)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrain_pitch)
		pitch = std::clamp(pitch, -89.0f, 89.0f);

	// update front, right and up Vectors using the updated Euler Angles
	recalculate_vectors();
}

void Camera::process_mouse_scroll(GLfloat yoffset, GLfloat sensitivity)
{
	zoom -= yoffset * sensitivity;
	// as the zoom value of the Camera class is used as the field of view (FOV) for the projection matrix,
	// we need to restrict it to a range that prevents the FOV from being too wide or too narrow,
	// preventing distortion or a flipped viewport
	zoom = std::clamp(zoom, 1.0f, 90.0f); // restrict zoom to a sensible range
}

void Camera::set_yaw(GLfloat yaw)
{
	this->yaw = yaw;
	recalculate_vectors();
}

void Camera::set_pitch(GLfloat pitch)
{
	this->pitch = std::clamp(pitch, -89.0f, 89.0f);
	recalculate_vectors();
}

bool Camera::set_clip_planes(GLfloat near_plane, GLfloat far_plane)
{
	if (!std::isfinite(near_plane) || !std::isfinite(far_plane) || near_plane <= 0.0f || far_plane <= near_plane)
	{
		std::cerr << "[WARNING::CAMERA::set_clip_planes] Invalid clip planes (near " << near_plane << ", far " << far_plane
				  << "): the near plane must be greater than 0 and the far plane greater than the near one; the planes are not changed"
				  << std::endl;
		return false;
	}
	this->near_plane = near_plane;
	this->far_plane  = far_plane;
	return true;
}

bool Camera::set_movement_speed(GLfloat speed)
{
	if (!std::isfinite(speed) || speed <= 0.0f)
	{
		std::cerr << "[WARNING::CAMERA::set_movement_speed] Invalid movement speed (" << speed
				  << "): it must be greater than 0; the speed is not changed" << std::endl;
		return false;
	}
	movement_speed = speed;
	return true;
}

bool Camera::apply_units(GLfloat units_per_metre)
{
	if (!std::isfinite(units_per_metre) || units_per_metre <= 0.0f)
	{
		std::cerr << "[WARNING::CAMERA::apply_units] Invalid number of units per metre (" << units_per_metre
				  << "): it must be greater than 0; the camera is not changed" << std::endl;
		return false;
	}

	// the position that Reset restores follows the unit only while it is the default position (in the previous unit): a position
	// given to the constructor belongs to the application and is kept
	if (initial_position == POSITION * this->units_per_metre)
		initial_position = POSITION * units_per_metre;

	near_plane            = NEAR_PLANE * units_per_metre;
	far_plane             = FAR_PLANE * units_per_metre;
	movement_speed        = SPEED * units_per_metre;
	this->units_per_metre = units_per_metre;
	return true;
}

void Camera::set_zoom(GLfloat zoom)
{
	// the same range as process_mouse_scroll, so that the field of view never distorts or flips the image
	this->zoom = std::clamp(zoom, 1.0f, 90.0f);
}

bool Camera::look_at(const glm::vec3& target)
{
	const glm::vec3 direction{ target - position };
	const GLfloat   length{ glm::length(direction) };
	if (!(length > 1e-6f)) // also false for a distance that is not a number
		return false;
	// the inverse of recalculate_vectors: front = (cos(yaw) cos(pitch), sin(pitch), sin(yaw) cos(pitch))
	yaw   = glm::degrees(std::atan2(direction.z, direction.x));
	pitch = std::clamp(glm::degrees(std::asin(std::clamp(direction.y / length, -1.0f, 1.0f))), -89.0f, 89.0f);
	recalculate_vectors();
	return true;
}

void Camera::Reset()
{
	position          = initial_position;
	front             = FRONT;
	world_up          = initial_up;
	yaw               = initial_yaw;
	pitch             = initial_pitch;
	mouse_sensitivity = SENSITIVITY;
	zoom              = ZOOM;
	recalculate_vectors();
}

// Private Methods
// ---------------
void Camera::recalculate_vectors()
{
	// calculate the new front vector
	glm::vec3 front;
	front.x     = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y     = sin(glm::radians(pitch));
	front.z     = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	// also re-calculate the right and up vector
	// (normalizing the vectors, because their length gets closer to 0
	// the more you look up or down which results in slower movement)
	right = glm::normalize(glm::cross(this->front, world_up));
	up    = glm::normalize(glm::cross(right, this->front));
}
