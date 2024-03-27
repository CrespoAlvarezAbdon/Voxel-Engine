#ifndef _VOXELENG_FRAMEBUFFER_
#define _VOXELENG_FRAMEBUFFER_

#include "../texture.h"

// TODO. PONER INLINES.

namespace VoxelEng {

	/**
	* @brief Class used to represent a buffer with size equal to the display screen
	* that holds the color of each of its pixels.
	* NOTE. Each framebuffer has a texture as a component, so be careful about the graphics API's
	* limitations about number of active texture units.
	*/
	class framebuffer {
	
	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		* Initializes the framebuffer with all its internal buffers.
		*/
		framebuffer(unsigned int width, unsigned int height);


		// Observers.


		// Modifiers.

		/**
		* @brief Bind the framebuffer so that all render commands affect its buffer instead.
		* NOTE. For these render commands to properly affect this framebuffer, Framebuffer::readyToBeDrawn() must return
		* true first.
		*/
		void bind();

		/**
		* @brief Unbind the currently bound framebuffer and bind the default framebuffer. The last one is the one that will
		* be used to draw on screen.
		*/
		void unbind();

		/**
		* @brief Returns the framebuffer's color buffer or texture.
		*/
		texture* colorBuffer();


		/*
		PLAN.
		1º. METER FUSTRUM CULLING POR CHUNK (O ALGO PARECIDO PORQUE MINECRAFT ES ESPECIAL Y ESO).
		2º. METER TODOS LOS TIPOS DE LUCES.
		3º. METER FORWARD+.
		*/

		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~framebuffer();

	private:

		unsigned int ID_;
		texture* texture_;
	
	};

}

#endif