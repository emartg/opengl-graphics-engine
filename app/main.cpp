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
	auto engine = std::make_unique<Core>();
	engine->InitOGL();

	auto camera = std::make_unique<Camera>("Main Camera");
	engine->AddCamera(std::move(camera));

	auto pointLight = std::make_unique<PointLight>("Main Light");
	engine->AddLight(std::move(pointLight));

	// create cube models
	auto cube1 = std::make_unique<Shape>("metalPlateCube1", positionsArr, normalsArr, texCoordsArr, nVertices, indicesArr, nIndices);
	auto cube2 = std::make_unique<Shape>("metalPlateCube2", verticesArr, nVertices, indicesArr, nIndices);
	auto cube3 = std::make_unique<Shape>("containerCube1", positionsVec, normalsVec, texCoordsVec, indicesVec);
	auto cube4 = std::make_unique<Shape>("containerCube2", verticesVec, indicesVec);

	// add cube models to the engine
	engine->AddModel(std::move(cube1));
	engine->AddModel(std::move(cube2));
	engine->AddModel(std::move(cube3));
	engine->AddModel(std::move(cube4));

	engine->MainLoop();

	return 0;
}
