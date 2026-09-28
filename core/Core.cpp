/*
 * Core.h
 * This file implments the Core class, which is is responsible for initializing OpenGL,
 * creating a window, and running the main loop.
 * It also manages the various managers used in the engine and holds the renderer instance,
 * and is thus responsible for coordinating their interactions.
 * It is a Singleton class.
 */

#include "Core.h"

#include <iostream>
#include <memory>
#include <algorithm>
#include <array>

#include "shader/Shader.h"
#include "texture/Texture.h"
#include "gizmos/Line.h"               // for directional light gizmo rendering
#include "gizmos/TRIANGLE_FAN_PLANE.h" // for directional light gizmo rendering
#include "camera/Camera.h"
#include "managers/Node_Manager.h"
#include "managers/Input_Manager.h"
#include "managers/Scene_Manager.h"
#include "managers/Selection_Manager.h"
#include "renderer/Renderer.h"
#include "utils/file_system/File_System_Utils.h"

// Built-in shader programs required by the renderer
// -------------------------------------------------
namespace
{
    // Names of the shader programs (as known by Renderer::set_shader_by_name) and file names of
    // their stages, relative to the "shaders" subdirectory of the resources directory
    // (an empty geometry file name means that the program has no geometry stage)
    struct Builtin_Shader_Info
    {
        const char* name;
        const char* vertex_file;
        const char* geometry_file;
        const char* fragment_file;
    };
    constexpr std::array<Builtin_Shader_Info, 9> BUILTIN_SHADERS{
        { { "Shape Model Shader", "shape_model.vert.glsl", "", "shape_model.frag.glsl" },
          { "Assimp Model Shader", "assimp_model.vert.glsl", "", "assimp_model.frag.glsl" },
          { "Single Albedo Shader", "single_albedo.vert.glsl", "", "single_albedo.frag.glsl" },
          { "Screen Quad Shader", "screen_quad.vert.glsl", "", "screen_quad.frag.glsl" },
          { "Picking Shader", "picking.vert.glsl", "", "picking.frag.glsl" },
          { "Skybox Shader", "skybox.vert.glsl", "", "skybox.frag.glsl" },
          { "Equirectangular to Cubemap Shader",
            "equirectangular_to_cubemap.vert.glsl",
            "",
            "equirectangular_to_cubemap.frag.glsl" },
          { "Reflective Shader", "reflective.vert.glsl", "", "reflective.frag.glsl" },
          { "Refractive Shader", "refractive.vert.glsl", "", "refractive.frag.glsl" } }
    };
}

// Static Instance initialization
// ------------------------------
Core* Core::instance{ nullptr };

// Constructors
// ------------
Core::Core() :
    renderer{ nullptr },
    node_manager{ std::make_shared<Node_Manager>() },
    input_manager{ std::make_shared<Input_Manager>() },
    scene_manager{ std::make_shared<Scene_Manager>() },
    selection_manager{ std::make_shared<Selection_Manager>() }
{}

// Destructor
// ----------
Core::~Core()
{
    std::cout << "[INFO::CORE::~Core] Core destructor called" << std::endl;
}

// Public Static Methods
// ---------------------
Core* Core::get_instance()
{
    if (!instance)
    { // if the Core instance is null, create a new instance
        std::cout << "[INFO::CORE::get_instance] Creating Core instance..." << std::endl;
        instance = new Core();
    }
    return instance;
}

// Private Static Methods
// ----------------------
void Core::destroy_instance()
{
    if (instance)
    { // check if the Core instance is not null before destroying it
        std::cout << "[INFO::CORE::destroy_instance] Destroying Core instance..." << std::endl;
        delete instance;    // destroy the Core instance
        instance = nullptr; // nullify the pointer to avoid dangling pointer issues
    }
}

// Private Methods
// ---------------
bool Core::resolve_resources_dir()
{
    if (!resources_dir.empty())
    { // if the directory was set explicitly, only verify that it exists
        std::error_code error;
        if (!std::filesystem::is_directory(resources_dir, error))
        { // if it doesn't exist, print an error message and return false
            std::cerr << "[ERROR::CORE::resolve_resources_dir] Resources directory not found: "
                      << resources_dir.string() << std::endl;
            return false;
        }
        resources_dir = std::filesystem::absolute(resources_dir, error);
    }
    else
    { // otherwise, search the candidate locations in order of precedence
        std::filesystem::path executable_dir = File_System_Utils::get_executable_directory();
        std::error_code error;
        std::filesystem::path current_dir = std::filesystem::current_path(error);

        std::vector<std::filesystem::path> candidates;
        if (!executable_dir.empty())
        {
            // resources staged beside the executable (e.g., a packaged build)
            candidates.push_back(executable_dir / "resources");
            // installation layout (<prefix>/bin/<executable> and <prefix>/<data dir>/.../resources)
            candidates.push_back(executable_dir.parent_path() / ENGINE_INSTALL_RESOURCES_DIR);
        }
        if (!current_dir.empty())
        { // resources in the current working directory
            candidates.push_back(current_dir / "resources");
        }
        // resources in the Engine's source tree (development builds, where they are not copied)
        candidates.push_back(ENGINE_SOURCE_RESOURCES_DIR);

        resources_dir = File_System_Utils::find_first_existing_directory(candidates);
        if (resources_dir.empty())
        { // if none of the candidates exists, print the searched locations and return false
            std::cerr << "[ERROR::CORE::resolve_resources_dir] Resources directory not found. Searched in:"
                      << std::endl;
            for (const auto& candidate : candidates) std::cerr << "  " << candidate.string() << std::endl;
            return false;
        }
    }

    std::cout << "[INFO::CORE::resolve_resources_dir] Using resources directory: " << resources_dir.string()
              << std::endl;
    return true;
}

bool Core::compile_builtin_shaders()
{
    // build the names and paths of the built-in shader programs
    std::vector<std::string> shader_names, vertex_shader_paths, geometry_shader_paths, fragment_shader_paths;
    for (const auto& shader_info : BUILTIN_SHADERS)
    {
        shader_names.emplace_back(shader_info.name);
        vertex_shader_paths.push_back(get_resource_path(std::string("shaders/") + shader_info.vertex_file));
        geometry_shader_paths.push_back(
            *shader_info.geometry_file == '\0'
                ? std::string{}
                : get_resource_path(std::string("shaders/") + shader_info.geometry_file));
        fragment_shader_paths.push_back(get_resource_path(std::string("shaders/") + shader_info.fragment_file));
    }

    return compile_shaders(shader_names, vertex_shader_paths, geometry_shader_paths, fragment_shader_paths);
}

// Public Methods
// --------------
std::string Core::get_resource_path(const std::string& relative_path) const
{
    return (resources_dir / relative_path).string();
}

bool Core::init()
{
    // use the current time as seed for the random number generator
    // (to get different positions, colors, etc. each run)
    srand(static_cast<unsigned int>(time(0)));

    // locate the resources directory before anything is loaded from it
    if (!resolve_resources_dir())
    { // if the resources directory is not found, print an error message and return false
        std::cerr << "[ERROR::CORE::init] Failed to locate the resources directory" << std::endl;
        return false;
    }

    // initialize the renderer
    if (!renderer->init())
    { // if the renderer initialization fails, print an error message and return false
        std::cerr << "[ERROR::CORE::init] Failed to initialize renderer" << std::endl;
        return false;
    }

    // create a window with the specified width, height, and title; and configure it
    renderer->create_window(screen_width, screen_height, "Test Window");
    renderer->configure_window();

    // set callback functions
    renderer->set_callback_functions();

    // load all OpenGL function pointers with GLAD
    if (!gladLoadGLLoader(renderer->get_proc_address()))
    { // if GLAD fails to load OpenGL functions, print an error message and return false
        std::cerr << "[ERROR::CORE::init] Failed to initialize GLAD" << std::endl;
        return false;
    }
    // now the OpenGL context is set up, and we can use OpenGL functions

    // set the viewport to the window size
    renderer->set_viewport(screen_width, screen_height);

    // initialize selection manager picking FBO with current window size
    if (selection_manager)
        selection_manager->resize(screen_width, screen_height);
    // configure OpenGL global state
    renderer->config_opengl();
    // initialize the user interface
    renderer->init_gui();

    // compile the built-in shaders required by the renderer
    if (!compile_builtin_shaders())
    { // if the built-in shaders fail to compile, print an error message and return false
        std::cerr << "[ERROR::CORE::init] Failed to compile the built-in shaders" << std::endl;
        return false;
    }

    // if all the initializations are successful, print a success message and return true
    std::cout << "[SUCCESS::CORE::init] Core initialized successfully" << std::endl;
    return true;
}

void Core::run()
{
    std::cout << "[INFO::CORE::run] Starting main loop..." << std::endl;
    while (!renderer->should_close())
    {
        // always wait for events first: the renderer fully blocks until an event occurs,
        // and when that happens, the renderer processes frames but throttles the frame rate,
        // reducing CPU / GPU usage and improving performance
        renderer->wait_for_events();

        renderer->frame_start_config(); // start of the frame configuration
        renderer->render_scene();       // composite the scene
        renderer->render_gui();         // render the GUI
        renderer->frame_end_config();   // end of the frame configuration
    }
}

void Core::shutdown()
{
    if (renderer)
    { // check if the renderer is not null before shutting it down
        std::cout << "[INFO::CORE::shutdown] Destroying renderer..." << std::endl;
        renderer->shutdown_gui(); // the GUI must be shut down before the renderer
        delete renderer;          // destroy the renderer before the Core instance
        renderer = nullptr;       // nullify the pointer to avoid dangling pointer issues
    }
    else
    { // if the renderer is null, print an error message and return
        std::cerr << "[INFO::CORE::shutdown] Renderer is null during Core destruction!" << std::endl;
        return;
    }

    // destroy the Core instance itself and print a message to the console
    destroy_instance();
    std::cout << "[INFO::CORE::shutdown] Core shutdown complete" << std::endl;
}

bool Core::compile_shaders(
    const std::vector<std::string>& shader_names,
    const std::vector<std::string>& vertex_shader_paths,
    const std::vector<std::string>& fragment_shader_paths)
{
    // delegate to the general overload with no geometry stage for any of the shaders
    return compile_shaders(
        shader_names,
        vertex_shader_paths,
        std::vector<std::string>(shader_names.size()),
        fragment_shader_paths);
}

bool Core::compile_shaders(
    const std::vector<std::string>& shader_names,
    const std::vector<std::string>& vertex_shader_paths,
    const std::vector<std::string>& geometry_shader_paths,
    const std::vector<std::string>& fragment_shader_paths)
{
    size_t shader_count = shader_names.size(); // number of shaders to compile

    // ensure the sizes of the input vectors match
    if (vertex_shader_paths.size() != shader_count || geometry_shader_paths.size() != shader_count ||
        fragment_shader_paths.size() != shader_count)
    { // if the sizes do not match, print an error message and return false
        std::cerr << "[ERROR::CORE::compile_shaders] Mismatched shader names and paths sizes!" << std::endl;
        return false;
    }

    for (size_t i{}; i < shader_count; i++)
    { // iterate through the shader names and paths
        // create a new Shader object with the name and paths (an empty geometry shader path
        // means that the program has no geometry stage), and compile it
        auto shader = std::make_shared<Shader>(
            shader_names[i],
            vertex_shader_paths[i],
            geometry_shader_paths[i],
            fragment_shader_paths[i]);
        if (!shader->compile()) // compile the shader
        {                       // if the shader compilation fails, print an error message and return false
            std::cerr << "[ERROR::CORE::compile_shaders] Failed to compile shader: " << shader_names[i] << std::endl;
            return false;
        }

        // set the shader in the renderer by name
        if (!renderer->set_shader_by_name(shader->get_name(), shader))
        { // if the shader was not set successfully, print an error message and return false
            std::cerr << "[ERROR::CORE::compile_shaders] Failed to set shader with name '" << shader_names[i]
                      << "' in the renderer (unknown name or null shader)" << std::endl;
            return false;
        }
        else
        { // if the shader was set successfully, print a success message
            std::cout << "[SUCCESS::CORE::compile_shaders] Shader with name '" << shader_names[i]
                      << "' set successfully in the renderer" << std::endl;
        }

        // if this is the picking shader, set it in the selection manager
        if (shader_names[i] == "Picking Shader" && selection_manager)
        {
            selection_manager->set_picking_shader(shader);
            std::cout << "[INFO::CORE::compile_shaders] Picking Shader assigned to selection manager" << std::endl;
        }

        // add the compiled shader to the node manager
        node_manager->add_node(std::move(shader));
    }

    // if all shaders are compiled successfully, print a success message and return true
    std::cout << "[SUCCESS::CORE::compile_shaders] Shaders compiled successfully" << std::endl;
    return true;
}

void Core::load_textures(
    const std::vector<std::string>& texture_names,
    const std::vector<std::string>& texture_paths,
    const std::vector<std::string>& texture_types)
{
    for (GLuint i{}; i < texture_names.size(); i++)
    {
        auto texture = std::make_shared<Texture>(texture_names[i], texture_paths[i], Texture_Type::DIFFUSE);
        node_manager->add_node(std::move(texture));
    }
}

void Core::framebuffer_size_callback(GLint width, GLint height)
{
    // keep Core's notion of the default framebuffer size in sync with the actual window size
    screen_width = static_cast<GLuint>(std::max(0, width));
    screen_height = static_cast<GLuint>(std::max(0, height));

    // update the default framebuffer viewport
    if (renderer)
        renderer->set_viewport(screen_width, screen_height);

    // keep picking/outline FBOs in sync with the default framebuffer size (the actual window size)
    if (selection_manager)
        selection_manager->resize(width, height);
}