#include <GL/glew.h>
#include "index_buffer.h"

Index_buffer::Index_buffer() : indices_count_(0)
{

    glGenBuffers(1, &renderer_ID_);

}

void Index_buffer::prepare_static(const unsigned int* data, unsigned int indices_count)
{

    indices_count_ = indices_count;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_ID_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_count * sizeof(GLuint), data, GL_STATIC_DRAW);    // GLuint instead of unsigned int cause there may be some platforms
                                                                                                    // where unsigned int doesn't have the same size as GLuint and OpenGL uses GLuint not unsigned int

}

void Index_buffer::bind() const
{

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_ID_);

}

void Index_buffer::unbind() const
{

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

Index_buffer::~Index_buffer()
{

    glDeleteBuffers(1, &renderer_ID_);

}

