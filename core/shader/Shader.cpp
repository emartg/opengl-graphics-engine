/*
 * Shader.cpp
 * This file implements the Shader class (a derived class of Node),
 * which is used to read, compile, and link shaders to a shader program.
 * It also provides methods to set uniform variables in the shader program.
 */

#include "Shader.h"

// Constructors
// ------------
Shader::Shader(
	const std::string& name,
	const std::string& vertex_path,
	const std::string& fragment_path,
	const GLboolean    deferred_compilation) :
	Node(name, Node_Type::SHADER), // set the node type to SHADER
	shader_program_id{},           // initialize the id to 0
	vertex_shader_path{ vertex_path },
	geometry_shader_path{ "" },
	fragment_shader_path{ fragment_path }
{
	// compile the shader if deferred_compilation is set to false
	if (!deferred_compilation)
		compile();
}

Shader::Shader(
	const std::string& name,
	const std::string& vertex_path,
	const std::string& geometryPath,
	const std::string& fragment_path,
	const GLboolean    deferred_compilation) :
	Node(name, Node_Type::SHADER), // set the node type to SHADER
	shader_program_id{},           // initialize the id to 0
	vertex_shader_path{ vertex_path },
	geometry_shader_path{ geometryPath },
	fragment_shader_path{ fragment_path }
{
	// compile the shader if deferred_compilation is set to false
	if (!deferred_compilation)
		compile();
}

// Public Methods
// --------------
bool Shader::compile()
{
	// load the source code of each stage from its file, resolving the #include directives
	const Shader_Source vertex_shader_source = Shader_Preprocessor::load(vertex_shader_path);
	const Shader_Source geometry_shader_source =
		geometry_shader_path.empty() ? Shader_Source{} : Shader_Preprocessor::load(geometry_shader_path);
	const Shader_Source fragment_shader_source = Shader_Preprocessor::load(fragment_shader_path);
	for (const Shader_Source* source : { &vertex_shader_source, &geometry_shader_source, &fragment_shader_source })
	{
		if (!source->is_valid())
		{ // if a file could not be read, print the error message and return false
			std::cerr << "[ERROR::SHADER::compile] Shader file reading error in '" << name << "': " << source->error << std::endl;
			return false;
		}
	}
	// convert strings to GLchar pointers
	const GLchar* vertex_shader_code   = vertex_shader_source.code.c_str();
	const GLchar* geometry_shader_code = nullptr;
	if (!geometry_shader_path.empty())
		geometry_shader_code = geometry_shader_source.code.c_str();
	const GLchar* fragment_shader_code = fragment_shader_source.code.c_str();

	// compile shaders and create shader program
	GLuint vertex_shader{}, geometry_shader{}, fragment_shader{};
	// vertex shader compilation
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	if (!check_compilation_linking_errors(vertex_shader, "VERTEX", vertex_shader_source.files))
	{ // if compilation of the vertex shader failed, delete it and return false
		glDeleteShader(vertex_shader);
		return false;
	}
	// geometry shader compilation (if provided)
	if (!geometry_shader_path.empty())
	{
		geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry_shader, 1, &geometry_shader_code, NULL);
		glCompileShader(geometry_shader);
		if (!check_compilation_linking_errors(geometry_shader, "GEOMETRY", geometry_shader_source.files))
		{ // if compilation of the geometry shader failed, delete the created shaders and return false
			glDeleteShader(vertex_shader);
			glDeleteShader(geometry_shader);
			return false;
		}
	}
	// fragment shader compilation
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	if (!check_compilation_linking_errors(fragment_shader, "FRAGMENT", fragment_shader_source.files))
	{ // if compilation of the fragment shader failed, delete the created shaders and return false
		glDeleteShader(vertex_shader);
		if (!geometry_shader_path.empty())
			glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);
		return false;
	}
	// shader program
	shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, vertex_shader);
	if (!geometry_shader_path.empty())
		glAttachShader(shader_program_id, geometry_shader);
	glAttachShader(shader_program_id, fragment_shader);
	glLinkProgram(shader_program_id);

	// delete the shaders, as they're either linked into the program or no longer needed
	// (they are only flagged for deletion until the program is deleted)
	glDeleteShader(vertex_shader);
	if (!geometry_shader_path.empty())
		glDeleteShader(geometry_shader);
	glDeleteShader(fragment_shader);

	if (!check_compilation_linking_errors(shader_program_id, "PROGRAM", {}))
	{ // if linking the program failed, delete it and return false
		glDeleteProgram(shader_program_id);
		shader_program_id = 0;
		return false;
	}

	std::cout << "[SUCCESS::SHADER::compile] Shader with name '" << name << "' compiled and linked successfully" << std::endl;
	return true; // return true if compilation and linking were successful
}

void Shader::use() const
{
	glUseProgram(shader_program_id);
}

void Shader::set_bool(const std::string& name, GLboolean value) const
{
	glUniform1i(glGetUniformLocation(shader_program_id, name.c_str()), (GLint)value);
}

void Shader::set_int(const std::string& name, GLint value) const
{
	glUniform1i(glGetUniformLocation(shader_program_id, name.c_str()), value);
}

void Shader::set_float(const std::string& name, GLfloat value) const
{
	glUniform1f(glGetUniformLocation(shader_program_id, name.c_str()), value);
}

void Shader::set_vec2(const std::string& name, const glm::vec2& vec) const
{
	glUniform2fv(glGetUniformLocation(shader_program_id, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::set_vec3(const std::string& name, const glm::vec3& vec) const
{
	glUniform3fv(glGetUniformLocation(shader_program_id, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::set_vec4(const std::string& name, const glm::vec4& vec) const
{
	glUniform4fv(glGetUniformLocation(shader_program_id, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::set_mat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(shader_program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(shader_program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

// Private Methods
// ---------------
bool Shader::check_compilation_linking_errors(GLuint shader, std::string type, const std::vector<std::filesystem::path>& files) const
{
	GLint  success;        // variable to store the success status of the compilation/linking process
	GLchar info_log[1024]; // buffer to store the information log in case of errors

	if (type != "PROGRAM")
	{ // if the type is not PROGRAM, it means we are checking a shader, so we check for compilation errors
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{ // if compilation failed, retrieve the info log, print it, and return false
			glGetShaderInfoLog(shader, 1024, NULL, info_log);
			std::cerr << "[ERROR::SHADER::compile] Shader compilation error of type: " << type << "\n" << info_log << std::endl;
			// list the files of the source, since the messages identify them by their source string number
			// (e.g., "0:12" or "0(12)" is line 12 of file 0, depending on the driver)
			if (files.size() > 1)
			{
				std::cerr << "Source files:" << std::endl;
				for (std::size_t i = 0; i < files.size(); ++i) std::cerr << "  " << i << ": " << files[i].string() << std::endl;
			}
			return false;
		}
	}
	else
	{ // if the type is PROGRAM, it means we are checking a program, so we check for linking errors
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{ // if linking the program failed, retrieve the info log, print it, and return false
			glGetProgramInfoLog(shader, 1024, NULL, info_log);
			std::cerr << "[ERROR::SHADER::compile] Shader linking error of type: " << type << "\n" << info_log << std::endl;
			return false;
		}
	}

	return true; // return true if no errors occurred during the compilation/linking process of the shader
}
