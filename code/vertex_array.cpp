#include "vertex_array.h"
#include <cstddef>
#include <GL/glew.h>


namespace VoxelEng {

    vertexArray::vertexArray() {

        // The first parameter is the number of arrays to generate.
        glGenVertexArrays(1, &rendererID_); 

    }

    void vertexArray::bind() const {

        glBindVertexArray(rendererID_);

    }

    void vertexArray::unbind() const {

        glBindVertexArray(0);

    }

    void vertexArray::addLayout(const vertexBufferLayout& layout) {

        const std::vector<vertexBufferElement>& elements = layout.elements();
        unsigned int offset = 0;

        for (unsigned int i = 0; i < elements.size(); i++)
        {

            vertexBufferElement element = elements[i];

            glEnableVertexAttribArray(i);

            // First parameter = attribute index.
            // Second parameter = number of values (bytes) that represent the attribute.
            // Third parameter = the type of what is representing the vertices (in this case float).
            // Fourth parameter = size of a vertex.
            // Fifth parameter = attributes offset (size to go to the second attribute if there is one. If not, simply put 0 in this parameter) (if you have to put a number, use const void * cast).
            glVertexAttribPointer(i, element.count, element.type, element.is_normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*) offset); 
        
            offset += element.count * openGLSizeOf(element.type);

        }

    }

    void vertexArray::addDynamicBuffer(const vertexBuffer& vb, const vertexBufferLayout& layout, size_t size, const void* data) {

        bind();
        vb.bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

        addLayout(layout);

    }

    vertexArray::~vertexArray() {

        glDeleteVertexArrays(1, &rendererID_);

    }

}