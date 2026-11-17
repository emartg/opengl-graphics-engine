/*
 * Camera_Controller.h
 * This file defines the Camera_Controller class, which moves a camera about a point (its pivot) as the view of an editor does:
 * it orbits the camera about the pivot, pans it in the plane of the view, and dollies it along its direction. It needs no
 * OpenGL context, so its geometry is unit tested; the input manager calls it with the movements of the mouse.
 */

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

class Camera;

// Settings of the mouse controls of the camera (their speeds and limits)
struct Camera_Control_Params
{
	GLfloat orbit_degrees_per_pixel{ 0.25f };    // rotation of the orbit for each pixel that the mouse moves
	GLfloat dolly_factor{ 0.85f };               // the distance to the pivot is multiplied by it for each notch of the wheel forward
	GLfloat min_distance_in_near_planes{ 5.0f }; // the dolly stops at this many times the near plane from the pivot
	GLfloat max_distance_in_far_planes{ 0.5f };  // and at this fraction of the far plane, so the limits follow the clip planes
};

class Camera_Controller
{
public:
	// Public Static Methods
	// ---------------------
	// Orbits the camera about the pivot by the movement of the mouse in pixels (to the right and down are positive). The turn is
	// rigid: the position of the camera turns about the pivot by the same rotation as its direction, so the pivot stays where it
	// is in the view. Moving the mouse to the right turns the scene to the right (a yaw about the vertical axis of the world), and
	// moving it down raises the camera and makes it look down (a pitch about the right axis of the camera, limited to the range
	// from -89 to 89 degrees of the camera)
	static void orbit(Camera& camera, const glm::vec3& pivot, GLfloat dx, GLfloat dy, const Camera_Control_Params& params);

	// Pans the camera along its right and up vectors by the movement of the mouse in pixels (to the right and down are positive),
	// so that a point at the depth of the pivot follows the mouse; the height of the view in pixels gives the size of a pixel at
	// that depth (2 times the depth times the tangent of half the vertical field of view, divided by the height)
	static void pan(Camera& camera, const glm::vec3& pivot, GLfloat dx, GLfloat dy, GLuint viewport_height);

	// Dollies the camera along its direction: the distance to the pivot along the view is multiplied by the dolly factor for each
	// notch of the wheel forward (positive) and divided by it for each notch backward, between the limits of the parameters. The
	// field of view does not change
	static void dolly(Camera& camera, const glm::vec3& pivot, GLfloat notches, const Camera_Control_Params& params);
};
