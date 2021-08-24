#include <string>
#include <thread>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <barrier>
#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "errors.hpp"
#include "renderer.h"
#include "vertex.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_array.h"
#include "vertex_buffer_layout.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "chunk.h"
#include "entity.h"
#include "threadPool.h"
#include <iostream>
#include <ostream>
using namespace std;


// THINGS TO DO
// 1º. ADD SOME CULLING TECHNIQUES.
// 2º. ADD BLOCK ADDING/REMOVING.
// 3º. ADD BLOCK ADDING/REMOVING CAPABLE OF UPDATING NEIGHBOR CHUNKS ON LIMIT BLOCK (aka BLOCK IN A CHUNK'S FRONTIER) CHANGED.
// 4º. INVESTIGATE ADDING MUTEX TO BLOCK ADDING/REMOVING.

/* General OpenGL notes.

    Attributes and unifroms are ways to obtain data from the CPU to the GPU.

    Unused uniforms will be removed in runtime.

    Binding is necessary in order to draw the object we are planning to show on screen.

    Vertex buffer is a GL_ARRAY_BUFFER.

    Material = Shader + all of it's uniforms.

    View projection matrix is the "view" of the camera (convention).

    Model projection matrices are used for, obviously, render models (convention).

    // Vertices' buffer. We always draw the vertices in anti-clockwise fashion.
    // Vertices can store positions, colors, texture coordinates, texture indices...
    // Each line = vertex.
    // First three floats of a line = positions of the vertex (x, y and z).
    // Second two floats of a line = texture coordinates (x, y).
    // The texture coordinates have their origin at the bottom left corner of the texture image with OpenGL.
    // The width and height of the atlas must match!
    // Texture's width and texture's height must match and must always be the same for every texture in the same atlas.

*/

int main()
{


    /*
    OpenGL's, GLFW's and GLEW's initialization.
    */
    GLFWwindow* mainWindow;
    const float width = 960.0f, height = 540.0f;

    // Initialize GLFW.
    if (!glfwInit())
        return -1;

    // This tells OpenGL that we are creating the default vertex array manually
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
    // Create a windowed mode window and its OpenGL context.
    // TODO: abstract this and the checking done after this call.
    mainWindow = glfwCreateWindow((int)width, (int)height, "Voxel Engine", NULL, NULL);

    if (!mainWindow)
    {
        
        glfwTerminate();
        return -1;

    }
    
    // Make the window's context current 
    glfwMakeContextCurrent(mainWindow); 

    // Lock mouse.
    glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    // Enable VSync (1). Disable VSync (0).
    glfwSwapInterval(0); 

    // Enable depth for proper 3D rendering.
    glEnable(GL_DEPTH_TEST); 

    // Culling.
    glEnable(GL_CULL_FACE); 

    // Enable basic blending (transparency).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set background color.
    glClearColor(134.0f / 255.0f, 169.0f / 255.0f, 254.0f / 255.0f, 1.0f);

    // With the previously created context, now initialize GLEW.
    if (glewInit() != GLEW_OK) 
        throw error(error::errorTypes::GLEW_INIT_FAILED);


    // Configurable options.
    int nChunksToDraw = 32; // WARNING. This option is for beefy computers. Average should be between 12 and 20.
    texture blockTextureAtlas("Resources/Textures/atlas.png");
    player player(45.0f, width, height, 0.1f, 500.0f, mainWindow, glm::vec3(128.0f, 145.0f, 128.0f));

    // Rendering related.
    shader default_shader("Resources/Shaders/vertexShader.shader", "Resources/Shaders/fragmentShader.shader");
    vertexArray va;
    vertexBufferLayout layout;
    vertexBufferProvider vboProvider;
    renderer renderer;
    glm::mat4 model_matrix, 
              MVPmatrix;
    
    // Chunk management thread related.
    chunkManager chunkMng(nChunksToDraw, player.getCamera());
    int nMeshingThreads = 2;
    atomic<bool> app_finished = false;
    deque<chunkRenderingData> const* chunksToDraw = nullptr;
    mutex managerThreadMutex;
    condition_variable managerThreadCV;

    // Configure some last things before starting to load the terrain.
    texture::setBlockAtlas(blockTextureAtlas);
    texture::setBlockAtlasResolution(512);
    
    cout << "[DEBUG]: Block texture atlas' size is " << blockTextureAtlas.width() << "x" << blockTextureAtlas.height() << endl
         << "and the block texture resolution is " << texture::blockAtlasResolution() << "x" << texture::blockAtlasResolution() << " pixels" << endl;

    thread chunk_management(&chunkManager::manageChunks, &chunkMng, ref(app_finished), nMeshingThreads,
                            ref(managerThreadMutex), ref(managerThreadCV));


    // The layout at the moment has only two floats that represent the vertex's position. 
    // If we want more things in the layout, call layout.push() again for each new thing.
    layout.push<GLbyte>(3); 
    layout.push<GLbyte>(1);
    layout.push<GLfloat>(2); 

    va.bind();
    default_shader.bind();
    blockTextureAtlas.bind();

    // Main game loop.
    double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
           lastFrameTime = lastSecondTime,
           actualTime,
           timeStep; // How much time has passed since the last frame was drawn.
    int nFramesDrawn = 0;
    unsigned int nVertices = 0;
    vertexBuffer* vbo = nullptr;
    while (!glfwWindowShouldClose(mainWindow))
    {

        // Times calculation.
        actualTime = glfwGetTime();
        timeStep = actualTime - lastFrameTime;
        lastFrameTime = actualTime;

        // ms/frame calculation and display.
        nFramesDrawn++;
        if (actualTime - lastSecondTime >= 1.0)
        {

            //cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame";
            nFramesDrawn = 0;
            lastSecondTime = glfwGetTime();

        }

        // Coordinate main thread and chunk management thread.
        if(managerThreadMutex.try_lock())
        {

            chunkMng.swapDrawableChunksLists();
            chunksToDraw = chunkMng.drawableChunksRead();

            managerThreadMutex.unlock();
            managerThreadCV.notify_one();

        }

        // Rendering starts here.
        renderer.clear();

        // Update camera.
        player.setCamera().updatePos(timeStep);
        player.setCamera().updateView();
       
        if (chunksToDraw)
        {

            // Render chunks.
            for (unsigned int i = 0; i < chunksToDraw->size(); i++)
            {

                nVertices = chunksToDraw->operator[](i).vertices.size();
                if (nVertices)
                {

                    vbo = vboProvider.requestVBO();
                    vbo->bind();
                    vbo->prepareStatic(chunksToDraw->operator[](i).vertices.data(), sizeof(vertex) * nVertices);

                    va.addLayout(layout);

                    model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(chunksToDraw->operator[](i).chunkPos.x * SCX, chunksToDraw->operator[](i).chunkPos.y * SCY, chunksToDraw->operator[](i).chunkPos.z * SCZ));
                    MVPmatrix = player.getCamera().projectionMatrix() * player.getCamera().viewMatrix() * model_matrix;
                    default_shader.setUniformMatrix4f("u_MVP", MVPmatrix);

                    renderer.draw(nVertices);

                }

            }
            // Free the VBOs that are not currently in use.
            vboProvider.freeVBOs();

        }

        // Swap front and back buffers.
        glfwSwapBuffers(mainWindow);

        // Poll for and process events.
        glfwPollEvents();
        
    }

    // Notify the chunk management thread that the game is closing.
    // In case the chunk management thread is locked waiting to sync with the rendering thread, unblock it.
    {

        unique_lock<mutex> lock(managerThreadMutex);
        app_finished = true;

    }
    managerThreadCV.notify_one();
    cout << "[DEBUG]: sent" << endl;

    // Wait for the chunk management thread to be finished before ending the main/rendering thread.
    chunk_management.join();

    // Terminates the GLFW library. Necessary to end the program.
    glfwTerminate();

    cout << "[DEBUG]: Rendering thread ended" << endl;

    return 0;
}