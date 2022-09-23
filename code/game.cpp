#include "game.h"
#include <string>
#include <thread>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <barrier>
#include <functional>
#include <iostream>
#include <ostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "vertex.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "entity.h"
#include "batch.h"
#include "graphics.h"
#include "gui.h"
#include "GUIFunctions.h"
#include "worldGen.h"
#include "input.h"
#include "inputFunctions.h"
#include "AIAPI.h"
#include "utilities.h"
#include "tickFunctions.h"
#include "definitions.h"
#include "AI/AIGameEx1.h"


namespace VoxelEng {

    ////////////
    //Classes.//
    ////////////


    // 'game' class.

    bool game::initialised_ = false,
         game::graphicalModeInitialised_ = false;

    window* game::mainWindow_ = nullptr;

    std::atomic<unsigned int> game::loopSelection_ = AIMENULOOP;
    std::atomic<double> game::timeStep_ = 0.0f;

    skybox game::defaultSkybox_(140, 170, 255, 1.0f); // TODO. MOVE THIS TO WORLD.H

    unsigned int game::saveSlot_ = 0,
                 game::blockReachRange_ = 5,
                 game::nMeshingThreads_ = 1;
    float game::FOV_ = 110.0f,
          game::zNear_ = 0.1f,
          game::zFar_ = 500.0f;

    camera* game::playerCamera_ = nullptr;

    bool game::useComplexLighting_ = false;

    texture* game::blockTextureAtlas_ = nullptr;

    std::unordered_map<vec3, std::vector<vertex>> const* game::chunksToDraw_ = nullptr;
    const std::vector<const model*>* game::batchesToDraw_ = nullptr;

    shader* game::defaultShader_ = nullptr;
    vertexBuffer* game::vbo_ = nullptr;
    vertexArray* game::va_ = nullptr;
    vertexBufferLayout* game::layout_ = nullptr;
    renderer* game::renderer_ = nullptr;

    #if GRAPHICS_API == OPENGL

        glm::mat4 game::MVPmatrix_;

    #else



    #endif

    slotAccessType game::slotAccessType_ = slotAccessType::load;


	void game::init() {

        if (initialised_)
            logger::errorLog("Game is already initialised");
        else {

            // Put here any game initialisation that does not involve
            // the engine's graphical mode.

            loopSelection_ = AIMENULOOP;
            timeStep_ = 0.0f;

            worldGen::init();

            initialised_ = true;

        }

	}
    
    void game::initGraphicalMode() {
    
        if (graphicalModeInitialised_)
            logger::errorLog("Engine's graphical mode already initialised");
        else {

            mainWindow_ = new window(800, 600, "VoxelEng");

            saveSlot_ = 0,
            blockReachRange_ = 5,
            nMeshingThreads_ = 1;
            
            FOV_ = 110.0f,
            zNear_ = 0.1f,
            zFar_ = 500.0f;

            useComplexLighting_ = false;

            #if GRAPHICS_API == OPENGL

                MVPmatrix_ = glm::mat4();

            #else



            #endif

            slotAccessType_ = slotAccessType::load;

            // Initialise the graphics API/libraries if not done yet.
            if (!graphics::initialised())
                graphics::init(*mainWindow_);

            // Load player system.
            player::init(FOV_, zNear_, zFar_, *mainWindow_, blockReachRange_);
            playerCamera_ = &player::getCamera();

            // Init user control input system.
            input::init();
            inputFunctions::init();

            // Input functions registration.
            input::setControlAction(controlCode::space, inputFunctions::moveUp);
            input::setControlAction(controlCode::leftShift, inputFunctions::moveDown);
            input::setControlAction(controlCode::w, inputFunctions::moveNorth);
            input::setControlAction(controlCode::s, inputFunctions::moveSouth);
            input::setControlAction(controlCode::a, inputFunctions::moveEast);
            input::setControlAction(controlCode::d, inputFunctions::moveWest);
            input::setControlAction(controlCode::r, inputFunctions::switchComplexLighting, false);


            // Load model system.
            models::init();

            // Custom model loading.
            models::loadCustomModel("Resources/Models/Warden.obj", 2);

            // Load chunk system and chunk management system if not loaded.
            if (!chunkManager::initialised()) {
                
                chunk::init();
                chunkManager::init();
            
            }

            // Load entity manager system.
            entityManager::init();


            // World settings.
            world::setSkybox(defaultSkybox_);


            // Load texture atlas and configure it.
            blockTextureAtlas_ = new texture("Resources/Textures/atlas.png");
            texture::setBlockAtlas(*blockTextureAtlas_);
            texture::setBlockAtlasResolution(16);


            // Load shaders.
            defaultShader_ = new shader("Resources/Shaders/vertexShader.shader", "Resources/Shaders/fragmentShader.shader");


            // Load graphics API data structures.
            vbo_ = new vertexBuffer();
            va_ = new vertexArray();
            layout_ = new vertexBufferLayout();
            renderer_ = new renderer();


            /*
            GUI initialization and GUI elements registration.
            WARNING. The engine currently only supports initialization of GUIElements
            before the main menu loop starts for the first time in the game's execution.
            */
            GUIManager::init(*mainWindow_, *defaultShader_, *renderer_);

            // Main menu.
            GUIManager::addGUIBox("mainMenu", 0.25, 0.15, 0.5, 0.7, 995, true, GUIContainer::both);
            GUIManager::addGUIButton("mainMenu.loadButton", 0.425, 0.5, 0.15, 0.10, 961, true, GUIContainer::both, "mainMenu", 1);
            GUIManager::addGUIButton("mainMenu.saveButton", 0.425, 0.35, 0.15, 0.10, 993, false, GUIContainer::both, "mainMenu", 1);
            GUIManager::addGUIButton("mainMenu.exitButton", 0.425, 0.20, 0.15, 0.10, 929, true, GUIContainer::both, "mainMenu", 1);
            GUIManager::addGUIButton("mainMenu.newButton", 0.425, 0.35, 0.15, 0.10, 1019, true, GUIContainer::both, "mainMenu", 1);

            // Load menu.
            GUIManager::addGUIBox("loadMenu", 0.25, 0.15, 0.5, 0.7, 1009, false, GUIContainer::both);
            GUIManager::addGUIButton("loadMenu.exitButton", 0.44, 0.18, 0.12, 0.07, 1017, false, GUIContainer::both, "loadMenu", 1);

            // Save menu.
            GUIManager::addGUIBox("saveMenu", 0.25, 0.15, 0.5, 0.7, 1013, false, GUIContainer::both);
            GUIManager::addGUIButton("saveMenu.exitButton", 0.44, 0.18, 0.12, 0.07, 1017, false, GUIContainer::both, "saveMenu", 1);

            // Save slot buttons.
            GUIManager::addGUIButton("saveSlot1", 0.325, 0.555, 0.15, 0.10, 999, false, GUIContainer::both, "", 1);
            GUIManager::addGUIButton("saveSlot2", 0.325, 0.405, 0.15, 0.10, 1001, false, GUIContainer::both, "", 1);
            GUIManager::addGUIButton("saveSlot3", 0.525, 0.555, 0.15, 0.10, 1003, false, GUIContainer::both, "", 1);
            GUIManager::addGUIButton("saveSlot4", 0.525, 0.405, 0.15, 0.10, 1005, false, GUIContainer::both, "", 1);
            GUIManager::addGUIButton("saveSlot5", 0.425, 0.275, 0.15, 0.10, 1007, false, GUIContainer::both, "", 1);

            // Level HUD.
            GUIManager::addGUIBox("blockPreview", 0.1, 0.75, 0.15, 0.15, 1);


            /*
            Set up GUIElements' keys and key functions.
            */

            // Main Menu/Level.
            GUIManager::bindActKeyFunction("mainMenu", GUIFunctions::changeStateLevelMenu, controlCode::e);
            GUIManager::bindActMouseButtonFunction("mainMenu.loadButton", GUIFunctions::showLoadMenu, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("mainMenu.saveButton", GUIFunctions::showSaveMenu, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("mainMenu.exitButton", GUIFunctions::exit, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("mainMenu.newButton", GUIFunctions::enterNewLevel, controlCode::leftButton);

            // Load menu.
            GUIManager::bindActMouseButtonFunction("loadMenu.exitButton", GUIFunctions::hideLoadMenu, controlCode::leftButton);

            // Save menu.
            GUIManager::bindActMouseButtonFunction("saveMenu.exitButton", GUIFunctions::hideSaveMenu, controlCode::leftButton);

            // Save slot buttons.
            GUIManager::bindActMouseButtonFunction("saveSlot1", GUIFunctions::accessSaveSlot, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("saveSlot2", GUIFunctions::accessSaveSlot, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("saveSlot3", GUIFunctions::accessSaveSlot, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("saveSlot4", GUIFunctions::accessSaveSlot, controlCode::leftButton);
            GUIManager::bindActMouseButtonFunction("saveSlot5", GUIFunctions::accessSaveSlot, controlCode::leftButton);


            // Finish connecting some objects.
            mainWindow_->playerCamera() = playerCamera_;


            // Set up GLFW callbacks.
            graphics::setMainWindow(mainWindow_);

            glfwSetMouseButtonCallback(mainWindow_->windowAPIpointer(), player::mouseButtonCallback);
            glfwSetWindowSizeCallback(mainWindow_->windowAPIpointer(), window::windowSizeCallback);


            // Configure the vertex layout for 3D rendering.
            layout_->push<GLfloat>(3);
            layout_->push<GLfloat>(2);
            layout_->push<normalVec>(1);


            // Bind the currently used VAO, shaders and atlases for 3D rendering.
            vbo_->bind();
            va_->bind();
            va_->addLayout(*layout_);
            defaultShader_->bind();
            blockTextureAtlas_->bind();

            graphicalModeInitialised_ = true;
        
        }
    
    }

    void game::mainLoop() {
    
        do {

            switch (loopSelection_) {

            case AIMENULOOP:  // AI game menu.

                aiMenuLoop();

                break;

            case GRAPHICALMENU: // Main menu/game loop.

                mainMenuLoop();

                break;

            case GRAPHICALLEVEL: // Level game loop.

                gameLoop(false);

                break;

            }

        } while (loopSelection_ != EXIT);
    
    }

    void game::aiMenuLoop() {
    
        if (loopSelection_ == AIMENULOOP) {
        
            unsigned int nGames = 0,
                chosenOption = 0;

            logger::say("AI menu. Please select one of the following options.");
            nGames = AIAPI::aiGame::listAIGames();
            logger::say(std::to_string(nGames + 1) + "). Enter level editor mode");
            logger::say(std::to_string(nGames + 2) + "). Exit");

            do {

                while (!validatedCinInput<unsigned int>(chosenOption) || chosenOption == 0 || chosenOption > nGames + 2)
                    logger::say("Invalid option. Please try again");

                if (chosenOption <= nGames) {

                    AIAPI::aiGame::selectGame(chosenOption);
                    AIAPI::aiGame::startGame();

                    // Once back from the selected game, display options again.
                    logger::say("AI menu. Please select one of the following options.");
                    nGames = AIAPI::aiGame::listAIGames();
                    logger::say(std::to_string(nGames + 1) + "). Enter level editor mode");
                    logger::say(std::to_string(nGames + 2) + "). Exit");

                }

                if (chosenOption == nGames + 1)
                    loopSelection_ = GRAPHICALMENU;
                else
                    loopSelection_ = EXIT;

            } while (loopSelection_ == AIMENULOOP);

        }

    }

    void game::mainMenuLoop() {

        if (game::loopSelection_ == GRAPHICALMENU) {
        
            if (!graphicalModeInitialised_)
                initGraphicalMode();

            // Set 3D rendering mode uniforms.
            defaultShader_->setUniform1i("u_renderMode", 1);
            graphics::setDepthTest(false);

            /*
            Rendering loop.
            */
            unsigned int nVertices = 0;
            while (game::loopSelection_ == GRAPHICALMENU) {

                // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
                // after the player has stopped resizing the window.
                if (mainWindow_->wasResized())
                    mainWindow_->resizeHeavyProcessing();

                // Clear the screen to draw the next frame.
                renderer_->clear();


                /*
                2D rendering.
                */
                GUIManager::drawGUI(true);

                // Swap front and back buffers.
                glfwSwapBuffers(mainWindow_->windowAPIpointer());


                /*
                Event handling
                */

                // Poll for and process Graphic API events.
                glfwPollEvents();

                // Handle user input.
                input::handleInputs();

            }
        
        }

    }

    void game::goToGraphicalMenu() {

        logger::debugLog("Returning to main menu from a level");

        game::loopSelection_ = GRAPHICALMENU;

        GUIManager::changeGUIState("mainMenu.saveButton");
        GUIManager::changeGUIState("mainMenu.newButton");
        GUIManager::changeGUIState("mainMenu.loadButton");

    }

    void game::goToAIMenu() {
    
        game::cleanUpGraphicalMode();
        game::loopSelection_ = AIMENULOOP;
    
    }

    void game::gameLoop(bool playingAIRecord, const std::string& terrainFile) {
    
        if (game::loopSelection_ == GRAPHICALLEVEL) {
        
            if (!graphicalModeInitialised_)
                initGraphicalMode();

            // Configure game window's settings.
            mainWindow_->changeStateMouseLock(false);


            // DEBUG testing world generators.
            if (!worldGen::isGenRegistered("miningWorldGen")) {
            
                worldGen::registerGen<AIExample::miningWorldGen>("miningWorldGen");
                worldGen::selectGen("miningWorldGen");
            
            }
            
            /*
            Level loading.
            */

            // Start the terrain management and
            // the player input processing threads.
            // Also load world.
            std::thread* chunkManagementThread = nullptr;
            std::thread playerInputThread(&player::processSelectionRaycast),
                        entityManagementThread(&entityManager::manageEntities, ref(timeStep_));
            if (chunkManager::infiniteWorld())
                chunkManagementThread = new std::thread(&chunkManager::manageChunks, nMeshingThreads_);
            else
                chunkManagementThread = new std::thread(&chunkManager::finiteWorldLoading, terrainFile);

            // If playing an AI record, set up the entity that plays the recording ticks.
            if (playingAIRecord)
                entityManager::registerEntity(0, vec3Zero, vec3Zero, TickFunctions::playRecordTick);


            // Set shader options.
            vec3 lightpos(10.0f, 150.0f, -10.0f);
            defaultShader_->setUniformVec3f("u_sunLightPos", lightpos);
            defaultShader_->setUniform1i("u_useComplexLighting", 0);


            /*
            Rendering loop.
            */

            // Time/FPS related stuff.
            double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
                   lastFrameTime = lastSecondTime,
                   actualTime;
            int nFramesDrawn = 0; 
            unsigned int nVertices = 0;
            while (game::loopSelection_ == GRAPHICALLEVEL) {

                // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
                // after the player has stopped resizing the window.
                if (mainWindow_->wasResized())
                    mainWindow_->resizeHeavyProcessing();

                if (!mainWindow_->isMouseFree())
                    playerCamera_->updatePos(game::timeStep());
                playerCamera_->updateView();

                MVPmatrix_ = playerCamera_->projectionMatrix() * playerCamera_->viewMatrix();
                defaultShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
                defaultShader_->setUniformVec3f("u_viewPos", playerCamera_->pos());

                // Times calculation.
                actualTime = glfwGetTime();
                timeStep_ = actualTime - lastFrameTime;
                lastFrameTime = actualTime;

                // Clear the screen to draw the next frame.
                renderer_->clear();

                // ms/frame calculation and display.
                nFramesDrawn++;
                if (actualTime - lastSecondTime >= 1.0) {

                    //std::cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame" << " and resolution is " << mainWindow_.width() << " x " << mainWindow_.height();
                    nFramesDrawn = 0;
                    lastSecondTime = glfwGetTime();

                }


                /*
                3D rendering.
                */
                defaultShader_->setUniform1i("u_renderMode", 0); // renderMode = 0 stands for 3D rendering mode.

                // Coordinate rendering thread and chunk management thread.
                if (chunkManager::managerThreadMutex().try_lock()) {

                    if (chunkManager::forceSyncFlag())
                        chunkManager::updatePriorityChunks();
                    else {

                        chunkManager::swapDrawableChunksLists();
                        chunksToDraw_ = chunkManager::drawableChunksRead();

                    }

                    chunkManager::managerThreadMutex().unlock();
                    chunkManager::managerThreadCV().notify_one();

                }

                // Coordinate rendering thread and entity management thread.
                if (entityManager::syncMutex().try_lock()) {

                    entityManager::swapReadWrite();
                    batchesToDraw_ = entityManager::renderingData();

                    entityManager::syncMutex().unlock();
                    entityManager::entityManagerCV().notify_one();

                }

                vbo_->bind();
                va_->bind();

                // Render chunks.
                if (chunksToDraw_) {

                    // chunk.first refers to the chunk's postion.
                    // chunk.second refers to the chunk's vertex data.
                    for (auto const& chunk : *chunksToDraw_) {

                        if (nVertices = chunk.second.size()) {

                            vbo_->prepareStatic(chunk.second.data(), sizeof(vertex) * nVertices);

                            renderer_->draw3D(nVertices);

                        }

                    }

                }

                // Render batches.
                if (batchesToDraw_) {

                    for (auto const& batch : *batchesToDraw_) {

                        if (nVertices = batch->size()) {

                            std::cout << "\r" << nVertices;

                            vbo_->prepareStatic(batch->data(), sizeof(vertex) * nVertices);

                            renderer_->draw3D(nVertices);

                        }

                    }

                }


                /*
                2D rendering.
                */
                graphics::setDepthTest(false);
                defaultShader_->setUniform1i("u_renderMode", 1);

                GUIManager::drawGUI();

                graphics::setDepthTest(true);

                // Swap front and back buffers.
                glfwSwapBuffers(mainWindow_->windowAPIpointer());

                // Poll for and process events.
                glfwPollEvents();

                // Handle user inputs.
                input::handleInputs();

            }


            /*
            Exit level procedure.
            */

            // Notify the chunk management and the high priority update threads that the game is closing.
            // In case a thread is waiting on a corresponding condition variable, send a notification to unblock it.
            {

                std::unique_lock<std::mutex> lock(chunkManager::managerThreadMutex());

                std::unique_lock<std::mutex> syncLock(entityManager::syncMutex());

            }

            chunkManager::managerThreadCV().notify_one();
            entityManager::entityManagerCV().notify_one();
            entityManager::cleanUp();

            chunkManagementThread->join();
            playerInputThread.join();
            entityManagementThread.join();

            chunk::cleanUp();
            chunkManager::cleanUp();
        
        }
    
    }

    void game::enterLevel() {

        // Initialise chunkManager system if necessary.
        if (!chunkManager::initialised()) {
        
            chunkManager::init();
            chunk::init();

        }
        chunkManager::setAImode(false);

        if (!game::selectedSaveSlot())
            worldGen::setSeed();

        game::loopSelection_ = GRAPHICALLEVEL;

    }

    void game::switchComplexLighting() {

        defaultShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
        useComplexLighting_ = !useComplexLighting_;

    }

    void game::cleanUp() {

        // Clear everything related to the engine that is not related to the engine's graphical mode.

        worldGen::cleanUp();

        if (graphicalModeInitialised_)
            cleanUpGraphicalMode();

        initialised_ = false;

    }

    void game::cleanUpGraphicalMode() {

        // Clear everything that is related to the engine's graphical mode.

        GUIManager::setMMGUIChanged(true); // TODO: SEE IF THIS CAN BE REMOVED

        GUIManager::cleanUp();

        player::cleanUp();

        input::cleanUp();
        inputFunctions::cleanUp();

        chunk::cleanUp();

        chunkManager::cleanUp();

        models::cleanUp();

        entityManager::cleanUp();

        graphics::cleanUp();

        if (blockTextureAtlas_)
            delete blockTextureAtlas_;

        if (defaultShader_)
            delete defaultShader_;

        if (vbo_)
            delete vbo_;

        if (va_)
            delete va_;

        if (layout_)
            delete layout_;

        if (renderer_)
            delete renderer_;

        mainWindow_ = nullptr;

        playerCamera_ = nullptr;

        chunksToDraw_ = nullptr;
        batchesToDraw_ = nullptr;

        graphicalModeInitialised_ = false;

    }

}