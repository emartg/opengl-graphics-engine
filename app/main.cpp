/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, sets the light source position,
* and adds models to the engine. It then runs the main loop of the engine that includes input processing.
*/

#include <memory> // for smart pointers

#include "CUBE.h"
#include "DECAHEDRON.h"
#include "../core/Core.h"
#include "renderer/GLFWRenderer.h"

int main(int argc, char** argv)
{
	// create pointer to a renderer object
	Renderer* renderer = new GLFWRenderer();

	// get engine instance, set the renderer, and initialize OpenGL
	auto engine = Core::GetInstance();
	engine->SetRenderer(renderer);
	engine->InitOGL();

	// create camera and light and add them to the engine
	auto camera = std::make_unique<Camera>("Camera 0");
	engine->AddAsset(std::move(camera));
	auto light = std::make_unique<PointLight>("Point Light " + std::to_string(engine->GetNPointLights()));
	std::string lightName = light->GetName(); // get the name of the light to use it when creating its shape
	glm::vec3 lightPos{ light->GetPosition() }; // get the position of the light to position its corresponding shape
	glm::vec3 lightDiffuse{ light->GetDiffuse() }; // get the diffuse color of the light to color its corresponding shape
	engine->AddAsset(std::move(light));

	// create a shape concatenating its name and "(Shape n)" and add it to the engine 
	// (where n is the number of shapes in the scene).
	// In this case, an decahedron is created at the position of the light source using the data from DECAHEDRON.h
	// and gets its color from the light's diffuse component
	auto pointLightShape = std::make_unique<Shape>(lightName + " (Shape " + std::to_string(engine->GetNShapes()) + ")",
												   decahedronVerticesVec, decahedronIndicesVec,
												   lightDiffuse, // set the shape's color to the light's diffuse color
												   lightPos // set the shape's position to the light's position
	);
	engine->AddAsset(std::move(pointLightShape));

	// create a shape concatenating its name and "(Shape n)" and add it to the engine 
	// (where n is the number of shapes in the scene).
	// In this case, a blue cube is created at a position of (0.0f, 0.0f, 0.5f) using the data from CUBE.h
	auto cubeShape = std::make_unique<Shape>("Cube (Shape " + std::to_string(engine->GetNShapes()) + ")",
											 cubeVerticesVec, cubeIndicesVec,
											 glm::vec3{ 0.0f, 0.0f, 0.5f });
	engine->AddAsset(std::move(cubeShape));

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Shape Shader Program", "Point Light Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/shape.vert.glsl" , "shaders/point_light.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/shape.frag.glsl", "shaders/point_light.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	engine->DestroyInstance();

	return 0;
}
