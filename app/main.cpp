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

	// create cube models and add them to the engine
	auto cube = std::make_unique<Shape>("Main Cube", verticesVec, indicesVec);
	engine->AddAsset(std::move(cube));

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Shape Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/shape.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/shape.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// run the main loop of the engine
	engine->MainLoop();

	return 0;
}
