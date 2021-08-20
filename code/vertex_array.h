#ifndef _VERTEXARRAY_
#define _VERTEXARRAY_
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"

class vertexArray
{

public:

	vertexArray();

	void bind() const;

	void unbind() const;

	void add_layout(const vertexBufferLayout& layout);

	void add_dynamic_buffer(const vertexBuffer& vb, const vertexBufferLayout& layout, size_t size, const void* data);

	~vertexArray();

private:

	unsigned int renderer_ID_;

};


#endif