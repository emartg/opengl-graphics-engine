/*
 * main.cpp
 * This file is the entry point of the App module. It serves as a simple test
 * of the Core engine. It follows these steps:
 * - It initializes the Core engine, which sets up OpenGL, window, and GUI, and
 * loads its shaders and materials.
 * - It sets up the scene chosen in the command line (see scenes/Scenes.h).
 * - It runs the main loop of the engine, which renders the scene and handles
 * events.
 * - It cleans up resources in the correct order and shuts down the engine.
 * Other important notes:
 * - GLFW_Renderer is used as the renderer implementation for the Core engine.
 * - The application uses the Core library to manage nodes, input, and scene
 * management.
 * Command-line options:
 * - --scene NAME: load the scene with the given name (see --list-scenes; "example" by default).
 * - --objects N: number of objects of the scenes that allow choosing it (e.g., the stress test scenes).
 * - --list-scenes: print the available scenes and exit.
 * - --frames N: render N frames and exit (e.g., for automated smoke tests).
 * - --help: print the usage and exit.
 */

#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <string_view>

#include "gui/Gui.h"
#include "scenes/Scenes.h"

#include "core/Core.h"

#include "platform/renderer/GLFW_Renderer.h"

// Command-line options of the application
struct App_Options
{
	std::string_view scene_name{ "example" }; // name of the scene to load
	int              object_count{ 0 };       // number of objects of the scene (0 means the scene's default)
	std::uint64_t    max_frames{ 0 };         // number of frames to render before exiting (0 means no limit)
	bool             list_scenes{ false };    // whether the list of scenes was requested
	bool             show_help{ false };      // whether the usage was requested
};

// Parses the command-line arguments into the given options.
// Returns true if the arguments are valid, false otherwise (with error messages)
bool parse_command_line(int argc, char** argv, App_Options& options);

// Prints the command-line usage of the application
void print_usage(const char* program_name);

// Prints the scenes of the App, with their names and descriptions
void print_scenes();

// Initializes the Core engine (which also loads its shaders and materials) and
// sets up the renderer. If interactive is true, waits for the user to press
// Enter before exiting when the initialization fails.
// Returns true if initialization was successful, false otherwise (with error messages)
bool initialize_core(Core* engine, bool interactive);

int main(int argc, char** argv)
{
	// parse the command-line options
	App_Options options;
	if (!parse_command_line(argc, argv, options))
	{ // if the arguments are not valid, print the usage and exit with an error code
		print_usage(argv[0]);
		return -1;
	}
	if (options.show_help)
	{ // if the usage was requested, print it and exit
		print_usage(argv[0]);
		return 0;
	}
	if (options.list_scenes)
	{ // if the list of scenes was requested, print it and exit
		print_scenes();
		return 0;
	}

	// find the scene to load before initializing the engine, so that an unknown name fails without opening a window
	const Scene_Info* scene = find_scene(options.scene_name);
	if (!scene)
	{ // if there is no scene with that name, print the available scenes and exit with an error code
		std::cerr << "[ERROR::main] Unknown scene: '" << options.scene_name << "'" << std::endl;
		print_scenes();
		return -1;
	}

	std::cout << "[INFO::main] Starting the application..." << std::endl;

	Core* engine = Core::get_instance(); // retrieve the singleton instance of
										 // the Core class

	// the application is interactive unless it renders a fixed number of frames (e.g., automated tests)
	const bool interactive = options.max_frames == 0;
	if (!initialize_core(engine, interactive))
	{ // if initialization failed, exit with an error code
		return -1;
	}

	// set up the chosen scene, with the chosen number of objects or the scene's default
	if (options.object_count > 0 && scene->default_object_count == 0)
		std::cerr << "[WARNING::main] The scene '" << scene->name << "' has a fixed number of objects, so --objects is ignored"
				  << std::endl;
	const int object_count = options.object_count > 0 ? options.object_count : scene->default_object_count;
	std::cout << "[INFO::main] Loading the scene '" << scene->name << "'";
	if (scene->default_object_count > 0)
		std::cout << " with " << object_count << " objects";
	std::cout << std::endl;
	scene->setup(engine, object_count);

	engine->run(options.max_frames); // run the main loop of the engine, which will render the
									 // scene and handle events

	engine->shutdown(); // clean up resources in the correct order and shut down
						// the engine

	std::cout << "[SUCCESS::main] Application finished successfully" << std::endl;

	return 0;
}

bool parse_command_line(int argc, char** argv, App_Options& options)
{
	// reads the value of an option, given as the next argument (e.g., "--frames 120") or in the same argument
	// (e.g., "--frames=120"). Returns false (with an error message) if the value is missing
	const auto read_value = [argc, argv](int& i, std::string_view argument, std::string_view option, std::string_view& value) {
		if (argument.size() > option.size())
		{ // the value is in the same argument, after the '='
			value = argument.substr(option.size() + 1);
			return true;
		}
		if (i + 1 >= argc)
		{ // if there is no next argument, print an error message and return false
			std::cerr << "[ERROR::main::parse_command_line] Missing value for " << option << std::endl;
			return false;
		}
		value = argv[++i];
		return true;
	};

	// converts a value to a positive integer no greater than the given maximum (the whole value must be a number).
	// Returns false (with an error message) if the value is not valid
	const auto to_positive_integer = [](std::string_view value, std::string_view option, std::uint64_t maximum, std::uint64_t& number) {
		auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), number);
		if (error != std::errc{} || end != value.data() + value.size() || number == 0 || number > maximum)
		{
			std::cerr << "[ERROR::main::parse_command_line] Invalid value for " << option << ": '" << value
					  << "' (expected a positive integer";
			if (maximum < std::numeric_limits<std::uint64_t>::max())
				std::cerr << " no greater than " << maximum;
			std::cerr << ")" << std::endl;
			return false;
		}
		return true;
	};

	// returns true if the argument is the given option, alone or followed by '=' and its value
	const auto is_option = [](std::string_view argument, std::string_view option) {
		return argument == option || (argument.starts_with(option) && argument.size() > option.size() && argument[option.size()] == '=');
	};

	for (int i{ 1 }; i < argc; i++)
	{ // iterate through the arguments (the first one is the program name)
		const std::string_view argument{ argv[i] };
		std::string_view       value;

		if (argument == "--help" || argument == "-h")
		{ // usage requested
			options.show_help = true;
		}
		else if (argument == "--list-scenes")
		{ // list of scenes requested
			options.list_scenes = true;
		}
		else if (is_option(argument, "--scene"))
		{ // name of the scene to load (validated by main, which knows the scenes)
			if (!read_value(i, argument, "--scene", value))
				return false;
			options.scene_name = value;
		}
		else if (is_option(argument, "--objects"))
		{ // number of objects of the scene
			std::uint64_t object_count{ 0 };
			if (!read_value(i, argument, "--objects", value) ||
				!to_positive_integer(value, "--objects", std::numeric_limits<int>::max(), object_count))
				return false;
			options.object_count = static_cast<int>(object_count);
		}
		else if (is_option(argument, "--frames"))
		{ // number of frames to render before exiting
			if (!read_value(i, argument, "--frames", value) ||
				!to_positive_integer(value, "--frames", std::numeric_limits<std::uint64_t>::max(), options.max_frames))
				return false;
		}
		else
		{ // if the argument is unknown, print an error message and return false
			std::cerr << "[ERROR::main::parse_command_line] Unknown argument: " << argument << std::endl;
			return false;
		}
	}

	return true;
}

void print_usage(const char* program_name)
{
	std::cout << "Usage: " << program_name << " [options]\n"
			  << "Options:\n"
			  << "  --scene NAME, --scene=NAME      Load the scene with the given name (\"example\" by default)\n"
			  << "  --objects N, --objects=N        Number of objects of the scenes that allow choosing it (e.g., stress tests)\n"
			  << "  --list-scenes                   Print the available scenes and exit\n"
			  << "  --frames N, --frames=N          Render N frames and exit (e.g., for automated smoke tests)\n"
			  << "  -h, --help                      Print this help and exit" << std::endl;
}

void print_scenes()
{
	std::cout << "Scenes:\n";
	for (const auto& scene : get_scenes())
	{
		std::cout << "  " << scene.name << ": " << scene.description;
		if (scene.default_object_count > 0)
			std::cout << " (" << scene.default_object_count << " objects by default)";
		std::cout << "\n";
	}
	std::cout << std::flush;
}

bool initialize_core(Core* engine, bool interactive)
{
	// create a GLFW_Renderer instance with the application's GUI, and set it as the renderer for the engine
	GLFW_Renderer* renderer = new GLFW_Renderer();
	renderer->set_gui_layer(std::make_unique<GUI>());
	engine->set_renderer(renderer);

	if (engine->init()) // initialize the engine (OpenGL, window, GUI, etc.)
	{                   // if the initialization is successful, print a success message
		std::cout << "[SUCCESS::main] Core initialized successfully" << std::endl;
	}
	else
	{
		// if the initialization fails, print an error messages and shut down
		// the engine, then prompt the user to exit
		std::cerr << "[ERROR::main] Failed to initialize the Core engine" << std::endl;
		engine->shutdown();
		if (interactive)
		{ // wait until the user presses a key before exiting
			std::cout << "[INFO::main] Enter any key and press Enter to exit" << std::endl;
			std::cin.get();
		}
		return false; // indicate that the program should exit with an error
					  // code
	}

	return true; // indicate that the initialization was successful
}
