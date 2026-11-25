/*
 * Node_Factory.h
 * This file defines the Shape_Descriptor and Light_Descriptor structures and the Node_Factory class, which reads the
 * descriptors of the shape and light assets and builds the nodes that they describe, so that the basic shapes and the
 * lights are assets like the materials: ordinary files that a user can move, duplicate, and change.
 *
 * A shape descriptor (<name>.shape.json) names a built-in shape and its parameters ("size", "color", and "two_sided"
 * are optional; the size is in metres, the color is RGB or RGBA from 0 to 1, and a plane has a width and a depth):
 * { "name": "Cube Shape", "shape": "cube", "size": [ 2.0, 2.0, 2.0 ], "color": [ 0.8, 0.8, 0.8, 1.0 ] }
 * { "name": "Plane Shape", "shape": "plane", "size": [ 2.0, 2.0 ], "two_sided": true }
 *
 * A light descriptor (<name>.light.json) gives the type of the light ("directional", "point", or "spot") and its
 * optional values: the colors ("color" is the diffuse color, with "ambient" and "specular"), the "direction" of a
 * directional light or a spotlight, the "range" of a point light or a spotlight (in metres, without it the light keeps
 * the engine's attenuation, which reaches about 50 metres), the cut-off angles of a spotlight (in degrees), and the size
 * of the gizmo (in metres):
 * { "name": "Spotlight", "type": "spot", "color": [ 0.8, 0.8, 0.8 ], "direction": [ 0.0, -1.0, 0.0 ], "range": 50.0,
 *   "inner_cutoff": 12.5, "outer_cutoff": 15.0, "gizmo_size": 0.3 }
 *
 * The lengths of a descriptor are in metres, and the factory converts them to the units of the scene with the number of
 * units per metre that it is given (Scene_Manager::get_units_per_metre), so the same asset fits a scene in metres and a
 * scene in centimetres. Reading a descriptor needs no OpenGL context; building a node does (its meshes are uploaded).
 */

#pragma once

#include "../light/Light.h"
#include "../model/Shape_Model.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Built-in shapes that a shape descriptor can name
enum class Shape_Primitive
{
	CUBE, // a box (its size is its width, height, and depth)
	PLANE // a flat rectangle facing up (its size is its width and depth)
};

// Contents of a shape descriptor file
struct Shape_Descriptor
{
	std::string              name;                               // the name of the node (before its identifier is added)
	Shape_Primitive          primitive{ Shape_Primitive::CUBE }; // the built-in shape
	std::optional<glm::vec3> size;                               // in metres (y is 0 for a plane); the shape's own size if not given
	glm::vec4                color{ 0.8f, 0.8f, 0.8f, 1.0f };    // the albedo of the node, whose alpha is its opacity
	bool                     is_two_sided{ false };              // true by default for a plane, false for a cube
};

// Contents of a light descriptor file
struct Light_Descriptor
{
	std::string              name;                            // the name of the node (before its identifier is added)
	Light_Type               type{ Light_Type::POINT_LIGHT }; // directional, point, or spot
	glm::vec3                ambient{ 0.1f };                 // ambient color
	glm::vec3                diffuse{ 0.8f };                 // diffuse color (the "color" field)
	glm::vec3                specular{ 1.0f };                // specular color
	std::optional<glm::vec3> direction;                       // directional lights and spotlights; their default if not given
	std::optional<GLfloat>   range;                           // point lights and spotlights, in metres; the default if not given
	GLfloat                  inner_cutoff{ 12.5f };           // spotlights, in degrees
	GLfloat                  outer_cutoff{ 15.0f };           // spotlights, in degrees
	std::optional<GLfloat>   gizmo_size;                      // in metres; the default of the kind of light if not given
};

class Node_Factory
{
public:
	// Public Constants
	// ----------------
	// largest cut-off angle of a spotlight, in degrees (the cone must stay narrower than a half space)
	static constexpr GLfloat MAX_CUTOFF_DEGREES{ 89.0f };

	// Public Static Methods
	// ---------------------
	// Parses the text of a shape descriptor (JSON). Returns the descriptor, or an empty optional (and an error message) if
	// the text is not valid JSON, lacks a required field ("name" and "shape"), names an unknown shape, or has a field of an
	// invalid type or value (a size that is not positive, a color outside 0 to 1)
	static std::optional<Shape_Descriptor> parse_shape_descriptor(const std::string& text, std::string& error);
	// Parses the text of a light descriptor (JSON). Returns the descriptor, or an empty optional (and an error message) if
	// the text is not valid JSON, lacks a required field ("name" and "type"), names an unknown type, has a field of an
	// invalid type or value, or has a field that the type of light does not have (e.g., a "range" for a directional light)
	static std::optional<Light_Descriptor> parse_light_descriptor(const std::string& text, std::string& error);

	// Reads and parses a shape or a light descriptor file. Returns an empty optional (and an error message that names the
	// file) if it cannot be read or is not a valid descriptor
	static std::optional<Shape_Descriptor> load_shape_descriptor(const std::filesystem::path& file, std::string& error);
	static std::optional<Light_Descriptor> load_light_descriptor(const std::filesystem::path& file, std::string& error);

	// Returns the scale of the node of a shape descriptor in a scene with the given number of units per metre: its size
	// divided by the size of the built-in shape (a cube and a plane are 2 units across), or 1 on each axis without a size
	static glm::vec3 get_shape_scale(const Shape_Descriptor& descriptor, GLfloat units_per_metre);

	// Builds the node of a shape descriptor at a position (in the units of the scene), named "{id: N} <name>". It needs an
	// OpenGL context. The caller adds it to the scene (Node_Manager::add_node)
	static std::shared_ptr<Shape_Model>
	create_shape(const Shape_Descriptor& descriptor, const glm::vec3& position, GLfloat units_per_metre = 1.0f);
	// Builds the light of a light descriptor at a position (in the units of the scene), named "{id: N} <name>", with its
	// gizmo. It needs an OpenGL context. The caller adds the light and its gizmo to the scene (Node_Manager::add_node)
	static std::shared_ptr<Light>
	create_light(const Light_Descriptor& descriptor, const glm::vec3& position, GLfloat units_per_metre = 1.0f);
};
