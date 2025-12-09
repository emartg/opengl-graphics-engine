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
	gizmo{ gizmo }, lightType{ lightType }
{
	nLights++;
}