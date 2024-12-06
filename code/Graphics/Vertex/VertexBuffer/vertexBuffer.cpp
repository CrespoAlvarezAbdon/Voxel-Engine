#include "vertexBuffer.h"
#include <stdexcept>
#include <renderer.h>
#include <Graphics/graphics.h>
#include <Utilities/Logger/logger.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

    // 'vertexBuffer' class.

    vertexBuffer::vertexBuffer() 
    : rendererID_(0), lastPushedBytePos_(0), maxSize_(0)
    { }

    void vertexBuffer::generate() {

        glGenBuffers(1, &rendererID_);

    }

    void vertexBuffer::prepareStatic(const void* data, unsigned int size) {

        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        maxSize_ = size;

    }

    void vertexBuffer::prepareDynamic(long long size) {

        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        maxSize_ = size;

    }

    void vertexBuffer::pushDynamicData(const void* data, long long size) {
    
        if (lastPushedBytePos_ + size < maxSize_) {

            glBufferSubData(GL_ARRAY_BUFFER, lastPushedBytePos_, size, data);
            lastPushedBytePos_ += size;

        }
        else
            throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) + 
                "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + size));
    
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