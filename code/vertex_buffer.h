#ifndef _VERTEXBUFFER_
#define _VERTEXBUFFER_

#include <vector>
#include <GL/glew.h>
using namespace std;

///////////
//Classes//
///////////

/*
Buffer to store all the vertex's data to send to the GPU for a draw call.
*/
class vertexBuffer
{

public:

	// Constructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	vertexBuffer();


	// Observers.


	// Modifiers.


	/*
	Supply the buffer with vertex data (static geometry).
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void prepareStatic(const void* data, unsigned int size);

	/*
	Supply the buffer with vertex data (dynamic geometry).
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void prepareDynamic(unsigned int size);

	/*
	Bind the vertex buffer for the next draw call.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void bind() const;
	
	/*
	Unbind the vertex buffer.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void unbind() const;


	// Destructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	~vertexBuffer();

private:

	GLuint rendererID_;

};


/*
Provides VBOs for objects that are going to be rendered like
chunks or entities.
*/
class vertexBufferProvider 
{

public:

	// Constructors.

	vertexBufferProvider();


	// Observers.


	// Modifiers.


	/*
	Request a VBO (vertex buffer object) from the vertexBuffer class to
	be used for rendering.
	Once all VBOs in this vertexBufferProvider have served their purpose, vertexBufferProvider::freeVBOs() should
	be called to make sure that those VBOs can be reused later.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	vertexBuffer* requestVBO();

	/*
	All VBOs stored in this vertexBufferProvider object are declared as free. Thus they
	can be reused when a call to vertexBufferProvider::requestVBO() is made.
	*/
	void freeVBOs();


	// Destructors.

private:

	/*
	Points to the first available VBO in vbos_.
	All VBOs that are before vbos_[vboIndex_] in the vector are in use.
	*/
	unsigned int vboIndex_; 
	vector<vertexBuffer*> vbos_;

};

inline void vertexBufferProvider::freeVBOs()
{

	vboIndex_ = 0;

}

#endif