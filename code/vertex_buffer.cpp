#include <GL/glew.h>
#include "vertex_buffer.h"
#include "renderer.h"

#include <iostream>
#include <ostream>
using namespace std;


// vertexBuffer

vertexBuffer::vertexBuffer()
{

    glGenBuffers(1, &renderer_ID_);

}

void vertexBuffer::prepare_static(const void* data, unsigned int size)
{

    glBindBuffer(GL_ARRAY_BUFFER, renderer_ID_);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

}

void vertexBuffer::prepare_dynamic(unsigned int size)
{

    glBindBuffer(GL_ARRAY_BUFFER, renderer_ID_);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

}

void vertexBuffer::bind() const
{

    glBindBuffer(GL_ARRAY_BUFFER, renderer_ID_);

}

void vertexBuffer::unbind() const
{

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

vertexBuffer::~vertexBuffer()
{

    glDeleteBuffers(1, &renderer_ID_);

}


// vertexBufferProvider

/*
Constructs a vertexBufferProvider object with no VBOs.
*/
vertexBufferProvider::vertexBufferProvider() 
    : vboIndex_(0)
{}

/*
Request a vertex buffer object (VBO) to use for rendering.
If there aren't already created VBOs available, more will be created.
Already created VBOs which are available are reused.
*/
vertexBuffer* vertexBufferProvider::requestVBO() 
{

    vertexBuffer* vbo = nullptr;

    // If there aren't VBOs available, create one more.
    if (vbos_.empty() || vboIndex_ == vbos_.size())
    {
    
        vbos_.push_back(new vertexBuffer());  
    
    }

    vbo = vbos_[vboIndex_];
    vboIndex_++;

    return vbo;

}