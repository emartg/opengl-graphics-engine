/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, a point light (with its gizmo),
* and a cube shape to the engine, compiles the shaders, and runs the main loop.
*/

#include <memory> // for smart pointers
#include <filesystem>

// includes from this module
#include "../core/Core.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Model.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"

// includes from the App module
#include "renderer/GLFWRenderer.h"

int main(int argc, char** argv)
{
	// use the current time as seed for the random number generator 
	// (to get different positions, colors, etc. each run)
	srand(static_cast<unsigned int>(time(0)));

	// create pointer to a renderer object
	Renderer* renderer = new GLFWRenderer();

	// get engine instance, set the renderer, and initialize OpenGL
	auto engine = Core::GetInstance();
	engine->SetRenderer(renderer);
	engine->InitOGL();

	// create the main camera
	auto camera = std::make_shared<Camera>(
		"Main Camera",
		glm::vec3{ 22.0f, 12.0f, 17.5f } // position (overridden)
	);
	engine->AddAsset(std::move(camera)); // add the camera to the engine

	// get the current number of models in the scene
	std::string nModels = std::to_string(engine->GetNModels());
	// create a directional light object with a name "Directional Light n",
	// where n is the current number of directional lights in the scene
	auto directionalLight = std::make_shared<DirectionalLight>(
		"Directional Light " + std::to_string(engine->GetNDirectionalLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 1.0f, 1.0f, 0.7f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 2.4f, 8.0f, -3.0f }, // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f } // direction (overridden)
	);
	// get gizmo's shared_ptr from the directional light before adding the latter to the engine 
	// (because it will be moved and we need to keep the shared_ptr)
	auto directionalLightGizmo = directionalLight->GetGizmo();
	engine->AddAsset(std::move(directionalLight)); // add the directional light to the engine
	// concatenate the name of the directional light and " (Model n)",
	// where n is the current number of models in the scene
	directionalLightGizmo->SetName(directionalLightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(directionalLightGizmo)); // add the directional light gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// create a point light object with a name "Point Light n", 
	// where n is the current number of point lights in the scene
	auto pointLight = std::make_shared<PointLight>(
		"Point Light " + std::to_string(engine->GetNPointLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 0.3f, 0.9f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f } // position (overridden)
	);
	// get gizmo's shared_ptr from the point light before adding the latter to the engine 
	// (because it will be moved and we need to keep the shared_ptr)
	auto pointLightGizmo = pointLight->GetGizmo();
	engine->AddAsset(std::move(pointLight)); // add the point light to the engine
	// concatenate the name of the point light and " (Model n)", 
	// where n is the current number of models in the scene
	pointLightGizmo->SetName(pointLightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(pointLightGizmo)); // add the point light gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// create a spotlight object with a name "Spotlight n",
	// where n is the current number of spotlights in the scene
	auto spotlight = std::make_shared<Spotlight>(
		"Spotlight " + std::to_string(engine->GetNSpotlights()),
		glm::vec3{ 0.1f }, // ambient color (override required although it is the default)
		glm::vec3{ 1.0f, 0.4f, 0.4f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 3.0f, -0.3f, -0.9f }, // position (overridden)
		glm::vec3{ -0.8f, 0.3f, 0.6f }, // direction (overridden)
		glm::cos(glm::radians(15.0f)), // inner cut-off (overridden)
		glm::cos(glm::radians(32.5f)) // outer cut-off (overridden)
	);
	// get gizmo's shared_ptr from the spotlight before adding the latter to the engine 
	// (because it will be moved and we need to keep the shared_ptr)
	auto spotlightGizmo = spotlight->GetGizmo();
	engine->AddAsset(std::move(spotlight)); // add the spotlight to the engine
	// concatenate the name of the spotlight and " (Model n)",
	// where n is the current number of models in the scene
	spotlightGizmo->SetName(spotlightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(spotlightGizmo)); // add the spotlight gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// define the path to the 3D model file
	std::string filepath = "assets/models/traffic_cone/gltf/traffic_cone.gltf";
	if (!std::filesystem::exists(filepath)) // check if the file exists
	{ // if the file does not exist, print an error message and exit the program
		std::cerr << "Error: The file " << filepath << " does not exist." << std::endl;
		return -1;
	}
	// get the filename from the file path
	std::string filename = std::filesystem::path(filepath).filename().string();
	// load the model from the assets folder and create an AssimpModel object.
	// The name of the model will "filename (Model n)"
	auto constructionHelmetModel = std::make_shared<AssimpModel>(
		filename + " (Model " + nModels + ")", 
		filepath,
		glm::vec3{ 0.5f }, // diffuse color (override required although it is the default)
		glm::vec3{ 0.0f }, // position (override required although it is the default)
		glm::vec3{ 0.0f, 45.0f, 0.0f }, // rotation in Euler angles (overridden)
		glm::vec3{ 0.15f } // scale (overridden)
	);
	engine->AddAsset(std::move(constructionHelmetModel)); // add the model to the engine

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{
		"Untextured Matt Shape Shader Program",
		"Assimp Model Shader Program",
		"Gizmo Shape Shader Program" };
	std::vector<std::string> vertexShaderPaths{
		"shaders/untextured_matt_shape.vert.glsl",
		"shaders/assimp_model.vert.glsl" ,
		"shaders/gizmo_shape.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{
		"shaders/untextured_matt_shape.frag.glsl",
		"shaders/assimp_model.frag.glsl",
		"shaders/gizmo_shape.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	engine->DestroyInstance();

	return 0;
}
