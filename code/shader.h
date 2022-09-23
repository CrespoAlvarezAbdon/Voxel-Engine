#ifndef _VOXELENG_SHADER_
#define _VOXELENG_SHADER_
#include <string>
#include <unordered_map>
#include <glm.hpp>
#include "definitions.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/*
	Abstraction of a GLSL shader.
	*/
	class shader {

	public:

		// Constructors.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

		// Observers.


		// Modifiers.


		/*
		Binds the shader to the corresponding OpenGL context inside the thread 
		from which this method was called.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void bind() const;

		/*
		Unbinds the shader from the corresponding OpenGL context inside the thread
		from which this method was called.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void unbind() const;

		/*
		Sets the value of an integer variable named 'name' in the GLSL program.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void setUniform1i(const std::string& name, int i1);

		/*
		Sets a vector of integers named 'name' in the GLSL program
		of size 'vSize'.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void setUniform1iv(const std::string& name, const int * v, int vSize);

		/*
		Sets a vector of 4 floats named 'name' in the GLSL program.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void setUniformVec4f(const std::string& name, float f1, float f2, float f3, float f4);

		/*
		Sets a vector of 3 floats named 'name' in the GLSL program.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void setUniformVec3f(const std::string& name, const vec3& vec);

		/*
		Sets an uniform matrix named 'name' of 4x4 floats in the GLSL program.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void setUniformMatrix4f(const std::string& name, const glm::mat4& matrix);


		// Destructors.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		~shader();

	private:

		unsigned int rendererID_;
		mutable std::unordered_map<std::string, GLint> uniformLocationCache_;

		/*
		Compile a GLSL shader.
		WARNING. Must be called in a thread with valid OpenGL context and return the shader's ID.
		*/
		unsigned int compileShader(const std::string& shaderSource, unsigned int type);

		/*
		Actually creates the shader. It's called as the last instruction of the constructor,
		when the shader files have been already loaded from disk and return the shader's ID.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		unsigned int createShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

		/*
		Get the location of a uniform in the GLSL program named 'name'.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		GLint getUniformLocation(const std::string& name) const;

	};

}

#endif