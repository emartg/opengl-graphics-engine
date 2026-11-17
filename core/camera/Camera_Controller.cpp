/*
 * Camera_Controller.cpp
 * This file implements the Camera_Controller class, which moves a camera about a point (its pivot) as the view of an editor
 * does: orbit, pan, and dolly.
 */

#include "Camera_Controller.h"

#include "Camera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
	// Returns the matrix that rotates by an angle in degrees about an axis
	glm::mat3 rotation(GLfloat degrees, const glm::vec3& axis)
	{
		return glm::mat3(glm::rotate(glm::mat4{ 1.0f }, glm::radians(degrees), axis));
	}

	// Returns the distance from the camera to the pivot along its direction (the depth of the pivot in the view), at least the
	// given minimum (a pivot behind the camera or too close to it would give no usable distance)
	GLfloat depth_of(const Camera& camera, const glm::vec3& pivot, GLfloat minimum)
	{
		return std::max(glm::dot(pivot - camera.get_position(), camera.get_front()), minimum);
	}
} // namespace

// Public Static Methods
// ---------------------
void Camera_Controller::orbit(Camera& camera, const glm::vec3& pivot, GLfloat dx, GLfloat dy, const Camera_Control_Params& params)
{
	const glm::vec3 up_axis{ glm::normalize(camera.get_world_up()) };
	glm::vec3       offset{ camera.get_position() - pivot };
	glm::vec3       front{ camera.get_front() };

	// the yaw turns the camera about the vertical axis through the pivot; moving the mouse to the right turns the scene to the
	// right, so the camera turns the other way
	const glm::mat3 yaw_rotation{ rotation(-dx * params.orbit_degrees_per_pixel, up_axis) };
	offset = yaw_rotation * offset;
	front  = yaw_rotation * front;

	// the pitch turns the camera about its right axis through the pivot; moving the mouse down raises the camera and makes it
	// look down. The pitch stays within the limits of the camera, and the position turns by the same angle as the direction
	const GLfloat   pitch{ glm::degrees(std::asin(std::clamp(glm::dot(front, up_axis), -1.0f, 1.0f))) };
	const GLfloat   new_pitch{ std::clamp(pitch - dy * params.orbit_degrees_per_pixel, -89.0f, 89.0f) };
	const glm::vec3 right{ glm::normalize(glm::cross(front, up_axis)) };
	const glm::mat3 pitch_rotation{ rotation(new_pitch - pitch, right) };
	offset = pitch_rotation * offset;
	front  = pitch_rotation * front;

	camera.set_position(pivot + offset);
	camera.look_at(pivot + offset + front);
}

void Camera_Controller::pan(Camera& camera, const glm::vec3& pivot, GLfloat dx, GLfloat dy, GLuint viewport_height)
{
	if (viewport_height == 0)
		return; // a view without height (a minimized window) has no pixel size

	// the size of a pixel in world units at the depth of the pivot (at least at the depth of the near plane)
	const GLfloat depth{ depth_of(camera, pivot, camera.get_near_plane()) };
	const GLfloat pixel_size{ 2.0f * depth * std::tan(glm::radians(camera.get_zoom()) * 0.5f) / static_cast<GLfloat>(viewport_height) };

	// the point follows the mouse, so the camera moves the other way: to the left when the mouse moves to the right, and up when
	// the mouse moves down
	camera.set_position(camera.get_position() - camera.get_right() * (dx * pixel_size) + camera.get_up() * (dy * pixel_size));
}

void Camera_Controller::dolly(Camera& camera, const glm::vec3& pivot, GLfloat notches, const Camera_Control_Params& params)
{
	const GLfloat min_distance{ params.min_distance_in_near_planes * camera.get_near_plane() };
	const GLfloat max_distance{ std::max(min_distance, params.max_distance_in_far_planes * camera.get_far_plane()) };
	const GLfloat distance{ depth_of(camera, pivot, min_distance) };
	const GLfloat new_distance{ std::clamp(distance * std::pow(params.dolly_factor, notches), min_distance, max_distance) };

	camera.set_position(camera.get_position() + camera.get_front() * (distance - new_distance));
}
