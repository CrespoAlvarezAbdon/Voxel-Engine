#include <GL/glew.h>
#include <cstdlib>
#include "renderer.h"


namespace VoxelEng {

    // OpenGL debugging.
    void GLEraseErrors() {

        while (glGetError());

    }

    void GLCheckErrors(std::ostream& os, const char* file, const char* function, unsigned int line) {

        bool errorsDetected = false;

        os << "[OpenGL error checking]" << std::endl;
        while (GLenum error = glGetError()) {

            errorsDetected = true;

            os << "Error: " << error << std::endl
               << "at function " << function << ", instruction before line " << line << std::endl
               << "in file " << file << std::endl << std::endl;

        }

        if (errorsDetected)
            abort();

    }


    // 'renderer' class.

    void renderer::draw3D(const indexBuffer& ib) const {

        // Second parameter = number of indices to draw (not the number of indices that exist). 
        // Fourth parameter = pointer to the index buffer.
        // The fourth parameter is a null pointer because glDrawElements(...) uses it to bind the index buffer, but that binding has already been done
        // in ib.bind() so we don't need to repeat already done operations.
        glDrawElements(GL_TRIANGLES, ib.nIndices(), GL_UNSIGNED_INT, nullptr);
                                                                
    }

    void renderer::draw3D(int count) const {

        glDrawArrays(GL_TRIANGLES, 0, count);

    }

    void renderer::draw2D(int count) const {

        glDrawArrays(GL_TRIANGLES, 0, count);

    }

    void renderer::clear() const {

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    }

}