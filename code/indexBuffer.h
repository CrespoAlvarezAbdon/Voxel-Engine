/**
* @file indexBuffer.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex index buffer.
* @brief Contains the definition of the vertex index buffer class, which
* is used to hold the indices to some vertices in order to reusem them
* and avoid sending duplicated vertices to the GPU.
* It is currently unused in the rest of the engine's code, but it was
* included in case the user needs it.
*/
#ifndef _VOXELENG_INDEXBUFFER_
#define _VOXELENG_INDEXBUFFER_


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Abstraction of an IBO (index buffer object) that holds indices to vertices.
	*/
	class indexBuffer {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		indexBuffer();


		// Observers.

		/**
		* @brief Returns the number of indices stored.
		*/
		unsigned int nIndices() const;


		// Modifiers.

		/**
		* @brief Supply the buffer with vertex data (static geometry).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void prepareStatic(const unsigned int* data, unsigned int indicesCount);

		/**
		* @brief Bind the IBO to the corresponding graphics API context of the thread from
		* which a call to this method was made.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void bind() const;

		/**
		* @brief Unbind the IBO to the corresponding graphics API context of the thread from
		* which a call to this method was made.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void unbind() const;


		// Destructors.

		/**
		* @brief Class destructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		~indexBuffer();

	private:

		unsigned int rendererID_;
		unsigned int nIndices_;

	};

	inline unsigned int indexBuffer::nIndices() const {

		return nIndices_; 

	}

}

#endif