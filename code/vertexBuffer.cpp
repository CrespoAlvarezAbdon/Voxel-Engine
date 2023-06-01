#include "vertexBuffer.h"
#include "renderer.h"
#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

    // 'vertexBuffer' class.

    vertexBuffer::vertexBuffer() {

        glGenBuffers(1, &rendererID_);

    }

    void vertexBuffer::prepareStatic(const void* data, unsigned int size) {

        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    }

    void vertexBuffer::prepareDynamic(unsigned int size) {

        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

    }

    void vertexBuffer::replaceDynamicData(const void* data, unsigned int size) {

        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

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