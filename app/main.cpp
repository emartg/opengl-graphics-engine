/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, a point light (with its gizmo),
* and a cube shape to the engine, compiles the shaders, and runs the main loop.
*/

#include <memory> // for smart pointers

#include "CUBE.h"
#include "../core/Core.h"
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

	// get the number of shapes in the scene (which is 0 at this point, as no shapes have been added yet)
	// to use it for the name of the point light gizmo before adding the point light to the engine
	// (since the creation of the point light gizmo - a Shape - is done in the constructor of the PointLight class)
	std::string nShapes = std::to_string(engine->GetNShapes());
	// create a point light object with a name "Point Light n", 
	// where n is the current number of point lights in the scene
	auto pointLight = std::make_shared<PointLight>("Point Light " + std::to_string(engine->GetNPointLights()),
												   glm::vec3{ 0.1f }, // ambient color (default)
												   glm::vec3{ 0.5f, 0.5f, 0.9f }, // diffuse color (bright blue)
												   glm::vec3{ 1.0f }, // specular color (default)
												   glm::vec3{ 1.8f, 1.8f, 4.5f } // position
	);
	// get gizmo's shared_ptr from the point light before adding the latter to the engine (as it will be moved)
	auto pointLightGizmo = pointLight->GetGizmoShape();
	engine->AddAsset(std::move(pointLight)); // add the point light to the engine
	// concatenate the name of the point light and " (Shape n)", 
	// where n is the current number of shapes in the scene
	pointLightGizmo->SetName(pointLightGizmo->GetName() + " (Shape " + nShapes + ")");
	engine->AddAsset(std::move(pointLightGizmo)); // add the point light gizmo (a decahedron) to the engine

	// update the number of shapes in the scene 
	// (which should be 1 at this point, as one shape has been added - the first light gizmo -)
	nShapes = std::to_string(engine->GetNShapes());
	// create a spotlight object with a name "Spotlight n",
	// where n is the current number of spotlights in the scene
	auto spotlight = std::make_shared<Spotlight>("Spotlight " + std::to_string(engine->GetNSpotlights()),
												 glm::vec3{ 0.1f }, // ambient color (default)
												 glm::vec3{ 0.9f, 0.5f, 0.5f }, // diffuse color (bright red)
												 glm::vec3{ 1.0f }, // specular color (default)
												 glm::vec3{ 0.0f, 4.5f, 0.0f }, // position (a bit above the origin)
												 glm::vec3{ 0.0f, -1.0f, 0.0f } // direction (directly downward)
	);
	// get gizmo's shared_ptr from the spotlight before adding the latter to the engine (as it will be moved)
	auto spotlightGizmo = spotlight->GetGizmoShape();
	engine->AddAsset(std::move(spotlight)); // add the spotlight to the engine
	// concatenate the name of the spotlight and " (Shape n)",
	// where n is the current number of shapes in the scene
	spotlightGizmo->SetName(spotlightGizmo->GetName() + " (Shape " + nShapes + ")");
	engine->AddAsset(std::move(spotlightGizmo)); // add the spotlight gizmo (a hexagonal pyramid) to the engine

	// update the number of shapes in the scene 
	// (which should be 2 at this point, as two shapes have been added - the lights' gizmos -)
	nShapes = std::to_string(engine->GetNShapes());
	// set the cube's name to "Cube (Shape n)", where n is the current number of shapes in the scene
	std::string cubeName = "Cube (Shape " + nShapes + ")";
	// create a green cube at (0.0f, 0.0f, 0.0f) - default position - using the data from CUBE.h
	auto cubeShape = std::make_shared<Shape>(cubeName, cubeVerticesVec, cubeIndicesVec,
											 glm::vec3{ 0.8f } // albedo color (almost white)
	);
	engine->AddAsset(std::move(cubeShape)); // add the cube shape to the engine

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Cube Shape Shader Program", "Light Gizmo Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/shape.vert.glsl" , "shaders/light_gizmo.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/shape.frag.glsl", "shaders/light_gizmo.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	engine->DestroyInstance();

	return 0;
}
