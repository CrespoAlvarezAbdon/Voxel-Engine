#ifndef _VERTEXBUFFERLAYOUT_
#define _VERTEXBUFFERLAYOUT_

#include <vector>
#include <GL/glew.h>
using namespace std;


/////////////
//Functions//
/////////////

/* 
Auxiliary function used to get the size of OpenGL types
*/
unsigned int openGLSizeOf(unsigned int type); 


///////////////////
//Classes/Structs//
///////////////////

/*
Represents an element of a vertex buffer.
*/
struct vertexBufferElement
{

	unsigned int type;
	unsigned int count;
	bool is_normalized;

};

/*
Represents a vertex buffer's layout.
This layout gives information about
how the data sent to the GPU is formed.
For example, it could tell that the first 32bits of
a vertex's data is a float that represents the position
of said vertex in the x-axis.
*/
class vertexBufferLayout
{

public:

	// Constructors.

	vertexBufferLayout();


	// Observers.

	const vector<vertexBufferElement>& elements() const noexcept;
	unsigned int stride() const noexcept;


	/*
	Pushes 'count' elements into the vertex buffer layout.
	Everytime an element is pushed into a layout, its the same
	as saying that the layout now contains the same data as before
	this push and the new 'count' elements pushed.
	This is a template function with no implementation, used the specialized methods.
	*/
	template <typename T>
	void push(unsigned int count);

	/*
	Now the vertex data will have 'count' more floats at the end.
	Be aware about OpenGL vertex data alignment.
	*/
	template <>
	void push<GLfloat>(unsigned int count);

	/*
	Now the vertex data will have 'count' more unsigned ints at the end.
	Be aware about OpenGL vertex data alignment.
	*/
	template <>
	void push<unsigned int>(unsigned int count);

	/*
	Now the vertex data will have 'count' more unsigned chars at the end.
	Be aware about OpenGL vertex data alignment.
	*/
	template <>
	void push<unsigned char>(unsigned int count);

	/*
	Now the vertex data will have 'count' more GLbytes at the end.
	Be aware about OpenGL vertex data alignment.
	*/
	template <>
	void push<GLbyte>(unsigned int count);

	/*
	Now the vertex data will have 'count' more GLints at the end.
	Be aware about OpenGL vertex data alignment.
	*/
	template <>
	void push<GLint>(unsigned int count);


	// Destructors.

private:

	vector<vertexBufferElement> elements_;
	unsigned int stride_;

};

inline vertexBufferLayout::vertexBufferLayout() : stride_(0) {}

inline const vector<vertexBufferElement>& vertexBufferLayout::elements() const noexcept
{

	return elements_;

}

inline unsigned int vertexBufferLayout::stride() const noexcept
{

	return stride_;

}

#endif