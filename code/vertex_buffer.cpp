#include <GL/glew.h>
#include "vertex_buffer.h"
#include "renderer.h"


// vertexBuffer.

vertexBuffer::vertexBuffer()
{

    glGenBuffers(1, &rendererID_);

}

void vertexBuffer::prepareStatic(const void* data, unsigned int size)
{

    glBindBuffer(GL_ARRAY_BUFFER, rendererID_);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

}

void vertexBuffer::prepareDynamic(unsigned int size)
{

    glBindBuffer(GL_ARRAY_BUFFER, rendererID_);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

}

void vertexBuffer::bind() const
{

    glBindBuffer(GL_ARRAY_BUFFER, rendererID_);

}

void vertexBuffer::unbind() const
{

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

vertexBuffer::~vertexBuffer()
{

    glDeleteBuffers(1, &rendererID_);

}


// vertexBufferProvider.

vertexBufferProvider::vertexBufferProvider() 
    : vboIndex_(0)
{}

vertexBuffer* vertexBufferProvider::requestVBO() 
{

    vertexBuffer* vbo = nullptr;

    // If there aren't VBOs available, create one.
    if (vbos_.empty() || vboIndex_ == vbos_.size())
    {
    
        vbos_.push_back(new vertexBuffer());  
    
    }

    vbo = vbos_[vboIndex_];
    vboIndex_++;

    return vbo;

}