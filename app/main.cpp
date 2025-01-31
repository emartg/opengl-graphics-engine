/*
* main.cpp
* This file is the entry point of the application.
* It creates an Engine object, initializes OpenGL, creates a camera, and adds a cube model to the engine.
* It then runs the main loop of the engine.
*/

#include <memory>

#include "CUBE.h"
#include "../engine/Engine.h"


int main(int argc, char** argv)
{
	auto myEngine = std::make_unique<Engine>();
	myEngine->InitOGL();

	auto camera = std::make_unique<Camera>(glm::vec3(4.25f, 2.5f, 4.25f), glm::vec3(0.0f, 1.0f, 0.0f), -135.0f, -24.0f); // hardcoded initial camera settings (the same as the constant values in Engine.h) - provisional
	myEngine->SetCamera(std::move(camera));
	auto lightPos = glm::vec3(-1.0f, 2.0f, 2.0f); // hardcoded initial light position (the same as the constant value in Engine.h) - provisional
	myEngine->SetLightPos(lightPos);

	auto cubeModel = std::make_unique<Shape>(cubeVertices, cubeNormals, cubeTexCoords, cubeIndices);
	//auto cubeModel = std::make_unique<Shape>(cubeInterleavedVertexData, cubeIndices);
	myEngine->AddResource("Main Cube", std::move(cubeModel));

	myEngine->MainLoop();

	return 0;
}
