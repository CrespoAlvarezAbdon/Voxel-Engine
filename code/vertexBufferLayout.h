/**
* @file vertexBufferLayout.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex buffer layout.
* @brief Contains the representation of the vertex buffer layout, used
* to tell the GPU the format of the vertex data received during a draw call.
*/
#ifndef _VOXELENG_VERTEXBUFFERLAYOUT_
#define _VOXELENG_VERTEXBUFFERLAYOUT_

#include <vector>
#include "definitions.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

	//////////////
	//Functions.//
	//////////////

	/**
	* @brief Get the size of the graphics API types.
	*/
	unsigned int graphicsAPISizeOf(unsigned int type); 


	////////////////////
	//Classes/Structs.//
	////////////////////

	/**
	* @brief Represents an element of a vertex buffer layout.
	*/
	struct vertexBufferElement {

		unsigned int type;
		unsigned int count;
		bool is_normalized;

	};

	/**
	* @brief Represents a vertex buffer's layout.
	* This layout gives information about
	* how the data sent to the GPU is formed.
	* For example, it could tell that the first 32bits of
	* a vertex's data is a float that represents the position
	* of said vertex in the x-axis.
	*/
	class vertexBufferLayout {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		vertexBufferLayout();


		// Observers.

		/**
		* @brief Returns the number of elements in the layout.
		*/
		const std::vector<vertexBufferElement>& elements() const;

		/**
		* @brief Return's the layout stride, that is, the byte count from the beginning of one vertex to the one of the next vertex.
		*/
		unsigned int stride() const;


		/**
		* @brief Pushes 'count' elements into the vertex buffer layout.
		* Everytime an element is pushed into a layout, its the same
		* as saying that the layout now contains the same data as before
		* this push and the new 'count' elements pushed.
		* This is a template function with no implementation, use the specialized methods.
		*/
		template <typename T>
		void push(unsigned int count);

		/**
		* @brief Now the vertex data will have 'count' more floats at the end.
		* Be aware that vertex data alignment must be of 4 bytes.
		*/
		template <>
		void push<GLfloat>(unsigned int count);

		/**
		* @brief Now the vertex data will have 'count' more unsigned ints at the end.
		* Be aware that vertex data alignment must be of 4 bytes
		*/
		template <>
		void push<unsigned int>(unsigned int count);

		/**
		* @brief Now the vertex data will have 'count' more unsigned ints at the end.
		* Be aware that vertex data alignment must be of 4 bytes
		*/
		template <>
		void push<unsigned char>(unsigned int count);

		/**
		* @brief Now the vertex data will have 'count' more unsigned ints at the end.
		* Be aware that vertex data alignment must be of 4 bytes
		*/
		template <>
		void push<GLbyte>(unsigned int count);

		/**
		* @brief Now the vertex data will have 'count' more unsigned ints at the end.
		* Be aware that vertex data alignment must be of 4 bytes
		*/
		template <>
		void push<normalVec>(unsigned int count);

	private:

		std::vector<vertexBufferElement> elements_;
		unsigned int stride_;

	};

	inline vertexBufferLayout::vertexBufferLayout() : stride_(0) {}

	inline const std::vector<vertexBufferElement>& vertexBufferLayout::elements() const {

		return elements_;

	}

	inline unsigned int vertexBufferLayout::stride() const {

		return stride_;

	}

}

#endif