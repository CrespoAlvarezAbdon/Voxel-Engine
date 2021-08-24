#ifndef _INDEXBUFFER_
#define _INDEXBUFFER_


///////////
//Classes//
///////////

/*
Abstraction of an IBO (index buffer object).
*/
class indexBuffer
{

public:

	// Constructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	indexBuffer();


	// Observers.

	/*
	Number of indices.
	*/
	unsigned int nIndices() const;


	// Modifiers.


	/*
	Supply the buffer with vertex data (static geometry).
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void prepareStatic(const unsigned int* data, unsigned int indicesCount);

	/*
	Bind the IBO to the corresponding OpenGL context of the thread from
	which a call to this method was made.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void bind() const;

	/*
	Unbind the IBO to the corresponding OpenGL context of the thread from
	which a call to this method was made.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void unbind() const;


	// Destructors.

	~indexBuffer();

private:

	unsigned int rendererID_;
	unsigned int nIndices_;

};

inline unsigned int indexBuffer::nIndices() const
{

	return nIndices_; 

}

#endif