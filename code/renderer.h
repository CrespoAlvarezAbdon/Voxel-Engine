/**
* @file renderer.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Renderer.
* @brief Contains the declaration of the 'renderer' class.
*/
#ifndef _VOXELENG_RENDERER_
#define _VOXELENG_RENDERER_

#include <cstddef>

#include "indexBuffer.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif



namespace VoxelEng {


	////////////
	//Classes.//
	////////////

	/**
	* @brief Responsible of properly issuing the draw calls
	* and clearing the window for the next frame. The vertices
	* to draw must be sent using properly bound and prepared VBOs,
	* along with other data structures that they could need.
	*/
	class renderer {

	public:

		// Modifiers.

		/**
		* @brief Performs a draw call using the indexBuffer object.
		* WARNING. Must be called in a thread with valid graphics context and the index buffer and any associated buffer bust be bound properly
		* before calling this method.
		*/
		static void draw3D(const indexBuffer& ib);

		/**
		* @brief Draws 'count' model triangles into a 3D space.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void draw3D(int count);

		/**
		* @brief Draw 'nPrimitives' primitives, taking into account that the vertex
		* data of said primitives is stored in the same currently bound VBO and that
		* the initial index and the size of each of these primitives are defined, respectively,
		* in the 'indices' and 'sizes' arrays.
		*/
		static void multiDraw3D(int* indices, int* sizes, std::size_t nPrimitives);

		/**
		* @brief Draws 'count' model triangles into a 2D space.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void draw2D(int count);


		// Clean up.

		/**
		* @brief Clears the window associated with the graphics API context
		* in order to prepare it for the next frame that will be drawn.
		* WARNING. Must be called in a thread with valid grahpics API context.
		*/
		static void clearWindow();

	private:


	};

	inline void renderer::draw3D(const indexBuffer& ib) {

		glDrawElements(GL_TRIANGLES, ib.nIndices(), GL_UNSIGNED_INT, nullptr);

	}

	inline void renderer::draw3D(int count) {

		glDrawArrays(GL_TRIANGLES, 0, count);

	}

	inline void renderer::multiDraw3D(int* indices, int* sizes, std::size_t nPrimitives) {

		glMultiDrawArrays(GL_TRIANGLES, indices, sizes, nPrimitives);
	
	}

	inline void renderer::draw2D(int count) {
		
		// For now it is equal to the draw3D(int count) method
		// but it is left as is in case this separation is needed
		// in the future for whatever reason.

		glDrawArrays(GL_TRIANGLES, 0, count);

	}

	inline void renderer::clearWindow() {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

}

#endif