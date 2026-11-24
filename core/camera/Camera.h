/*
 * Camera.h
 * This file defines the Camera class (a derived class of Node),
 * which is used to process input and calculate the corresponding Euler Angles,
 * vectors, and Matrices for use in OpenGL.
 */

#pragma once

#include "../Node.h"

#include <iostream>
#include <algorithm>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Enumeration that defines several possible options for camera movement.
// Used as abstraction to stay away from window-system specific input methods
enum class Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera : public Node
{
public:
	// Public Constants
	// ----------------
	// default distances of the near and far planes of the perspective projection, in world units (a camera starts with them,
	// and set_clip_planes changes them for that camera)
	static constexpr GLfloat NEAR_PLANE = 0.1f;
	static constexpr GLfloat FAR_PLANE  = 100.0f;

	// Constructors
	// ------------
	// Constructor with vectors
	Camera(
		const std::string& name,
		const glm::vec3    position = POSITION,
		const glm::vec3    up       = UP,
		const GLfloat      yaw      = YAW,
		const GLfloat      pitch    = PITCH);
	// Constructor with scalar values
	Camera(
		const std::string& name,
		const GLfloat      pos_x,
		const GLfloat      pos_y,
		const GLfloat      pos_z,
		const GLfloat      up_x,
		const GLfloat      up_y,
		const GLfloat      up_z,
		const GLfloat      yaw   = YAW,
		const GLfloat      pitch = PITCH);

	// Destructor
	// ----------
	~Camera() = default;

	// Public Methods
	// --------------
	// Loads the camera
	void load() override {}

	// Deallocates all the resources of the camera
	void deallocate_resources() override {}

	// Does not draw anything by default, as a camera has no visual representation
	virtual void draw() const override {}
	// Does not draw anything by default, as a camera has no visual representation
	virtual void draw(const Shader& shader) const override {}

	// Getters
	glm::vec3 get_position() const { return position; }
	glm::vec3 get_front() const { return front; }
	glm::vec3 get_up() const { return up; }
	glm::vec3 get_right() const { return right; }
	glm::vec3 get_world_up() const { return world_up; }
	GLfloat   get_yaw() const { return yaw; }
	GLfloat   get_pitch() const { return pitch; }
	GLfloat   get_movement_speed() const { return movement_speed; }
	GLfloat   get_mouse_sensitivity() const { return mouse_sensitivity; }
	GLfloat   get_zoom() const { return zoom; }
	GLfloat   get_near_plane() const { return near_plane; }
	GLfloat   get_far_plane() const { return far_plane; }

	// Setters
	// Moves the camera to a position, in world units, without turning it (its direction and its angles do not change). Like
	// get_position, it is the position of the view, which the camera keeps apart from the position of its node
	void set_position(const glm::vec3& position) { this->position = position; }
	// Sets the yaw angle, in degrees, and turns the camera to it (the direction vectors are recalculated)
	void set_yaw(GLfloat yaw);
	// Sets the pitch angle, in degrees, and turns the camera to it (the direction vectors are recalculated). The pitch is
	// limited to the range from -89 to 89 degrees, as the rotation with the mouse limits it, so that the camera never looks
	// straight up or down (its right vector would not be defined)
	void set_pitch(GLfloat pitch);
	// Sets the distances of the near and far planes of the projection, in world units, and returns true. Returns false and
	// keeps the current planes when they are not valid: both must be finite, the near plane greater than 0, and the far plane
	// greater than the near one. An application whose scene is very large or very small sets planes that fit it
	bool set_clip_planes(GLfloat near_plane, GLfloat far_plane);
	// Sets the speed of the movement with the keyboard (process_keyboard), in world units per second, and returns true. Returns
	// false and keeps the current speed when it is not finite or not greater than 0. An application whose scene is very large or
	// very small sets a speed that fits it, as it sets the clip planes (Reset keeps both)
	bool set_movement_speed(GLfloat speed);
	// Sets the zoom, which is the vertical field of view of the projection in degrees, limited to the range from 1 to 90 degrees
	// that the mouse wheel also respects
	void set_zoom(GLfloat zoom);

	// Turns the camera so that it looks at a point, in world units, and returns true (the yaw and the pitch are calculated from
	// the direction, and the pitch is limited as set_pitch limits it). Returns false and does not turn the camera when the point is
	// the position of the camera, which gives no direction
	bool look_at(const glm::vec3& target);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 get_view_matrix() const;
	// Returns the perspective projection matrix for a window of the given aspect ratio (width over height): the zoom of the
	// camera is the vertical field of view in degrees, and the planes are the ones of the camera (get_near_plane and
	// get_far_plane). An application that draws on the image of the scene (for example, its own highlight of the selected
	// node) combines it with the view matrix to project the points of the scene to the screen
	glm::mat4 get_projection_matrix(GLfloat aspect_ratio) const;
	// Returns the projection matrix for a window or a framebuffer of the given size in pixels (the aspect ratio is the width
	// over the height, or 1 when the height is 0). The renderer, the picking, and the outline of the selection all use it,
	// so that what is drawn, what is picked, and what is outlined always agree
	glm::mat4 get_projection_matrix(GLuint width, GLuint height) const;

	// Processes input received from any keyboard-like input system.
	// Accepts input parameter in the form of defined enum class (to abstract it from windowing systems)
	void process_keyboard(Camera_Movement direction, GLfloat delta_time);
	// Processes input received from a left-click drag event.
	// Expects the offset values in both the x and y directions and the sensitivity of the mouse movement.
	// This method is intended to be used for camera translation
	void process_mouse_translation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity = 0.1f);
	// Processes input received from a mouse right-click drag event.
	// Expects the offset values in both the x and y directions, the sensitivity of the mouse movement,
	// and whether the user wants to constrain the pitch.
	// This method is intended to be used for camera rotation
	void process_mouse_rotation(GLfloat xoffset, GLfloat yoffset, GLfloat sensitivity = 0.1f, GLboolean constrain_pitch = true);
	// Processes input received from a mouse scroll-wheel event.
	// Expects the offset value in the y direction and the sensitivity of the scroll
	void process_mouse_scroll(GLfloat yoffset, GLfloat sensitivity = 1.0f);

	// Resets the camera to its default values (its position, direction, sensitivity, and zoom). The clip planes and the movement
	// speed are kept: they fit the scale of the scene, which a reset of the view does not change
	void Reset();

private:
	// Private Attributes
	// ------------------
	// camera attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	// euler angles
	GLfloat yaw;
	GLfloat pitch;

	// camera options
	GLfloat movement_speed;
	GLfloat mouse_sensitivity;
	GLfloat zoom;
	GLfloat near_plane{ NEAR_PLANE };
	GLfloat far_plane{ FAR_PLANE };

	// initial values for camera reset
	glm::vec3 initial_position;
	glm::vec3 initial_up;
	GLfloat   initial_yaw;
	GLfloat   initial_pitch;

	// Private Static Attributes
	// -------------------------
	// default values for the camera attributes
	static constexpr glm::vec3 POSITION    = glm::vec3{ 16.0f, 6.0f, 16.0f };
	static constexpr glm::vec3 FRONT       = glm::vec3{ 0.0f, 0.0f, -1.0f };
	static constexpr glm::vec3 UP          = glm::vec3{ 0.0f, 1.0f, 0.0f };
	static constexpr GLfloat   YAW         = -135.0f;
	static constexpr GLfloat   PITCH       = -10.0f;
	static constexpr GLfloat   SPEED       = 2.5f;
	static constexpr GLfloat   SENSITIVITY = 0.1f;
	static constexpr GLfloat   ZOOM        = 45.0f;

	// Private Methods
	// ---------------
	// Recalculates the front, right and up vectors from the camera's (updated) Euler Angles
	void recalculate_vectors();
};
