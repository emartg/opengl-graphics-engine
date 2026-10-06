/*
 * main.cpp
 * This file is the entry point of the consumer test, a standalone application that uses the Engine
 * as an external dependency (like the X-ray simulator). It follows these steps:
 * - It creates a GLFW_Renderer with its own GUI layer, and initializes the Core engine.
 * - It sets up a minimal scene (a camera).
 * - It renders a fixed number of frames and shuts down the engine.
 */

#include <memory>

#include <imgui.h>

#include "core/Core.h"
#include "core/camera/Camera.h"
#include "core/managers/Node_Manager.h"
#include "core/managers/Scene_Manager.h"

#include "platform/renderer/GLFW_Renderer.h"

// GUI layer of the consumer test, with a single ImGui window
class Consumer_Gui : public Gui_Layer
{
public:
	void draw() override
	{
		// set the position and size explicitly, so that the window is visible from the first frame
		ImGui::SetNextWindowPos(ImVec2{ 20.0f, 20.0f });
		ImGui::SetNextWindowSize(ImVec2{ 360.0f, 80.0f });
		ImGui::Begin("Consumer Test");
		ImGui::TextUnformatted("Rendered with Engine::Platform");
		ImGui::End();
	}
};

int main()
{
	Core* engine = Core::get_instance(); // retrieve the singleton instance of the Core class

	// create a GLFW_Renderer instance with the consumer's GUI, and set it as the renderer for the engine
	GLFW_Renderer* renderer = new GLFW_Renderer();
	renderer->set_gui_layer(std::make_unique<Consumer_Gui>());
	engine->set_renderer(renderer);

	if (!engine->init())
	{ // if the initialization fails, shut down the engine and exit with an error code
		engine->shutdown();
		return -1;
	}

	// create a camera, since the renderer requires one to render the scene
	auto camera = std::make_shared<Camera>("Camera");
	engine->get_scene_manager()->set_camera(camera);
	engine->get_node_manager()->add_node(std::move(camera));

	engine->run(60);    // render 60 frames and exit the main loop
	engine->shutdown(); // clean up resources in the correct order and shut down the engine

	return 0;
}
