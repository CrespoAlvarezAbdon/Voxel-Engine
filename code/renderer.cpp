#include <GL/glew.h>
#include <cstdlib>
#include "renderer.h"
#include <iostream>
#include <ostream>
using namespace std;

void GL_erase_errors()
{

    while (glGetError());

}

void GL_check_errors(ostream& os, const char* file, const char* function, unsigned int line)
{

    bool errors_detected = false;

    os << "OpenGL error checking\n";
    while (GLenum error = glGetError())
    {

        errors_detected = true;

        os << "Error: " << error << endl
           << "at function " << function << ", instruction before line " << line << "\nin file " << file << endl
           << endl;

    }

    if (errors_detected)
        abort();

}

void renderer::draw(const vertexArray& va, const Index_buffer& ib, const Shader& sh) const
{

    sh.bind();
    va.bind();
    ib.bind();

    glDrawElements(GL_TRIANGLES, ib.indices_count(), GL_UNSIGNED_INT, nullptr);  // Draw call.
                                                                // Second parameter = number of indices to draw (not the number of indices that exist). Fourth parameter = pointer to the index buffer
                                                                // Fourth parameter is nullptr because glDrawElements uses it to bind the index buffer, but that binding has already been done
                                                                // in instruction glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object),that is, ib.bind()

}

void renderer::draw(int count) const
{
    GLint size = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    glDrawArrays(GL_TRIANGLES, 0, count);  // Draw call.
                                                                // Second parameter = number of indices to draw (not the number of indices that exist). Fourth parameter = pointer to the index buffer
                                                                // Fourth parameter is nullptr because glDrawElements uses it to bind the index buffer, but that binding has already been done
                                                                // in instruction glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object),that is, ib.bind()

}

void renderer::clear() const
{

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

}