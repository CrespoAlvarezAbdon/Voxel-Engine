#include "vertexBuffer.h"
#include "renderer.h"
#include "logger.h"
#include "graphics.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h> // POSSIBLE ERROR AQUÍ POR PONERLO EN ESTE ORDEN?

#endif


namespace VoxelEng {

    // 'vertexBuffer' class.

    vertexBuffer::vertexBuffer() 
    : rendererID_(0)
    { }

    void vertexBuffer::generate()
    {

        glGenBuffers(1, &rendererID_);

    }

    void vertexBuffer::prepareStatic(const void* data, unsigned int size) {

        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    }

    void vertexBuffer::prepareDynamic(long long size) {

        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

    }

    void vertexBuffer::setDynamicData(const void* data, long long offset, long long size) {

        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);

    }

    void vertexBuffer::bind() const {

        glBindBuffer(GL_ARRAY_BUFFER, rendererID_);

    }

    void vertexBuffer::unbind() const {

        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }

    vertexBuffer::~vertexBuffer() {

        glDeleteBuffers(1, &rendererID_);

    }

}