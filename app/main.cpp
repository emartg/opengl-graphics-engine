/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, a point light (with its gizmo),
* and a cube shape to the engine, compiles the shaders, and runs the main loop.
*/

#include <memory> // for smart pointers

// includes from the engine
#include "../core/Core.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Model.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"

// includes from the app itself
#include "CUBE.h"
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
	auto camera = std::make_shared<Camera>("Main Camera");
	engine->AddAsset(std::move(camera)); // add the camera to the engine

	// get the current number of models in the scene
	std::string nModels = std::to_string(engine->GetNModels());
	// create a directional light object with a name "Directional Light n",
	// where n is the current number of directional lights in the scene
	auto directionalLight = std::make_shared<DirectionalLight>("Directional Light "
															   + std::to_string(engine->GetNDirectionalLights()),
															   glm::vec3{ 0.1f }, // ambient color (default)
															   glm::vec3{ 1.0f, 1.0f, 0.7f }, // diffuse color
															   glm::vec3{ 1.0f }, // specular color (same as default)
															   glm::vec3{ 2.4f, 8.0f, -3.0f }, // position
															   glm::vec3{ -0.2f, -0.8, 0.5f } // direction
	);
	// get gizmo's shared_ptr from the directional light before adding the latter to the engine (as it will be moved)
	auto directionalLightGizmo = directionalLight->GetGizmo();
	engine->AddAsset(std::move(directionalLight)); // add the directional light to the engine
	// concatenate the name of the directional light and " (Model n)",
	// where n is the current number of models in the scene
	directionalLightGizmo->SetName(directionalLightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(directionalLightGizmo)); // add the directional light gizmo (a rectangular plane) to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// create a point light object with a name "Point Light n", 
	// where n is the current number of point lights in the scene
	auto pointLight = std::make_shared<PointLight>("Point Light " + std::to_string(engine->GetNPointLights()),
												   glm::vec3{ 0.1f }, // ambient color (default)
												   glm::vec3{ 0.3f, 0.9f, 1.0f }, // diffuse color
												   glm::vec3{ 1.0f }, // specular color (same as default)
												   glm::vec3{ -0.6f, 3.2f, 3.2f } // position
	);
	// get gizmo's shared_ptr from the point light before adding the latter to the engine (as it will be moved)
	auto pointLightGizmo = pointLight->GetGizmo();
	engine->AddAsset(std::move(pointLight)); // add the point light to the engine
	// concatenate the name of the point light and " (Model n)", 
	// where n is the current number of models in the scene
	pointLightGizmo->SetName(pointLightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(pointLightGizmo)); // add the point light gizmo (a decahedron) to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// create a spotlight object with a name "Spotlight n",
	// where n is the current number of spotlights in the scene
	auto spotlight = std::make_shared<Spotlight>("Spotlight " + std::to_string(engine->GetNSpotlights()),
												 glm::vec3{ 0.1f }, // ambient color (default)
												 glm::vec3{ 1.0f, 0.4f, 0.4f }, // diffuse color
												 glm::vec3{ 1.0f }, // specular color (same as default)
												 glm::vec3{ 3.0f, -0.3f, -0.9f }, // position
												 glm::vec3{ -0.8f, 0.3f, 0.6f }, // direction
												 glm::cos(glm::radians(15.0f)), // inner cut-off
												 glm::cos(glm::radians(32.5f)) // outer cut-off
	);
	// get gizmo's shared_ptr from the spotlight before adding the latter to the engine (as it will be moved)
	auto spotlightGizmo = spotlight->GetGizmo();
	engine->AddAsset(std::move(spotlight)); // add the spotlight to the engine
	// concatenate the name of the spotlight and " (Model n)",
	// where n is the current number of models in the scene
	spotlightGizmo->SetName(spotlightGizmo->GetName() + " (Model " + nModels + ")");
	engine->AddAsset(std::move(spotlightGizmo)); // add the spotlight gizmo (a hexagonal pyramid) to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// load the model from the assets folder and create an AssimpModel object
	auto hydrantModel = std::make_shared<AssimpModel>(
		"Hydrant Model (Model " + nModels + ")",
		"assets/models/hydrant/hydrant.obj"
	);
	hydrantModel->SetPosition(glm::vec3{ -1.5f, 0.0f, 0.0f });
	hydrantModel->SetScale(glm::vec3{ 0.05f }); // scale the model down
	engine->AddAsset(std::move(hydrantModel)); // add the model to the engine

	// update the number of models in the scene
	nModels = std::to_string(engine->GetNModels());
	// load the model from the assets folder and create an AssimpModel object
	auto constructionHelmetModel = std::make_shared<AssimpModel>(
		"Construction Helmet Model (Model " + nModels + ")",
		"assets/models/construction_helmet/construction_helmet.obj"
	);
	constructionHelmetModel->SetPosition(glm::vec3{ 1.5f, 0.0f, 0.0f });
	constructionHelmetModel->SetRotationInEulerAngles(glm::vec3{ 0.0f, 30.0f, 0.0f });
	constructionHelmetModel->SetScale(glm::vec3{ 0.1f }); // scale the model down
	engine->AddAsset(std::move(constructionHelmetModel)); // add the model to the engine

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Model Shader Program", "Gizmo Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/model.vert.glsl" , "shaders/gizmo.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/model.frag.glsl", "shaders/gizmo.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	engine->DestroyInstance();

	return 0;
}
