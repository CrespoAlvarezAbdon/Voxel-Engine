#ifndef _VERTEXBUFFERLAYOUT_
#define _VERTEXBUFFERLAYOUT_

#include <vector>
#include <GL/glew.h>
using namespace std;

unsigned int OpenGL_size_of(unsigned int type); // Auxiliary function used to get the size of OpenGL types

struct Vertex_buffer_element
{

	unsigned int type; // OpenGL types are represented with unsigned ints
	unsigned int count;
	bool is_normalized;

};

class vertexBufferLayout
{

public:

	vertexBufferLayout();

	template <typename T>
	void push(unsigned int count);
	template <>
	void push<GLfloat>(unsigned int count);
	template <>
	void push<unsigned int>(unsigned int count);
	template <>
	void push<unsigned char>(unsigned int count);
	template <>
	void push<GLbyte>(unsigned int count);
	template <>
	void push<GLint>(unsigned int count);

	const vector<Vertex_buffer_element>& elements() const noexcept;
	unsigned int stride() const noexcept;

private:

	vector<Vertex_buffer_element> elements_;

	unsigned int stride_;

};

inline vertexBufferLayout::vertexBufferLayout() : stride_(0) {}

inline const vector<Vertex_buffer_element>& vertexBufferLayout::elements() const noexcept
{

	return elements_;

}

inline unsigned int vertexBufferLayout::stride() const noexcept
{

	return stride_;

}

#endif