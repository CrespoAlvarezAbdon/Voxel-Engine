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
#include <iostream>
#include <ostream>
using namespace std;


// THINGS TO DO
// 1º. ADD SOME CULLING TECHNIQUES.
// 2º. ADD BLOCK ADDING/REMOVING.
// 3º. ADD BLOCK ADDING/REMOVING CAPABLE OF UPDATING NEIGHBOR CHUNKS ON LIMIT BLOCK (aka BLOCK IN A CHUNK'S FRONTIER) CHANGED.
// 4º. INVESTIGATE ADDING MUTEX TO BLOCK ADDING/REMOVING.
// 5º. VERIFY CUSTOM CHUNK ATLAS SIZE AND RESOLUTION.

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

    glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GLFW_TRUE);


    // Configurable options.
    int nChunksToDraw = 32; // WARNING. This option is for beefy computers. Average should be between 12 and 20.
    unsigned int blockReachRange = 5;
    texture blockTextureAtlas("Resources/Textures/atlas.png");
    player player(45.0f, width, height, 0.1f, 500.0f, mainWindow, blockReachRange, glm::vec3(128.0f, 145.0f, 128.0f));

    // Set player input callbacks.
    // TODO. Abstract this into a input.h or similar.
    glfwSetWindowUserPointer(mainWindow, &player);
    glfwSetMouseButtonCallback(mainWindow, player::mouseButtonCallback);

    // Rendering related.
    shader defaultShader("Resources/Shaders/vertexShader.shader", "Resources/Shaders/fragmentShader.shader");
    vertexArray va;
    vertexBufferLayout layout;
    vertexBufferProvider vboProvider;
    renderer renderer;
    glm::mat4 modelMatrix, 
              MVPmatrix;
    
    // Chunk management thread related.
    chunkManager chunkMng(nChunksToDraw, player.mainCamera());
    int nMeshingThreads = 2; // Number of threads for non-high priority mesh updates.
    unordered_map<glm::vec3, vector<vertex>> const* chunksToDraw = nullptr;
    atomic<bool> appFinished = false;

    // Finish connecting some objects.
    player.setChunkManager(&chunkMng);
    player.mainCamera().setChunkManager(&chunkMng);

    // Configure texture block atlas.
    texture::setBlockAtlas(blockTextureAtlas);
    texture::setBlockAtlasResolution(16);
    
    // Debug information.
    cout << "[DEBUG]: Block texture atlas' size is " << blockTextureAtlas.width() << "x" << blockTextureAtlas.height() << endl
         << "and the block texture resolution is " << texture::blockAtlasResolution() << "x" << texture::blockAtlasResolution() << " pixels" << endl;

    // Start generating and meshing the terrain.
    thread chunkManagementThread(&chunkManager::manageChunks, &chunkMng, ref(appFinished), nMeshingThreads);


    // Configure the vertex layout.
    layout.push<GLbyte>(3); 
    layout.push<GLbyte>(1);
    layout.push<GLfloat>(2); 

    // Bind the currently used VAO, shaders and atlases.
    va.bind();
    defaultShader.bind();
    blockTextureAtlas.bind();

    // Main game loop starts here.
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

            cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame";
            nFramesDrawn = 0;
            lastSecondTime = glfwGetTime();

        }

        // Coordinate main thread and chunk management thread.
        if(chunkMng.managerThreadMutex().try_lock())
        {

            if (chunkMng.forceSyncFlag()) 
            {
                
                chunkMng.updatePriorityChunk(chunkMng.priorityChunkPos());

            }
            else 
            {
            
                chunkMng.swapDrawableChunksLists();
                chunksToDraw = chunkMng.drawableChunksRead();
            
            }

            chunkMng.managerThreadMutex().unlock();
            chunkMng.managerThreadCV().notify_one();

        }

        // Rendering starts here.
        renderer.clear();

        // Update player view.
        player.mainCamera().updatePos(timeStep);
        player.selectBlock();
        player.mainCamera().updateView();
       
        if (chunksToDraw)
        {

            // Render chunks.
            // chunk.first refers to the chunk's postion.
            // chunk.second refers to the chunk's vertex data.
            for (auto const& chunk : *chunksToDraw)
            {

                nVertices = chunk.second.size();
                if (nVertices)
                {

                    vbo = vboProvider.requestVBO();
                    vbo->bind();
                    vbo->prepareStatic(chunk.second.data(), sizeof(vertex) * nVertices);

                    va.addLayout(layout);

                    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.first.x * SCX, chunk.first.y * SCY, chunk.first.z * SCZ));
                    MVPmatrix = player.mainCamera().projectionMatrix() * player.mainCamera().viewMatrix() * modelMatrix;
                    defaultShader.setUniformMatrix4f("u_MVP", MVPmatrix);

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

    // Notify the chunk management and the high priority update threads that the game is closing.
    // In case a thread is waiting on a corresponding condition variable, send a notification to unblock it.
    {

        unique_lock<mutex> lock(chunkMng.managerThreadMutex());
        appFinished = true;

    }
    chunkMng.managerThreadCV().notify_one();


    // Wait for the chunk management thread to be finished before ending the main/rendering thread.
    chunkManagementThread.join();

    // Terminates the GLFW library. Necessary to end the program.
    glfwTerminate();

    cout << "[DEBUG]: Rendering thread ended" << endl;

    return 0;
}