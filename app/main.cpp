/*
* main.cpp
* This file is is an entry point to the App module. It serves as a simple test of the Core engine.
* It follows these steps:
* - It initializes the Core engine, which sets up OpenGL, window, and GUI.
* - It compiles shaders and sets up the initial scene.
* - It runs the main loop of the engine, which renders the scene and handles events.
* - It cleans up resources in the correct order and shuts down the engine.
* Other important notes:
* - GLFWRenderer is used as the renderer implementation for the Core engine.
* - The application uses the Core library to manage nodes, input, and scene management.
*/

#include <memory>
#include <filesystem>

#include "CUBE.h"
#include "PLANE.h"
#include "renderer/GLFWRenderer.h"

#include "../core/Core.h"
#include "../core/Node.h"
#include "../core/camera/Camera.h"
#include "../core/light/Light.h"
#include "../core/light/DirectionalLight.h"
#include "../core/light/PointLight.h"
#include "../core/light/Spotlight.h"
#include "../core/model/Model.h"
#include "../core/model/Shape.h"
#include "../core/model/AssimpModel.h"
#include "../core/managers/SceneManager.h"
#include "../core/managers/NodeManager.h"

// Defines hardcoded shader names and paths for the initial scene in a tuple
std::tuple<
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>,
	std::vector<std::string>> DefineShadersInfo();
// Sets up the initial scene with a camera, lights, a shape, an Assimp model, and a skybox
void SetupInitialScene(Core* engine);

int main(int argc, char** argv)
{
	std::cout << "[INFO::main] Starting the application..." << std::endl;

	Core* engine = Core::GetInstance(); // retrieve the singleton instance of the Core class

	// create a GLFWRenderer instance and set it as the renderer for the engine
	Renderer* renderer = new GLFWRenderer();
	engine->SetRenderer(renderer);

	if (engine->Init()) // initialize the engine (OpenGL, window, GUI, etc.)
	{ // if the initialization is successful, print a success message
		std::cout << "[SUCCESS::main] Core initialized successfully" << std::endl;
	}
	else
	{
		// if the initialization fails, print an error messages and shut down the engine, 
		// then prompt the user to exit
		std::cerr << "[ERROR::main] Failed to initialize OpenGL" << std::endl;
		engine->Shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	// get the shader names and paths for the initial scene
	auto [shaderNames, vertexShaderPaths, geometryShaderPaths, fragmentShaderPaths]
		= DefineShadersInfo();

	// create, compile, and link the shader programs, and add them to the engine's node manager
	if (engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths))
	{ // if the shaders are compiled successfully, print a success message
		std::cout << "[SUCCESS::main] Shaders compiled successfully" << std::endl;
	}
	else
	{ // if the shader compilation fails, print an error message and shut down the engine
		std::cerr << "[ERROR::main] Failed to compile shaders" << std::endl;
		engine->Shutdown();
		// wait until the user presses a key before exiting
		std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
		std::cin.get();
		return -1; // exit the program with an error code
	}

	SetupInitialScene(engine); // set up the initial scene with a camera, lights, and a model

	engine->Run(); // run the main loop of the engine, which will render the scene and handle events

	engine->Shutdown(); // clean up resources in the correct order and shut down the engine

	std::cout << "[SUCCESS::main] Application finished successfully" << std::endl;

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
		"Picking Shader",
		"Skybox Shader",
		"Equirectangular to Cubemap Shader",
		"Reflective Shader",
		"Refractive Shader"
	};

	std::string shadersDir = "resources/shaders/";

	std::vector<std::string> vertexShaderPaths = {
		shadersDir + "untextured_matt_shape.vert.glsl",
		shadersDir + "assimp_model.vert.glsl",
		shadersDir + "single_albedo.vert.glsl",
		shadersDir + "screen_quad.vert.glsl",
		shadersDir + "picking.vert.glsl",
		shadersDir + "skybox.vert.glsl",
		shadersDir + "equirectangular_to_cubemap.vert.glsl",
		shadersDir + "reflective.vert.glsl",
		shadersDir + "refractive.vert.glsl"
	};
	std::vector<std::string> geometryShaderPaths = {
		"", // no custom geometry shader for the untextured matt shape shader
		"", // no custom geometry shader for the assimp model shader
		""  // no custom geometry shader for the single albedo shader
		"", // no custom geometry shader for the screen shader
		"", // no custom geometry shader for the picking shader
		"", // no custom geometry shader for the skybox shader
		"", // no custom geometry shader for the equirectangular to cubemap shader
		"", // no custom geometry shader for the reflective shader
		""  // no custom geometry shader for the refractive shader
	};
	std::vector<std::string> fragmentShaderPaths = {
		shadersDir + "untextured_matt_shape.frag.glsl",
		shadersDir + "assimp_model.frag.glsl",
		shadersDir + "single_albedo.frag.glsl",
		shadersDir + "screen_quad.frag.glsl",
		shadersDir + "picking.frag.glsl",
		shadersDir + "skybox.frag.glsl",
		shadersDir + "equirectangular_to_cubemap.frag.glsl",
		shadersDir + "reflective.frag.glsl",
		shadersDir + "refractive.frag.glsl"
	};
	return { shaderNames, vertexShaderPaths, geometryShaderPaths, fragmentShaderPaths };
}

void SetupInitialScene(Core* engine)
{
	// retrieve the node manager and scene manager from the engine
	auto& nodeManager = engine->GetNodeManager();
	auto& sceneManager = engine->GetSceneManager();

	// create a camera, set it as the active camera in the scene manager and add it to the node manager
	auto camera = std::make_shared<Camera>("Main Camera");
	sceneManager->SetCamera(camera);
	nodeManager->AddNode(std::move(camera));

	// create a directional light and add it along with its gizmo to the node manager
	auto directionalLight = std::make_shared<DirectionalLight>(
		"Directional Light",
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 1.0f, 1.0f, 0.7f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 2.4f, 8.0f, -3.0f }, // position (overridden)
		glm::vec3{ -0.2f, -0.8, 0.5f } // direction (overridden)
	);
	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	directionalLight->CreateGizmo();
	// register the gizmo child for selection/picking before moving the light
	auto directionalLightGizmo = directionalLight->GetGizmo();
	nodeManager->AddNode(std::move(directionalLightGizmo));
	nodeManager->AddNode(std::move(directionalLight));

	// create a point light and add it along with its gizmo to the node manager
	auto pointLight = std::make_shared<PointLight>(
		"Point Light",
		glm::vec3{ 0.1f }, // ambient color (default)
		glm::vec3{ 0.3f, 0.9f, 1.0f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (same as default)
		glm::vec3{ -0.6f, 3.2f, 3.2f } // position (overridden)
	);
	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	pointLight->CreateGizmo();
	// register the gizmo child for selection/picking before moving the light
	auto pointLightGizmo = pointLight->GetGizmo();
	nodeManager->AddNode(std::move(pointLightGizmo));
	nodeManager->AddNode(std::move(pointLight));

	// create a spotlight and add it along with its gizmo to the node manager
	auto spotlight = std::make_shared<Spotlight>(
		"Spotlight",
		glm::vec3{ 0.1f }, // ambient color (override required although it is the default)
		glm::vec3{ 1.0f, 0.4f, 0.4f }, // diffuse color (overridden)
		glm::vec3{ 1.0f }, // specular color (override required although it is the default)
		glm::vec3{ 3.0f, -0.3f, -0.9f }, // position (overridden)
		glm::vec3{ -0.8f, 0.3f, 0.6f }, // direction (overridden)
		glm::cos(glm::radians(15.0f)), // inner cut-off (overridden)
		glm::cos(glm::radians(32.5f)) // outer cut-off (overridden)
	);
	// create the gizmo child (the light has been fully constructed and placed in a shared_ptr)
	spotlight->CreateGizmo();
	// register the gizmo child for selection/picking before moving the light
	auto spotlightGizmo = spotlight->GetGizmo();
	if (spotlightGizmo) nodeManager->AddNode(std::move(spotlightGizmo));
	nodeManager->AddNode(std::move(spotlight));

	// create a composite model hierarchy mixing shapes and an Assimp model
	auto rootGroup = std::make_shared<Model>("Root Group");

	// create a shapes group to hold multiple shapes as children of the root group
	auto shapesGroup = std::make_shared<Model>(
		"Shapes Group", NodeType::COMPOSITE_MODEL,
		glm::vec3{ 1.0f }, // albedo (overriden)
		glm::vec3{ 0.0f, 0.0f, -2.5f } // position (overriden - offset from root)
	);

	// create two shapes with different colors and transformations as children of the shapes group
	auto redShape = std::make_shared<Shape>(
		"Red Shape",
		cubeVerticesVec, cubeIndicesVec,
		glm::vec3{ 0.9f, 0.2f, 0.2f }, // albedo (overridden)
		glm::vec3{ -1.0f, 0.0f, 0.0f }, // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(15.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.8f } // scale (overridden)
	);
	auto blueShape = std::make_shared<Shape>(
		"Blue Shape",
		cubeVerticesVec, cubeIndicesVec,
		glm::vec3{ 0.2f, 0.2f, 0.9f }, // albedo (overridden)
		glm::vec3{ 1.0f, 0.0f, 0.0f }, // position (overridden - local offset from parent)
		glm::quat(glm::vec3{ 0.0f, glm::radians(-25.0f), 0.0f }), // rotation (overridden)
		glm::vec3{ 0.6f } // scale (overridden)
	);

	// attach shapes under the shapes group
	shapesGroup->AddChild(redShape);
	shapesGroup->AddChild(blueShape);

	// create an Assimp model and add it as a child of the root group
	std::string assimpPath = "resources/models/gltf/teapot/teapot.gltf";
	if (std::filesystem::exists(assimpPath))
	{ // check if the file exists before loading it
		auto assimpModel = std::make_shared<AssimpModel>(
			"Teapot Model", assimpPath,
			glm::vec3{ 1.0f }, // albedo (overriden - unused if model has textures)
			glm::vec3{ 0.0f, 0.0f, 3.0f }, // position (overriden - offset from root)
			glm::quat(glm::vec3{ 0.0f }), // rotation (default)
			glm::vec3{ 0.25f } // scale (overriden)
		);

		// create a marker shape to indicate the origin of the Assimp model of which it is a child
		// (demonstrates mixed hierarchy)
		auto markerShape = std::make_shared<Shape>(
			"Teapot Origin Marker",
			cubeVerticesVec, cubeIndicesVec,
			assimpModel->GetAlbedo(), // albedo (overriden - same as parent)
			glm::vec3{ 0.5f, 0.0f, 0.5f }, // position (overridden - local offset from parent)
			glm::quat(glm::vec3{ 0.0f }), // rotation (default)
			glm::vec3{ 0.1f } // scale (overridden)
		);

		// attach marker shape under Assimp model
		assimpModel->AddChild(markerShape);

		// attach Assimp model under the root group
		rootGroup->AddChild(assimpModel);

		// register child nodes for selection/picking (renderer skips them via parent check)
		nodeManager->AddNode(assimpModel);
		nodeManager->AddNode(markerShape);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[WARNING::main::SetupInitialScene] Assimp model not found: " << assimpPath
			<< std::endl;
	}

	// attach the shapes group under the root group
	rootGroup->AddChild(shapesGroup);

	// register all nodes (including children) so they exist as nodes
	nodeManager->AddNode(rootGroup);
	nodeManager->AddNode(shapesGroup);
	nodeManager->AddNode(redShape);
	nodeManager->AddNode(blueShape);

	// load an HDR skybox texture and set it as the skybox in the scene manager
	std::string skyboxFilepath = "resources/textures/skyboxes/hdr/tiergarten_4k.hdr";
	if (std::filesystem::exists(skyboxFilepath))
	{ // check if the file exists before loading it
		sceneManager->LoadSkybox(skyboxFilepath);
	}
	else
	{ // if the file does not exist, print an error message
		std::cerr << "[ERROR::main::SetupInitialScene] Skybox file not found: " << skyboxFilepath
			<< std::endl;
	}

	// print a success message indicating the initial scene setup is complete
	std::cout << "[SUCCESS::main::SetupInitialScene] Initial scene setup completed successfully"
		<< std::endl;
}