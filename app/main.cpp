/*
* main.cpp
* This file is the entry point of the application: it initializes the engine,
* creates a window, sets up a simple scene with a camera, lights, and models,
* and runs the main loop to render the created scene.
* It uses the GLFWRenderer for rendering the scene.
*/

#include <memory> // for smart pointers
#include <filesystem>

#include "renderer/GLFWRenderer.h"

#include "../core/Core.h"
#include "../core/camera/Camera.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/managers/AssetManager.h"
#include "../core/model/Model.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"

int main(int argc, char** argv)
{
	// use the current time as seed for the random number generator 
	// (to get different positions, colors, etc. each run)
	srand(static_cast<unsigned int>(time(0)));

	// create pointer to a renderer object
	Renderer* renderer = new GLFWRenderer();

	// get engine instance
	auto engine = Core::GetInstance();
	// get the managers from the engine
	auto& assetManager = engine->GetAssetManager();
	auto& inputManager = engine->GetInputManager();
	auto& sceneManager = engine->GetSceneManager();

	// set the renderer in the engine and initialize OpenGL
	engine->SetRenderer(renderer);
	engine->InitOGL();

	// create the main camera, set it in the scene manager, and add it to the engine
	auto camera = std::make_shared<Camera>("Main Camera");
	sceneManager->SetCamera(camera);
	assetManager->AddAsset(std::move(camera));

	// get the current number of models in the scene
	std::string nModels = std::to_string(assetManager->GetNModels());
	// create a directional light object with a name "Directional Light n",
	// where n is the current number of directional lights in the scene
	auto directionalLight = std::make_shared<DirectionalLight>(
		"Directional Light " + std::to_string(assetManager->GetNDirectionalLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 1.0f, 1.0f, 0.7f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 2.4f, 8.0f, -3.0f }, // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f } // direction (overridden)
	);
	// get gizmo's shared_ptr from the directional light before adding the latter to the engine 
	// (because it will be moved and we need to keep the shared_ptr)
	auto directionalLightGizmo = directionalLight->GetGizmo();
	assetManager->AddAsset(std::move(directionalLight)); // add the directional light to the engine
	// concatenate the name of the directional light and " (Model n)",
	// where n is the current number of models in the scene
	directionalLightGizmo->SetName(directionalLightGizmo->GetName() + " (Model " + nModels + ")");
	assetManager->AddAsset(std::move(directionalLightGizmo)); // add the directional light gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(assetManager->GetNModels());
	// create a point light object with a name "Point Light n", 
	// where n is the current number of point lights in the scene
	auto pointLight = std::make_shared<PointLight>(
		"Point Light " + std::to_string(assetManager->GetNPointLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 0.3f, 0.9f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f } // position (overridden)
	);
	// get gizmo's shared_ptr from the point light before adding the latter to the engine 
	// (because it will be moved and we need to keep the shared_ptr)
	auto pointLightGizmo = pointLight->GetGizmo();
	assetManager->AddAsset(std::move(pointLight)); // add the point light to the engine
	// concatenate the name of the point light and " (Model n)", 
	// where n is the current number of models in the scene
	pointLightGizmo->SetName(pointLightGizmo->GetName() + " (Model " + nModels + ")");
	assetManager->AddAsset(std::move(pointLightGizmo)); // add the point light gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(assetManager->GetNModels());
	// create a spotlight object with a name "Spotlight n",
	// where n is the current number of spotlights in the scene
	auto spotlight = std::make_shared<Spotlight>(
		"Spotlight " + std::to_string(assetManager->GetNSpotlights()),
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
	assetManager->AddAsset(std::move(spotlight)); // add the spotlight to the engine
	// concatenate the name of the spotlight and " (Model n)",
	// where n is the current number of models in the scene
	spotlightGizmo->SetName(spotlightGizmo->GetName() + " (Model " + nModels + ")");
	assetManager->AddAsset(std::move(spotlightGizmo)); // add the spotlight gizmo to the engine

	// update the number of models in the scene
	nModels = std::to_string(assetManager->GetNModels());
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
	auto trafficConeModel = std::make_shared<AssimpModel>(
		filename + " (Model " + nModels + ")",
		filepath,
		glm::vec3{ 0.8f }, // diffuse color (override required although it is the default)
		glm::vec3{ 0.0f }, // position (override required although it is the default)
		glm::vec3{ 0.0f, 45.0f, 0.0f }, // rotation in Euler angles (overridden)
		glm::vec3{ 0.15f } // scale (overridden)
	);
	assetManager->AddAsset(std::move(trafficConeModel)); // add the model to the engine

	// define shader program names and their paths and compile the shaders
	std::vector<std::string> shaderProgramNames{
		"Untextured Matt Shape Shader Program",
		"Assimp Model Shader Program",
		"Single Albedo Shader Program",
		"Assimp Model Outline Shader Program",
		"Shape Outline Shader Program"
	};
	std::vector<std::string> vertexShaderPaths{
		"shaders/untextured_matt_shape.vert.glsl",
		"shaders/assimp_model.vert.glsl",
		"shaders/single_albedo.vert.glsl",
		"shaders/assimp_model_outline.vert.glsl",
		"shaders/shape_outline.vert.glsl"
	};
	std::vector<std::string> fragmentShaderPaths{
		"shaders/untextured_matt_shape.frag.glsl",
		"shaders/assimp_model.frag.glsl",
		"shaders/single_albedo.frag.glsl",
		"shaders/assimp_model_outline.frag.glsl",
		"shaders/shape_outline.frag.glsl"
	};
	engine->CompileShaders(shaderProgramNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	engine->DestroyInstance();

	return 0;
}
