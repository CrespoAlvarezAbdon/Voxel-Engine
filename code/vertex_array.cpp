#include <cstddef>
#include <GL/glew.h>
#include "vertex_array.h"
using namespace std;

vertexArray::vertexArray()
{

    glGenVertexArrays(1, &renderer_ID_); // First parameter, number of arrays to generate

}

void vertexArray::bind() const
{

    glBindVertexArray(renderer_ID_);

}

void vertexArray::unbind() const
{

    glBindVertexArray(0);

}

void vertexArray::add_layout(const vertexBufferLayout& layout)
{

    const vector<Vertex_buffer_element>& elements = layout.elements();
    unsigned int offset = 0;

    for (unsigned int i = 0; i < elements.size(); i++)
    {

        Vertex_buffer_element element = elements[i];

        // We define the vertices' layout
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.is_normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*)offset); 
        // first parameter = attribute index
        // second parameter = number of values that represent the attribute
        // third parameter = the type of what is representing the vertices (in this case float)
        // fourth parameter = size of a vertex
        // fifth parameter = attributes offset (size to go to the second attribute if there is one. If not, simply put 0 in this parameter) (if you have to put a number, use const void * cast)

        offset += element.count * OpenGL_size_of(element.type);

    }

}

void vertexArray::add_dynamic_buffer(const vertexBuffer& vb, const vertexBufferLayout& layout, size_t size, const void* data) // Change this to use Vertex_array::add_layout()
{

    bind();
    vb.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data); // Set the vertices' data

    const vector<Vertex_buffer_element>& elements = layout.elements();
    unsigned int offset = 0;

    for (unsigned int i = 0; i < elements.size(); i++)
    {

        Vertex_buffer_element element = elements[i];

        // We define the vertices' layout
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.is_normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*)offset);
        // first parameter = attribute index
        // second parameter = number of values that represent the attribute
        // third parameter = the type of what is representing the vertices
        // fourth parameter = size of a vertex
        // fifth parameter = attributes offset (size to go to the second attribute if there is one. If not, simply put 0 in this parameter) (if you have to put a number, use const void * cast)

        offset += element.count * OpenGL_size_of(element.type);

    }

}

vertexArray::~vertexArray()
{

    glDeleteVertexArrays(1, &renderer_ID_);

}