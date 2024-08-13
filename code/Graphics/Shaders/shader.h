/**
* @file shader.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Shader.
* @brief Contains the declaration of the 'shader' class.
*/
#ifndef _VOXELENG_SHADER_
#define _VOXELENG_SHADER_

#include <string>
#include <unordered_map>
#include "../../definitions.h"
#include "../../vec.h"
#include "../UBOs/UBOs.h"
#include "../Materials/materials.h"

namespace VoxelEng {

	/**
	* @brief Abstraction of a graphics API shader. It is usually described as a program
	* executed by the graphics API in order to properly process the vertex data
	* that is sent to the GPU.
	*/
	class shader {

	public:

		// Constructors.

		/**
		* @brief Class constructor. The engine only accepts, for now, a combination of
		* vertex shader and fragment shader in order to process the vertex data.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);


		// Modifiers.

		/**
		* @brief Binds the shader to the corresponding graphics API context inside the thread 
		* from which this method was called.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void bind() const;

		/**
		* @brief Unbinds the shader from the corresponding graphics API context inside the thread
		* from which this method was called.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void unbind() const;

		/**
		* @brief Sets the value of a previously existing integer variable named 'name' in the shader program.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void setUniform1i(const std::string& name, int i1);

		/**
		* @brief Sets a previously existing vector of integers named 'name' in the shader program
		* of size 'vSize'.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void setUniform1iv(const std::string& name, const int * v, int vSize);

		/**
		* @brief Sets a previously existing vector of 4 floats named 'name' in the shader program.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void setUniformVec4f(const std::string& name, float f1, float f2, float f3, float f4);

		/**
		* @brief Sets a previously existing vector of 3 floats named 'name' in the shader program.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void setUniformVec3f(const std::string& name, const vec3& vec);

		/**
		* @brief Sets a previously existing uniform matrix named 'name' of 4x4 floats in the shader program.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void setUniformMatrix4f(const std::string& name, const glm::mat4& matrix);

		/**
		* @brief Bind the specified materials UFO for this shader. 
		* WARNING. The name of this UFO must be defined as a valid Uniform Block in the shader's code
		* or else an exception will be thrown.
		* WARNING. An exception will be also thrown if the provided UFO is not properly initialised.
		* @param ufo The specified materials UFO.
		*/
		void bindUFO(const UBO<material>& ufo); // NEXT. DO THIS AND THE FUNCTIONS THAT ARE PENDING TO BE IMPLEMENTED IN UBO.h


		// Destructors.

		/**
		* @brief Class destructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		~shader();

	private:

		unsigned int rendererID_;
		mutable std::unordered_map<std::string, GLint> uniformLocationCache_;

		/*
		Compile a shader shader.
		WARNING. Must be called in a thread with valid graphics API context and return the shader's ID.
		*/
		unsigned int compileShader(const std::string& shaderSource, unsigned int type);

		/*
		Actually creates the shader. It's called as the last instruction of the constructor,
		when the shader files have been already loaded from disk and return the shader's ID.
		WARNING. Must be called in a thread with valid graphics API context.
		*/
		unsigned int createShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

		/*
		Get the location of a uniform in the shader program named 'name'.
		WARNING. Must be called in a thread with valid graphics API context.
		*/
		GLint getUniformLocation(const std::string& name) const;

	};

}

#endif