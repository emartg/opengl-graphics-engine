/*
* main.cpp
* This file is is an entry point to the App module. It serves as a simple test of the Core engine.
* It follows these steps:
* - It initializes the Core engine, which sets up OpenGL, window, and GUI.
* - It compiles shaders and sets up the initial scene with a camera, lights, and a model.
* - It runs the main loop of the engine, which renders the scene and handles events.
* - It cleans up resources in the correct order and shuts down the engine.
* Other important notes:
* - GLFWRenderer is used as the renderer implementation for the Core engine.
* - The application uses the Core library to manage assets, input, and scene management.
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

// Defines hardcoded shader names and paths for the initial scene in a tuple
std::tuple<
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>> DefineShadersInfo();
// Sets up the initial scene with a camera, lights, and a model
void SetupInitialScene(Core* engine);

int main(int argc, char** argv)
{
	std::cout << "[main] Starting the application..." << std::endl;

	Core* engine = Core::GetInstance(); // retrieve the singleton instance of the Core class

	// create a GLFWRenderer instance and set it as the renderer for the engine
	Renderer* renderer = new GLFWRenderer();
	engine->SetRenderer(renderer);

	if (engine->Init()) // initialize the engine (OpenGL, window, GUI, etc.)
	{ // if the initialization is successful, print a success message
		std::cout << "[main] Core initialized successfully" << std::endl;
	}
	else
	{
		// if the initialization fails, print an error messages and shut down the engine, 
		// then prompt the user to exit
		std::cerr << "[main] Failed to initialize OpenGL" << std::endl;
		engine->Shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	// get the shader names and paths for the initial scene
	auto [shaderNames, vertexShaderPaths, geometryShaderPaths, fragmentShaderPaths]
		= DefineShadersInfo();

	// create, compile, and link the shader programs, and add them to the engine's asset manager
	if (engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths))
	{ // if the shaders are compiled successfully, print a success message
		std::cout << "[main] Shaders compiled successfully" << std::endl;
	}
	else
	{ // if the shader compilation fails, print an error message and shut down the engine
		std::cerr << "[main] Failed to compile shaders" << std::endl;
		engine->Shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	SetupInitialScene(engine); // set up the initial scene with a camera, lights, and a model

	engine->Run(); // run the main loop of the engine, which will render the scene and handle events

	engine->Shutdown(); // clean up resources in the correct order and shut down the engine

	std::cout << "[main] Application finished successfully" << std::endl;

	return 0;
}

std::tuple<std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>> DefineShadersInfo()
{
	std::vector<std::string> shaderNames = {
		"Untextured Matt Shape Shader",
		"Assimp Model Shader",
		"Single Albedo Shader",
		"Screen Shader",
		"Picking Shader"
	};
	std::vector<std::string> vertexShaderPaths = {
		"resources/shaders/untextured_matt_shape.vert.glsl",
		"resources/shaders/assimp_model.vert.glsl",
		"resources/shaders/single_albedo.vert.glsl",
		"resources/shaders/screen_quad.vert.glsl",
		"resources/shaders/picking.vert.glsl"
	};
	std::vector<std::string> geometryShaderPaths = {
		"", // no custom geometry shader for the untextured matt shape shader
		"", // no custom geometry shader for the assimp model shader
		""  // no custom geometry shader for the single albedo shader
		"", // no custom geometry shader for the screen shader
		""  // no custom geometry shader for the picking shader
	};
	std::vector<std::string> fragmentShaderPaths = {
		"resources/shaders/untextured_matt_shape.frag.glsl",
		"resources/shaders/assimp_model.frag.glsl",
		"resources/shaders/single_albedo.frag.glsl",
		"resources/shaders/screen_quad.frag.glsl",
		"resources/shaders/picking.frag.glsl"
	};
	return { shaderNames, vertexShaderPaths, geometryShaderPaths, fragmentShaderPaths };
}

void SetupInitialScene(Core* engine)
{
	// retrieve the asset manager and scene manager from the engine
	auto& assetManager = engine->GetAssetManager();
	auto& sceneManager = engine->GetSceneManager();

	// create a camera, set it as the active camera in the scene manager and add it to the asset manager
	auto camera = std::make_shared<Camera>("Main Camera");
	sceneManager->SetCamera(camera);
	assetManager->AddAsset(std::move(camera));

	// create a directional light and its gizmo, then add them to the asset manager
	auto directionalLight = std::make_shared<DirectionalLight>(
		"Directional Light " + std::to_string(assetManager->GetNDirectionalLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 1.0f, 1.0f, 0.7f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 2.4f, 8.0f, -3.0f }, // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f } // direction (overridden)
	);
	auto directionalLightGizmo = directionalLight->GetGizmo();
	assetManager->AddAsset(std::move(directionalLight));
	directionalLightGizmo->SetName(directionalLightGizmo->GetName()
								   + " (Model " + std::to_string(assetManager->GetNModels()) + ")");
	assetManager->AddAsset(std::move(directionalLightGizmo));

	// create a point light and its gizmo, then add them to the asset manager
	auto pointLight = std::make_shared<PointLight>(
		"Point Light " + std::to_string(assetManager->GetNPointLights()),
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 0.3f, 0.9f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f } // position (overridden)
	);
	auto pointLightGizmo = pointLight->GetGizmo();
	assetManager->AddAsset(std::move(pointLight));
	pointLightGizmo->SetName(pointLightGizmo->GetName()
							 + " (Model " + std::to_string(assetManager->GetNModels()) + ")");
	assetManager->AddAsset(std::move(pointLightGizmo));

	// create a spotlight and its gizmo, then add them to the asset manager
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
	auto spotlightGizmo = spotlight->GetGizmo();
	assetManager->AddAsset(std::move(spotlight));
	spotlightGizmo->SetName(spotlightGizmo->GetName()
							+ " (Model " + std::to_string(assetManager->GetNModels()) + ")");
	assetManager->AddAsset(std::move(spotlightGizmo));

	// load a model from an specific filepath and add it to the asset manager
	std::string filepath = "resources/models/traffic_cone/gltf/traffic_cone.gltf";
	if (std::filesystem::exists(filepath))
	{ // check if the file exists before loading it
		std::string filename = std::filesystem::path(filepath).filename().string();
		auto trafficConeModel = std::make_shared<AssimpModel>(
			filename + " (Model " + std::to_string(assetManager->GetNModels()) + ")", filepath,
			glm::vec3{ 0.8f }, // diffuse color (override required although it is the default)
			glm::vec3{ 0.0f }, // position (override required although it is the default)
			glm::vec3{ 0.0f, 45.0f, 0.0f }, // rotation in Euler angles (overridden)
			glm::vec3{ 0.15f } // scale (overridden)
		);
		assetManager->AddAsset(std::move(trafficConeModel));
	}
	else // if the file does not exist, print an error message
		std::cerr << "[main::SetupInitialScene] File not found: " << filepath << std::endl;
}