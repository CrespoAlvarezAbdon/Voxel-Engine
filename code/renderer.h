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
#include "indexBuffer.h"


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

		// Misc methods.

		/**
		* @brief Performs a draw call using the indexBuffer object.
		* WARNING. Must be called in a thread with valid graphics context and the index buffer and any associated buffer bust be bound properly
		* before calling this method.
		*/
		void draw3D(const indexBuffer& ib) const;

		/**
		* @brief Draws 'count' model triangles into a 3D space.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void draw3D(int count) const;

		/**
		* @brief Draws 'count' model triangles into a 2D space.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void draw2D(int count) const;

		/**
		* @brief Clears the window associated with the graphics API context
		* in order to prepare it for the next frame that will be drawn.
		* WARNING. Must be called in a thread with valid grahpics API context.
		*/
		void clear() const;

	private:



	};

	inline void renderer::draw3D(const indexBuffer& ib) const {

		glDrawElements(GL_TRIANGLES, ib.nIndices(), GL_UNSIGNED_INT, nullptr);

	}

	inline void renderer::draw3D(int count) const {

		glDrawArrays(GL_TRIANGLES, 0, count);

	}

	inline void renderer::draw2D(int count) const {
		
		// For now it is equal to the draw3D(int count) method
		// but it is left as is in case this separation is needed
		// in the future for whatever reason.

		glDrawArrays(GL_TRIANGLES, 0, count);

	}

	inline void renderer::clear() const {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

}

#endif