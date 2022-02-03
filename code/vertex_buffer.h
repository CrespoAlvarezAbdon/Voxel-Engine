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
class vertexBuffer {

public:

	// Constructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	vertexBuffer();


	// Observers.


	// Modifiers.


	/*
	Initialize a static geometry buffer with vertex data.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void prepareStatic(const void* data, unsigned int size);

	/*
	Initialize the vertex buffer empty and prepared for dynamic geometry.
	This buffer is initialized empty but a size of what it is expected to
	store is needed.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void prepareDynamic(unsigned int size);

	/*
	Replaces the
	WARNING. Must be called in a thread with valid OpenGL context.
	Also, this vertex buffer must have been initialized as a dynamic geometry
	vertex buffer for this function to work as expected.
	*/
	void replaceDynamicData(const void* data, unsigned int size);

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

#endif