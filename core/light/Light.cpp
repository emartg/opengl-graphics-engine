/*
* Light.h
* This file implements the Light class (a derived class of Node),
* which is an abstract base class used to create a light source.
*/

#include "Light.h"

#include "../texture/Texture.h"

// Static Protected Attributes
// ---------------------------
GLuint Light::nLights{}; // initialize the number of lights in the scene to 0

// Constructors
// ------------
Light::Light(const std::string& name,
			 const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular,
			 const std::shared_ptr<Node> gizmo, const LightType lightType)
	: Node(name, NodeType::LIGHT), // set the node type to LIGHT
	ambient{ ambient }, diffuse{ diffuse }, specular{ specular },
	lightType{ lightType }
{
	nLights++;
}

// Public Methods
// --------------
void Light::Draw() const
{
	auto gizmo = GetGizmo(); // get the gizmo representing the light
	if (gizmo) gizmo->Draw(); // draw the gizmo if it exists
}

void Light::Draw(const Shader& shader) const
{
	auto gizmo = GetGizmo(); // get the gizmo representing the light
	if (gizmo) gizmo->Draw(shader); // draw the gizmo with the specified shader if it exists
}

std::shared_ptr<Node> Light::GetGizmo() const
{
	// return the first child node thta has a non-NONE gizmo type (i.e., the gizmo representing the light)
	for (auto& child : children)
		if (child && child->GetGizmoType() != GizmoType::NONE)
			return child;
	// otherwise, return nullptr
	return nullptr;
}

void Light::SyncGizmoColorFromLight()
{
	auto gizmo = GetGizmo(); // get the gizmo representing the light
	if (gizmo) gizmo->SetAlbedo(diffuse); // set the gizmo's albedo to the light's diffuse color
}
