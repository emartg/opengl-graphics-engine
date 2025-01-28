#include "Engine.h"

#include "CUBE.h"

// Camera settings
glm::vec3 initialCameraPosition{ 4.25f, 2.5f, 4.25f }, initialCameraUp{ 0.0f, 1.0f, 0.0f };
GLfloat initialCameraYaw{ -135.0f }, initialCameraPitch{ -24.0f };

// Lighting settings
glm::vec3 initialLightPos{ glm::vec3(-1.0f, 2.0f, 2.0f) };

int main(int argc, char** argv)
{
	auto myEngine = std::make_unique<Engine>();
	myEngine->InitOGL();

	auto m_camera = std::make_unique<Camera>(initialCameraPosition, initialCameraUp, initialCameraYaw, initialCameraPitch);
	myEngine->SetCamera(std::move(m_camera));
	myEngine->SetLightPos(initialLightPos);

	auto cubeModel = std::make_unique<Shape>(cubeInterleavedVertexData, cubeIndices);
	myEngine->AddResource("Main Cube", std::move(cubeModel));

	myEngine->MainLoop();

	return 0;
}
