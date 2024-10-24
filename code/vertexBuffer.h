/**
* @file vertexBuffer.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex buffer.
* @brief Contains the definition of the vertex buffer object (VBO) used in the engine.
*/
#ifndef _VOXELENG_VERTEXBUFFER_
#define _VOXELENG_VERTEXBUFFER_
#include <vector>
#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Buffer to store vertex's data to send to the GPU.
	*/
	class vertexBuffer {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		vertexBuffer();


		// Modifiers.

		/**
		* @brief Initialize a static geometry buffer with vertex data.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void prepareStatic(const void* data, unsigned int size);

		/**
		* @brief Initialize the vertex buffer empty and prepared for dynamic geometry.
		* This buffer is initialized empty but a size of what it is expected to
		* store is needed.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void prepareDynamic(unsigned int size);

		/**
		* @brief Replaces the vertex's data with new one.
		* WARNING. Must be called in a thread with valid graphics API context.
		* This vertex buffer must have been initialized as a dynamic geometry
		* vertex buffer for this function to work as expected.
		*/
		void replaceDynamicData(const void* data, unsigned int size);

		/**
		* @brief Bind the vertex buffer for the next draw call.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void bind() const;
	
		/**
		* @brief Unbind the vertex buffer.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void unbind() const;


		// Destructors.

		/**
		* @brief Class destructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		~vertexBuffer();

	private:

		GLuint rendererID_;

	};

}

#endif