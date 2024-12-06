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
		* @brief Draws model triangles into a 3D space space using the vertices stored in the currently bound VBO
		* from vertex number 0 to vertex number 'count'.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void draw3D(long long count);

		/**
		* @brief Draws model triangles into a 3D space space using the vertices stored in the currently bound VBO
		* from vertex number 'startPos' to vertex number 'endPos'.
		* @param startPos The index in the VBO for the initial vertex to use.
		* @param endPos The index in the VBO for the last vertex to use. The index must be greater than or equal to 'startPos'.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void draw3D(long long startPos, long long endPos);

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

		static void draw3D_(long long startPos, long long endPos);

	};

	inline void renderer::draw3D(const indexBuffer& ib) {

		#if GRAPHICS_API == OPENGL

			glDrawElements(GL_TRIANGLES, ib.nIndices(), GL_UNSIGNED_INT, nullptr);

		#endif

	}

	inline void renderer::draw3D(long long count) {

		draw3D_(0, count);

	}

	inline void renderer::draw3D(long long startPos, long long endPos) {
	
		draw3D_(startPos, endPos);
	
	}

	inline void renderer::multiDraw3D(int* indices, int* sizes, std::size_t nPrimitives) {

		#if GRAPHICS_API == OPENGL

			glMultiDrawArrays(GL_TRIANGLES, indices, sizes, nPrimitives);

		#endif
	
	}

	inline void renderer::draw2D(int count) {
		
		// For now it is equal to the draw3D(int count) method
		// but it is left as is in case this separation is needed
		// in the future for whatever reason.

		#if GRAPHICS_API == OPENGL

			glDrawArrays(GL_TRIANGLES, 0, count);

		#endif

	}

	inline void renderer::clearWindow() {

		#if GRAPHICS_API == OPENGL

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		#endif
	
	}

	inline void renderer::draw3D_(long long startPos, long long endPos) {

		#if GRAPHICS_API == OPENGL

			glDrawArrays(GL_TRIANGLES, startPos, endPos);

		#endif
		
	}

}

#endif