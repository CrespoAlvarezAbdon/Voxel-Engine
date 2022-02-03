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
#include "gameWindow.h"
#include "graphics.h"
#include "world.h"
#include <iostream>
#include <ostream>


// THINGS TO DO
// 1º. ADD SOME CULLING TECHNIQUES.
// 2º. VERIFY CUSTOM CHUNK ATLAS SIZE AND RESOLUTION.
// 3º. ABSTRACT GRAPHICS API CALLS IN A 'graphics' CLASS.
// 4º. ADD A PROPER SKYBOX INSTEAD OF RELYING ON THE GRAPHICS_API BACKGROUND COLOR.
// 5º. ADD ANY CALLS TO VBO, VAO, RENDERERS THROUGH THE 'graphics' CLASS.
// 6º. FIX GAME TAKING TOO LONG TO CLOSE (LOOK AT MESHING LOOPS AND ADD ADDITIONAL CONDITION).
// 7º. FIX BEING UNABLE TO BUILD IF YOU WALK TO FAR (look at raycasting method that we are using to check blocks and do some debug starting from there).

int main()
{

    // Create game's main window.
    VoxelEng::window mainWindow(800, 600, "Voxel engine");

    // APIs/libraries initializations.
    VoxelEng::graphics::initialize(mainWindow);

    // Configure the graphics API/libraries.
    VoxelEng::graphics::setVSync(false);
    VoxelEng::graphics::setDepthTest(true);
    VoxelEng::graphics::setBasicFaceCulling(true);
    VoxelEng::graphics::setTransparency(true);

    // Configure game window's settings.
    mainWindow.lockMouse();


    // Game Variables/objects.
    atomic<bool> appFinished = false; // Signals if the game has finished executing or not.

    // TODO. ADD CUSTOM SKY COLOR PER WORLD.
    // Worlds. 
    VoxelEng::world spaceWorld(0);
    VoxelEng::world firstPlanet(1);

    // Configurable.
    VoxelEng::skybox defaultSkybox(134.0f, 169.0f, 254.0f, 1.0f);
    glm::vec3 playerSpawnPosition(128.0f, 145.0f, 128.0f);
    unsigned int blockReachRange = 5,
                 spawnWorldID = 0;
    float FOV = 110.0f,
          zNear = 0.1f,
          zFar = 500.0f;
    player player(FOV, zNear, zFar, mainWindow, blockReachRange, playerSpawnPosition, spawnWorldID, &appFinished);
    int nChunksToDraw = 20; // Controls the maximun render distance in the x and z axis. Average should be between 12 and 20. Max 32 is supported.
    texture blockTextureAtlas("Resources/Textures/atlas.png");
    
    // Custom model loading.
    VoxelEng::models::loadCustomModel("Resources/Models/Warden.obj", 14);

    // Rendering related.
    shader defaultShader("Resources/Shaders/vertexShader.shader", "Resources/Shaders/fragmentShader.shader");
    vertexBuffer vbo = vertexBuffer();
    vertexArray va;
    vertexBufferLayout layout;
    renderer renderer;
    glm::mat4 modelMatrix, 
              MVPmatrix;
    
    // Chunk management thread related.
    chunkManager chunkMng(nChunksToDraw, player.mainCamera());
    int nMeshingThreads = 2; // Number of threads for non-high priority mesh updates.
    unordered_map<glm::vec3, vector<vertex>> const* chunksToDraw = nullptr;

    // Configure texture block atlas.
    texture::setBlockAtlas(blockTextureAtlas);
    texture::setBlockAtlasResolution(16);

    // Finish connecting some objects.
    mainWindow.playerCamera() = &player.mainCamera();
    player.setChunkManager(&chunkMng);
    player.mainCamera().setChunkManager(&chunkMng);


    // Set up callbacks.
    VoxelEng::graphics::setPlayerCallbackPtr(&player);
    VoxelEng::graphics::setWindowCallbackPtr(&mainWindow);

    glfwSetMouseButtonCallback(mainWindow.windowAPIpointer(), player::mouseButtonCallback);
    glfwSetWindowSizeCallback(mainWindow.windowAPIpointer(), VoxelEng::window::windowSizeCallback);


    // Configure the vertex layout.
    layout.push<GLfloat>(3);
    layout.push<GLfloat>(2);

    // Bind the currently used VAO, shaders and atlases.
    vbo.bind();
    va.bind();
    va.addLayout(layout);
    defaultShader.bind();
    blockTextureAtlas.bind();


    // Start the terrain management and
    // the player input processing threads.
    thread chunkManagementThread(&chunkManager::manageChunks, &chunkMng, ref(appFinished), nMeshingThreads),
           playerInputThread(&player::processPlayerInput, &player);


    // Print some startup debug information.
    std::cout << "[DEBUG]: Block texture atlas' size is " << blockTextureAtlas.width() << "x" << blockTextureAtlas.height() << std::endl
         << "and the block texture resolution is " << texture::blockAtlasResolution() << "x" << texture::blockAtlasResolution() << " pixels" << std::endl;


    // W.I.P DEBUG MODEL LOADING SETUP.
    // TODO. ADD 'batch' CLASS. SIMILIAR TO 'chunk' CLASS EXCEPT IT STORES VERTEX DATA FROM
    // MANY ARBITRARY MODELS TO SAVE DRAW CALLS.
    // La esfera no se cullea y algunas texturas están donde no deberían estar
    VoxelEng::model* robotModel = VoxelEng::models::models_[14];
    
    // Rendering loop starts here.
    double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
           lastFrameTime = lastSecondTime,
           actualTime,
           timeStep; // How much time has passed since the last frame was drawn.
    int nFramesDrawn = 0;
    unsigned int nVertices = 0;
    while (!mainWindow.isClosing())
    {

        // Times calculation.
        actualTime = glfwGetTime();
        timeStep = actualTime - lastFrameTime;
        lastFrameTime = actualTime;

        // ms/frame calculation and display.
        nFramesDrawn++;
        if (actualTime - lastSecondTime >= 1.0)
        {

            std::cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame";
            nFramesDrawn = 0;
            lastSecondTime = glfwGetTime();

        }

        // Coordinate main thread and chunk management thread.
        if(chunkMng.managerThreadMutex().try_lock())
        {

            if (chunkMng.forceSyncFlag()) 
                chunkMng.updatePriorityChunks();
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

        // Update player's camera.
        player.mainCamera().updatePos(timeStep);
        player.mainCamera().updateView();

        // Render chunks.
        if (chunksToDraw)
        {
            
            // chunk.first refers to the chunk's postion.
            // chunk.second refers to the chunk's vertex data.
            for (auto const& chunk : *chunksToDraw)
            {

                nVertices = chunk.second.capacity();
                
                if (nVertices)
                {

                    vbo.prepareStatic(chunk.second.data(), sizeof(vertex) * nVertices);

                    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.first.x * SCX, chunk.first.y * SCY, chunk.first.z * SCZ));
                    MVPmatrix = player.mainCamera().projectionMatrix() * player.mainCamera().viewMatrix() * modelMatrix;
                    defaultShader.setUniformMatrix4f("u_MVP", MVPmatrix);

                    renderer.draw(nVertices);

                }

            }

        }

        // Render batches (test W.I.P).
        if (true) {

            vbo.prepareDynamic(sizeof(vertex) * robotModel->size());
            vbo.replaceDynamicData(robotModel->data(), sizeof(vertex) * robotModel->size());

            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(128, 150, 128));
            MVPmatrix = player.mainCamera().projectionMatrix() * player.mainCamera().viewMatrix() * modelMatrix;
            defaultShader.setUniformMatrix4f("u_MVP", MVPmatrix);

            renderer.draw(robotModel->size());

        }

        // Swap front and back buffers.
        glfwSwapBuffers(mainWindow.windowAPIpointer());

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
    playerInputThread.join();

    // Terminates the GLFW library. Necessary to end the program.
    glfwTerminate();

    std::cout << "[DEBUG]: Rendering thread ended" << std::endl;

    return 0;
}