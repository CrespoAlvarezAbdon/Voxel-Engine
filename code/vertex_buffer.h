#ifndef _VERTEXBUFFER_
#define _VERTEXBUFFER_
#include <vector>
#include <GL/glew.h>
using namespace std;


class vertexBuffer
{

public:

	vertexBuffer();

	void prepare_static(const void* data, unsigned int size); // Static geometry
	void prepare_dynamic(unsigned int size); // Dynamic geometry

	void bind() const;
	void unbind() const;

	~vertexBuffer();

private:

	GLuint renderer_ID_;

};


/*
Provides VBOs for objects that are going to be rendered.
*/
class vertexBufferProvider 
{

public:

	vertexBufferProvider();

	vertexBuffer* requestVBO();
	void freeVBOs();

private:

	// Points to the first available VBO in vbos_. 
	// All VBOs that are before vbos_[vboIndex_] in the vector are in use.
	unsigned int vboIndex_; 
	vector<vertexBuffer*> vbos_;

};

/*
All VBOs stored in this object are declared as available.
Make sure that the operations of currently used VBOs 
associated with this object are finished
before calling this method.
*/
inline void vertexBufferProvider::freeVBOs()
{

	vboIndex_ = 0;

}

#endif