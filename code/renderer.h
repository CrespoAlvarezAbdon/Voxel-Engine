#ifndef _RENDERER_
#define _RENDERER_

#include <ostream>
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "shader.h"
using namespace std;


void GL_erase_errors();
void GL_check_errors(ostream& os, const char* file, const char* function, unsigned int line);

class renderer
{

public:

	void draw(const vertexArray& va, const Index_buffer& ib, const Shader& sh) const; // TODO: remove va and sh parameters
	void draw(int count) const;

	void clear() const;

private:



};

#endif