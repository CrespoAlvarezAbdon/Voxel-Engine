#include <GL/glew.h>
#include "index_buffer.h"


// indexBuffer class.

indexBuffer::indexBuffer() : nIndices_(0)
{

    glGenBuffers(1, &rendererID_);

}

void indexBuffer::prepareStatic(const unsigned int* data, unsigned int indicesCount)
{

    nIndices_ = indicesCount;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID_);

    // We use GLuint instead of unsigned int because there may be some platforms
    // where unsigned int doesn't have the same size as GLuint and OpenGL uses GLuint not unsigned int.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(GLuint), data, GL_STATIC_DRAW);   
                                                                                                    

}

void indexBuffer::bind() const
{

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID_);

}

void indexBuffer::unbind() const
{

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

indexBuffer::~indexBuffer()
{

    glDeleteBuffers(1, &rendererID_);

}

