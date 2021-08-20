#include <iostream>
#include <ostream>
#include <iterator>
#include <fstream>
#include <ios>
#include <GL/glew.h>
#include "shader.h"

unsigned int Shader::compile_shader(const string& shader_source, unsigned int type)
{

    unsigned int shader_id = glCreateShader(type);
    const char* source_pointer = shader_source.c_str();

    glShaderSource(shader_id, 1, &source_pointer, nullptr);
    glCompileShader(shader_id);

    int compile_result;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result)
    {

        // Errors were detected while compiling the shader
        int length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char* compile_log = new char[length];
        glGetShaderInfoLog(shader_id, length, &length, compile_log);

        if (type == GL_VERTEX_SHADER)
            cout << "ERRORS WHILE COMPILING VERTEX SHADER:\n";
        else
            cout << "ERRORS WHILE COMPILING FRAGMENT SHADER:\n";
        cout << compile_log << endl;

        delete[] compile_log;

    }

    return shader_id;

}

// creates a shader and returns the unique identifier of the shader created
unsigned int Shader::create_shader(const string& vertex_shader, const string& fragment_shader) // these two strings are the source code of the vertex shader and the fragment shader
{

    unsigned int shader_program = glCreateProgram();
    // Compile shaders
    unsigned int vertex_shader_obj = compile_shader(vertex_shader, GL_VERTEX_SHADER);
    unsigned int fragment_shader_obj = compile_shader(fragment_shader, GL_FRAGMENT_SHADER);

    // Attach shaders (like object code in C++ compiling procedure for example)
    glAttachShader(shader_program, vertex_shader_obj);
    glAttachShader(shader_program, fragment_shader_obj);
    glLinkProgram(shader_program); // Linking stage
    glValidateProgram(shader_program); // Validation stage

    glDeleteShader(vertex_shader_obj); // Delete object files (like object code in C++ compiling procedure for example)
    glDeleteShader(fragment_shader_obj);

    return shader_program;

}

Shader::Shader(const string& vertex_shader_path, const string& fragment_shader_path) // The attributes that represent the filepaths are only for debugging purposes
	: vertex_shader_path_(vertex_shader_path), fragment_shader_path_(fragment_shader_path), renderer_ID_(0)
{

    // Load the shaders from the files and compile them
    ifstream vertex_shader_file(vertex_shader_path);
    noskipws(vertex_shader_file); // Don't skip whitespaces (they have rights too you meanie person! D:)
    istream_iterator<char> vertex_It(vertex_shader_file);

    ifstream fragment_shader_file(fragment_shader_path);
    noskipws(fragment_shader_file);
    istream_iterator<char> fragment_It(fragment_shader_file);

    istream_iterator<char> end;
    string vertex_shader(vertex_It, end);
    string fragment_shader(fragment_It, end);

    vertex_shader_file.close();
    fragment_shader_file.close();

    renderer_ID_ = create_shader(vertex_shader, fragment_shader); // Shader compilation here

}

void Shader::bind() const
{

    glUseProgram(renderer_ID_);

}

void Shader::unbind() const
{

    glUseProgram(0);

}

GLint Shader::get_uniform_location(const string& name) const
{

    if (uniform_location_cache.find(name) != uniform_location_cache.end()) // We use a cache to prevent getting the location N times, which is costly
        return uniform_location_cache[name];

    int location = glGetUniformLocation(renderer_ID_, name.c_str());

    if (location == -1)
        cout << "ERROR: Uniform " << name << " doesn't exist\n";

    uniform_location_cache[name] = location;

    return location;

}

void Shader::set_uniform_1i(const string& name, int i1)
{

    glUniform1i(get_uniform_location(name), i1);

}

void Shader::set_uniform_1iv(const string& name, const int* v, int v_size)
{

    glUniform1iv(get_uniform_location(name), v_size, v);

}

void Shader::set_uniform_4f(const string& name, float f1, float f2, float f3, float f4)
{

    glUniform4f(get_uniform_location(name), f1, f2, f3, f4);

}

void Shader::set_uniform_matrix4f(const string& name, const glm::mat4& matrix)
{

    glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]); // The third parameter means that it isn't necessary to transpose the matrix since glm
                                                                                // creates matrix that OpenGL don't need to transpose in order to understand them
}

Shader::~Shader()
{

    glDeleteProgram(renderer_ID_);

}