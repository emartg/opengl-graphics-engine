/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, sets the light source position,
* and adds models to the engine. It then runs the main loop of the engine that includes input processing.
*/

#include <memory> // for smart pointers

#include "CUBE.h"
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
	auto camera = std::make_unique<Camera>("Main Camera");
	engine->AddAsset(std::move(camera));
	auto light = std::make_unique<PointLight>("Main Light");
	engine->AddAsset(std::move(light));

	// create a blue shape named "Shape n" and add it to the engine (where n is the number of shapes in the scene)
	auto shape = std::make_unique<Shape>("Shape " + std::to_string(engine->GetNShapes()), verticesVec, indicesVec, glm::vec3{ 0.0f, 0.0f, 0.5f });
	engine->AddAsset(std::move(shape));

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Shape Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/shape.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/shape.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	// destroy the engine instance
	Core::DestroyInstance();

	return 0;
}
