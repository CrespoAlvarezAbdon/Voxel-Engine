#include <GL/glew.h>
#include <cstdlib>
#include "renderer.h"
#include <iostream>
#include <ostream>
using namespace std;


void GLEraseErrors()
{

    while (glGetError());

}

void GLCheckErrors(ostream& os, const char* file, const char* function, unsigned int line)
{

    bool errorsDetected = false;

    os << "[OpenGL error checking]" << endl;
    while (GLenum error = glGetError())
    {

        errorsDetected = true;

        os << "Error: " << error << endl
           << "at function " << function << ", instruction before line " << line << endl
           << "in file " << file << endl << endl;

    }

    if (errorsDetected)
        abort();

}


// 'renderer' class.

void renderer::draw(const vertexArray& va, const indexBuffer& ib, const shader& sh) const
{

    sh.bind();
    va.bind();
    ib.bind();

    // Second parameter = number of indices to draw (not the number of indices that exist). 
    // Fourth parameter = pointer to the index buffer.
    // The fourth parameter is a null pointer because glDrawElements(...) uses it to bind the index buffer, but that binding has already been done
    // in ib.bind() so we don't need to repeat already done operations.
    glDrawElements(GL_TRIANGLES, ib.nIndices(), GL_UNSIGNED_INT, nullptr);
                                                                
}

void renderer::draw(int count) const
{

    glDrawArrays(GL_TRIANGLES, 0, count);

}

void renderer::clear() const
{

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

}