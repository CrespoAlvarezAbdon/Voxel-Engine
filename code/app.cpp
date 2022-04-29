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
#include "batch.h"
#include "gameWindow.h"
#include "graphics.h"
#include "world.h"
#include "gui.h"
#include <iostream>
#include <ostream>


int main()
{

    // Create game's main window.
    VoxelEng::window mainWindow(800, 600, "Voxel engine");


    // Configure the graphics API/libraries.
    VoxelEng::graphics::initialize(mainWindow);
    VoxelEng::graphics::setVSync(false);
    VoxelEng::graphics::setDepthTest(true);
    VoxelEng::graphics::setBasicFaceCulling(true);
    VoxelEng::graphics::setTransparency(true);


    // Configure game window's settings.
    mainWindow.changeStateMouseLock(false);


    // Game Variables/objects.
    atomic<bool> appFinished = false; // Signals if the game has finished executing or not.
    atomic<double> timeStep = 0.0f; // How much time has passed since the last frame was drawn. Use this to move entities without caring about FPS.


    // Worlds. 
    VoxelEng::world world(0);


    // Configurable.
    VoxelEng::skybox defaultSkybox(41, 37, 77.0f, 1.0f);
    glm::vec3 playerSpawnPosition(0, 145.0f, 0);
    unsigned int blockReachRange = 5,
                 spawnWorldID = 0;
    float FOV = 110.0f,
          zNear = 0.1f,
          zFar = 500.0f;
    player player(FOV, zNear, zFar, mainWindow, blockReachRange, playerSpawnPosition, spawnWorldID, &appFinished);
    camera& playerCamera = player.mainCamera();
    int nChunksToDraw = 20; // Controls the maximun render distance in the x and z axis. Average should be between 12 and 20. Max 32 is supported.
    texture blockTextureAtlas("Resources/Textures/atlas.png");
    

    // Custom model loading.
    VoxelEng::models::loadCustomModel("Resources/Models/Warden.obj", 14);


    // Rendering related.
    shader defaultShader("Resources/Shaders/vertexShader.shader", "Resources/Shaders/fragmentShader.shader");
    vertexBuffer vbo;
    vertexArray va;
    vertexBufferLayout layout;
    renderer renderer;
    glm::mat4 MVPmatrix;
    world.setBackground(defaultSkybox);


    // Configure texture block atlas.
    texture::setBlockAtlas(blockTextureAtlas);
    texture::setBlockAtlasResolution(16);


    // GUI initialization and GUI elements registration.
    VoxelEng::GUIManager::initialize(mainWindow, defaultShader, renderer);
    VoxelEng::GUIManager::addGUIBox("mainMenu", 0.35, 0.25, 0.3, 0.5, 995, false);
    VoxelEng::GUIManager::addGUIBox("mainMenu.button1", 0.425, 0.35, 0.15, 0.10, 993, false, "mainMenu");
    VoxelEng::GUIManager::addGUIBox("mainMenu.button2", 0.425, 0.5, 0.15, 0.10, 961, false, "mainMenu");

    
    // Set up GUIElements' keys and key functions.
    VoxelEng::GUIManager::bindActKey("mainMenu", VoxelEng::key::e);
    VoxelEng::GUIManager::bindActKeyFunction("mainMenu", []() {

        VoxelEng::GUIManager::changeGUIState("mainMenu");
        VoxelEng::GUIManager::changeGUIState("mainMenu.button1");
        VoxelEng::GUIManager::changeGUIState("mainMenu.button2");
        VoxelEng::graphics::getMainWindow()->changeStateMouseLock();

    });


    // Chunk management thread related.
    chunkManager chunkMng(nChunksToDraw, playerCamera);
    int nMeshingThreads = 2; // Number of threads for non-high priority mesh updates.
    unordered_map<glm::vec3, vector<vertex>> const* chunksToDraw = nullptr;
    const std::vector<const VoxelEng::model*>* batchesToDraw = nullptr;


    // Finish connecting some objects.
    mainWindow.playerCamera() = &playerCamera;
    player.setChunkManager(&chunkMng);
    playerCamera.setChunkManager(&chunkMng);


    // Set up GLFW callbacks.
    VoxelEng::graphics::setPlayerCallbackPtr(&player);
    VoxelEng::graphics::setWindowCallbackPtr(&mainWindow);

    glfwSetMouseButtonCallback(mainWindow.windowAPIpointer(), player::mouseButtonCallback);
    glfwSetWindowSizeCallback(mainWindow.windowAPIpointer(), VoxelEng::window::windowSizeCallback);


    // Configure the vertex layout for 3D rendering.
    layout.push<GLfloat>(3);
    layout.push<GLfloat>(2);
    layout.push<VoxelEng::normalVec>(1);

    
    // Bind the currently used VAO, shaders and atlases for 3D rendering.
    vbo.bind();
    va.bind();
    va.addLayout(layout);
    defaultShader.bind();
    blockTextureAtlas.bind();


    // Start the terrain management and
    // the player input processing threads.
    thread chunkManagementThread(&chunkManager::manageChunks, &chunkMng, ref(appFinished), nMeshingThreads),
           playerInputThread(&player::processPlayerInput, &player),
           entityManagementThread(&VoxelEng::entityManager::manageEntities, ref(timeStep), ref(appFinished));


    // Set sun light's position.
    glm::vec3 lightpos(10.0f, 150.0f, -10.0f);
    defaultShader.setUniformVec3f("u_sunLightPos", lightpos);


    // Time/FPS related stuff.
    double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
           lastFrameTime = lastSecondTime,
           actualTime;
    int nFramesDrawn = 0;


    /*
    Rendering loop.
    */
    unsigned int nVertices = 0;
    while (!mainWindow.isClosing()) {

        // Get key inputs for GUIManager.
        VoxelEng::GUIManager::processGUIInputs();

        // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
        // after the player has stopped resizing the window.
        if (mainWindow.wasResized()) {
        
            mainWindow.resizeHeavyProcessing();
        
        }

        // Set 3D rendering mode uniforms.
        defaultShader.setUniform1i("u_renderMode", 0); // renderMode = 0 stands for 3D rendering mode.

        MVPmatrix = playerCamera.projectionMatrix() * playerCamera.viewMatrix();
        defaultShader.setUniformMatrix4f("u_MVP", MVPmatrix);
        defaultShader.setUniformVec3f("u_camPos", playerCamera.pos());

        // Times calculation.
        actualTime = glfwGetTime();
        timeStep = actualTime - lastFrameTime;
        lastFrameTime = actualTime;

        // Clear the screen to draw the next frame.
        renderer.clear();


        // ms/frame calculation and display.
        nFramesDrawn++;
        if (actualTime - lastSecondTime >= 1.0) {

            std::cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame" << " and resolution is " << mainWindow.width() << " x " << mainWindow.height();
            nFramesDrawn = 0;
            lastSecondTime = glfwGetTime();

        }

        // Coordinate rendering thread and chunk management thread.
        if (chunkMng.managerThreadMutex().try_lock()) {

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

        // Coordinate rendering thread and entity management thread.
        if (VoxelEng::entityManager::syncMutex().try_lock()) {
        
            VoxelEng::entityManager::swapReadWrite();
            batchesToDraw = VoxelEng::entityManager::renderingData();

            VoxelEng::entityManager::syncMutex().unlock();
            VoxelEng::entityManager::entityManagerCV().notify_one();
        
        }


        /*
        3D rendering.
        */
        vbo.bind();
        va.bind();

        // Update player's camera.
        if (!mainWindow.isMouseFree())
            playerCamera.updatePos(timeStep);
        playerCamera.updateView();

        // Render chunks.
        if (chunksToDraw) {
                   
            // chunk.first refers to the chunk's postion.
            // chunk.second refers to the chunk's vertex data.
            for (auto const& chunk : *chunksToDraw) {
                
                if (nVertices = chunk.second.size()) {
                    
                    vbo.prepareStatic(chunk.second.data(), sizeof(vertex) * nVertices);

                    renderer.draw3D(nVertices);

                }

            }

        }

        // Render batches.
        if (batchesToDraw) {

            for (auto const& batch : *batchesToDraw) {
            
                if (nVertices = batch->size()) {
                
                    vbo.prepareStatic(batch->data(), sizeof(vertex) * nVertices);

                    renderer.draw3D(nVertices);
                
                }

            }

        }


        /*
        2D rendering.
        */ 
        VoxelEng::GUIManager::drawGUI();

        // Swap front and back buffers.
        glfwSwapBuffers(mainWindow.windowAPIpointer());

        // Poll for and process events.
        glfwPollEvents();
        
    }

    // Notify the chunk management and the high priority update threads that the game is closing.
    // In case a thread is waiting on a corresponding condition variable, send a notification to unblock it.
    {

        unique_lock<mutex> lock(chunkMng.managerThreadMutex());
        std::unique_lock<std::mutex> syncLock(VoxelEng::entityManager::syncMutex());
        appFinished = true;

    }
    chunkMng.managerThreadCV().notify_one();
    VoxelEng::entityManager::entityManagerCV().notify_one();


    // Wait for the chunk management thread to be finished before ending the main/rendering thread.
    chunkManagementThread.join();
    playerInputThread.join();
    entityManagementThread.join();

    // Terminates the GLFW library. Necessary to end the program.
    glfwTerminate();

    std::cout << "[DEBUG]: Rendering thread ended" << std::endl;

    return 0;
}