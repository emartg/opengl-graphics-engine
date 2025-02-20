/*
* main.cpp
* This file is the entry point of the application.
* It creates a Core object, initializes OpenGL, adds a camera, sets the light source position,
* and adds models to the engine. It then runs the main loop of the engine that includes input processing.
*/

#include <memory>

#include "CUBE.h"
#include "../core/Core.h"

int main(int argc, char** argv)
{
	// create engine and initialize OpenGL
	auto engine = std::make_unique<Core>();
	engine->InitOGL();

	// create camera and light and add them to the engine
	auto camera = std::make_unique<Camera>("Main Camera");
	engine->AddAsset(std::move(camera));
	auto light = std::make_unique<PointLight>("Main Light");
	engine->AddAsset(std::move(light));

	// create cube models and add them to the engine
	auto cube1 = std::make_unique<Shape>("Blue Metal Plate Cube 1", positionsArr, normalsArr, texCoordsArr, nVertices, indicesArr, nIndices);
	auto cube2 = std::make_unique<Shape>("Blue Metal Plate Cube 2", verticesArr, nVertices, indicesArr, nIndices);
	auto cube3 = std::make_unique<Shape>("Container Cube 1", positionsVec, normalsVec, texCoordsVec, indicesVec);
	auto cube4 = std::make_unique<Shape>("Container Cube 2", verticesVec, indicesVec);
	engine->AddAsset(std::move(cube1));
	engine->AddAsset(std::move(cube2));
	engine->AddAsset(std::move(cube3));
	engine->AddAsset(std::move(cube4));

	// define shader names and paths and compile the shaders
	std::vector<std::string> shaderNames{ "Shape Shader Program" };
	std::vector<std::string> vertexShaderPaths{ "shaders/shape.vert.glsl" };
	std::vector<std::string> fragmentShaderPaths{ "shaders/shape.frag.glsl" };
	engine->CompileShaders(shaderNames, vertexShaderPaths, fragmentShaderPaths);

	// define texture names, paths, and types and load the textures
	std::vector<std::string> textureNames{ "Blue Metal Plate Diffuse Map", "Blue Metal Plate Specular Map" };
	std::vector<std::string> texturePaths{ "textures/blue_metal_plate_diffuse.jpg", "textures/blue_metal_plate_specular.jpg" };
	std::vector<std::string> textureTypes{ "DIFFUSE", "SPECULAR" };
	engine->LoadTextures(textureNames, texturePaths, textureTypes);

	// run the main loop of the engine
	engine->MainLoop();

	return 0;
}
