/*
* Light.h
* This file defines the Light class (a derived class of Node),
* which is an abstract base class used to create a light source.
*/

#pragma once

#include "../Node.h"

#include <iostream>
#include <string>
#include <memory> 

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

// Enumeration for the different types of lights
enum class Light_Type { UNDEFINED = 0, DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT };

class Light : public Node
{
public:
	// Constructors
	// ------------
	Light(const std::string& name,
		  const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
		  const std::shared_ptr<Node> gizmo = nullptr,
		  const Light_Type type = Light_Type::UNDEFINED);

	// Virtual destructor
	// ------------------
	virtual ~Light() = default;

	// Public Methods
	// --------------
	// Loads the light and its resources, including its children if the light is composite
	void load() override { for (const auto& child : children) if (child) child->load(); }

	// Deallocates all the resources of the light, including its children if the light is composite
	void deallocate_resources() override
	{
		for (const auto& child : children) if (child) child->deallocate_resources();
	}

	// Draws the meshes of the gizmos representing the light (if any),
	// as the light itself has no visual representation
	virtual void draw() const override;
	// Draws the meshes of the gizmos representing the light (if any) with the specified shader,
	// as the light itself has no visual representation
	virtual void draw(const Shader& shader) const override;

	// Getters
	glm::vec3 get_ambient() const { return ambient; }
	glm::vec3 get_diffuse() const { return diffuse; }
	glm::vec3 get_specular() const { return specular; }
	Light_Type get_light_type() const { return light_type; }
	std::shared_ptr<Node> get_gizmo() const;

	// Setters
	void set_ambient(glm::vec3 ambient) { this->ambient = ambient; }
	void set_diffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
	void set_specular(glm::vec3 specular) { this->specular = specular; }
	void set_light_type(Light_Type type) { light_type = type; }

	// create the gizmo for the light
	virtual void create_gizmo() = 0;
	// Syncronize gizmo's diffuse color with the light's diffuse color
	void sync_gizmo_color_from_light();

protected:
	// Protected Attributes
	// --------------------
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	Light_Type light_type; // type of the light (e.g., DIRECTIONAL_LIGHT, POINT_LIGHT, SPOTLIGHT)

};