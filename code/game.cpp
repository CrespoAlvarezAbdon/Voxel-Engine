#include "game.h"
#include <barrier>
#include <deque>
#include <functional>
#include <fstream>
#include <filesystem>
#include <string>
#include <shared_mutex>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "AIAPI.h"
#include "batch.h"
#include "block.h"
#include "camera.h"
#include "entity.h"
#include "player.h"
#include "tickFunctions.h"
#include "texture.h"
#include "graphics.h"
#include "gui.h"
#include "GUIfunctions.h"
#include "input.h"
#include "inputFunctions.h"
#include "renderer.h"
#include "utilities.h"
#include "vertex.h"
#include "worldGen.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#endif


namespace VoxelEng {

    ////////////
    //Classes.//
    ////////////


    // 'game' class.

    bool game::initialised_ = false,
         game::graphicalModeInitialised_ = false,
         game::AImodeON_ = false,
         game::useComplexLighting_ = false;

    window* game::mainWindow_ = nullptr;

    std::thread* game::chunkManagementThread_ = nullptr,
               * game::priorityChunkUpdatesThread_ = nullptr,
               * game::playerInputThread_ = nullptr,
               * game::tickManagementThread_ = nullptr;

    std::atomic<bool> game::threadsExecute[3] = {false, false, false};
    std::atomic<engineMode> game::loopSelection_ = engineMode::AIMENULOOP;
    std::atomic<double> game::timeStep_ = 0.0f;

    skybox game::defaultSkybox_(140, 170, 255, 1.0f);

    unsigned int game::saveSlot_ = 0,
        game::blockReachRange_ = 5,
        game::nMeshingThreads_ = 0;
    float game::FOV_ = 110.0f,
          game::zNear_ = 0.1f,
          game::zFar_ = 500.0f;

    camera* game::playerCamera_ = nullptr;
    texture* game::blockTextureAtlas_ = nullptr;

    std::unordered_map<vec3, model> const* game::chunksToDraw_ = nullptr;
    const std::vector<model>* game::batchesToDraw_ = nullptr;

    shader* game::defaultShader_ = nullptr;
    vertexBuffer* game::chunksVbo_ = nullptr,
                * game::entitiesVbo_ = nullptr;
    vertexArray* game::vao_ = nullptr;

    #if GRAPHICS_API == OPENGL

        glm::mat4 game::MVPmatrix_;

    #else



    #endif


    void game::init() {

        if (initialised_)
            logger::errorLog("Game is already initialised");
        else {

            // Put here any game initialisation that does not involve
            // the engine's graphical mode.

            // Directory creation.
            std::filesystem::create_directory("AIData");

            std::filesystem::create_directory("resources");

            std::filesystem::create_directory("saves");
            std::filesystem::create_directory("saves/slot1");
            std::filesystem::create_directory("saves/slot2");
            std::filesystem::create_directory("saves/slot3");
            std::filesystem::create_directory("saves/slot4");
            std::filesystem::create_directory("saves/slot5");
            std::filesystem::create_directory("saves/recordings");
            std::filesystem::create_directory("saves/recordingWorlds");

            


            // General variables.
            loopSelection_ = engineMode::AIMENULOOP;
            timeStep_ = 0.0f;
            AImodeON_ = false;

            // Block registration.
            block::registerBlock("starminer::grass", 1); // manual texture ID assignemt is temporary
            block::registerBlock("starminer::stone", 2);
            block::registerBlock("starminer::sand", 3);
            block::registerBlock("starminer::marbleBlock", 4);
            block::registerBlock("starminer::dirt", 6);
            block::registerBlock("starminer::coalOre", 7);
            block::registerBlock("starminer::ironOre", 8);
            block::registerBlock("starminer::goldOre", 9);
            block::registerBlock("starminer::diamondOre", 10);

            // Worldgen initialisation.

            worldGen::init();

            initialised_ = true;

        }

    }
    
    void game::initGraphicalMode() {
    
        if (graphicalModeInitialised_)
            logger::errorLog("Engine's graphical mode already initialised");
        else if (AImodeON_)
            logger::errorLog("Graphical mode is not allowed with AI mode turned ON.");
        else {

            mainWindow_ = new window(800, 800, "VoxelEng");

            saveSlot_ = 0;
            blockReachRange_ = 5;
            nMeshingThreads_ = (std::thread::hardware_concurrency() > 8) ? 8 : std::thread::hardware_concurrency();
            
            FOV_ = 110.0f;
            zNear_ = 0.1f;
            zFar_ = 500.0f;

            useComplexLighting_ = false;

            #if GRAPHICS_API == OPENGL

                MVPmatrix_ = glm::mat4();

            #endif

            // Initialise the graphics API/libraries if not done yet.
            if (!graphics::initialised())
                graphics::init(*mainWindow_);

            chunksVbo_ = &graphics::vbo("chunks"),
            entitiesVbo_ = &graphics::vbo("entities");
            vao_ = &graphics::vao("3D");

            // Load model system.
            models::init();

            // Custom model loading.
            models::loadCustomModel("resources/Models/Warden.obj", 2);

            // Load texture atlas and configure it.
            blockTextureAtlas_ = new texture("resources/Textures/atlas.png");
            texture::setBlockAtlas(*blockTextureAtlas_);
            texture::setBlockAtlasResolution(16);

            // World settings.
            world::init();
            world::setSkybox(defaultSkybox_);

            // Load chunk system and chunk management system if not loaded.
            if (!chunk::initialised())
                chunk::init();

            if (!chunkManager::initialised())
                chunkManager::init();

            // Load entity manager system.
            if (!entityManager::initialised())
                entityManager::init();

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
            input::setControlAction(controlCode::q, inputFunctions::rollRight);
            input::setControlAction(controlCode::e, inputFunctions::rollLeft);
            input::setControlAction(controlCode::r, inputFunctions::switchComplexLighting, false);

            // Load shaders.
            defaultShader_ = new shader("resources/Shaders/vertexShader.shader", "resources/Shaders/fragmentShader.shader");


            /*
            GUI initialization and GUI elements registration.
            WARNING. The engine currently only supports initialization of GUIElements
            before the main menu loop starts for the first time in the game's execution.
            */
            GUImanager::init(*mainWindow_, *defaultShader_);

            // Main menu.
            GUImanager::addGUIBox("mainMenu", 0.5, 0.5, 0.3, 0.35, 995, true, GUIcontainer::both);
            GUImanager::addGUIButton("mainMenu.loadButton", 0.5, 0.65, 0.10, 0.05, 961, true, GUIcontainer::both, "mainMenu", 1);
            GUImanager::addGUIButton("mainMenu.saveButton", 0.5, 0.45, 0.10, 0.05, 993, false, GUIcontainer::both, "mainMenu", 1);
            GUImanager::addGUIButton("mainMenu.exitButton", 0.5, 0.25, 0.10, 0.05, 929, true, GUIcontainer::both, "mainMenu", 1);
            GUImanager::addGUIButton("mainMenu.newButton", 0.5, 0.45, 0.10, 0.05, 1019, true, GUIcontainer::both, "mainMenu", 1);

            // Load menu.
            GUImanager::addGUIBox("loadMenu", 0.5, 0.5, 0.3, 0.35, 1009, false, GUIcontainer::both);
            GUImanager::addGUIButton("loadMenu.exitButton", 0.5, 0.1, 0.10, 0.05, 1017, false, GUIcontainer::both, "loadMenu", 1);

            // Save slot buttons.
            GUImanager::addGUIButton("saveSlot1", 0.3, 0.65, 0.10, 0.05, 999, false, GUIcontainer::both, "loadMenu", 1);
            GUImanager::addGUIButton("saveSlot2", 0.7, 0.65, 0.10, 0.05, 1001, false, GUIcontainer::both, "loadMenu", 1);
            GUImanager::addGUIButton("saveSlot3", 0.3, 0.45, 0.10, 0.05, 1003, false, GUIcontainer::both, "loadMenu", 1);
            GUImanager::addGUIButton("saveSlot4", 0.7, 0.45, 0.10, 0.05, 1005, false, GUIcontainer::both, "loadMenu", 1);
            GUImanager::addGUIButton("saveSlot5", 0.5, 0.275, 0.10, 0.05, 1007, false, GUIcontainer::both, "loadMenu", 1);


            /*
            Set up GUIElements' keys and key functions.
            */

            // Main Menu/Level.
            GUImanager::bindActKeyFunction("mainMenu", GUIfunctions::changeStateLevelMenu, controlCode::escape);
            GUImanager::bindActMouseButtonFunction("mainMenu.loadButton", GUIfunctions::showLoadMenu, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("mainMenu.saveButton", GUIfunctions::saveGame, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("mainMenu.exitButton", GUIfunctions::exit, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("mainMenu.newButton", GUIfunctions::enterNewLevel, controlCode::leftButton);

            // Load menu.
            GUImanager::bindActMouseButtonFunction("loadMenu.exitButton", GUIfunctions::hideLoadMenu, controlCode::leftButton);

            // Save slot buttons.
            GUImanager::bindActMouseButtonFunction("saveSlot1", GUIfunctions::loadGame, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("saveSlot2", GUIfunctions::loadGame, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("saveSlot3", GUIfunctions::loadGame, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("saveSlot4", GUIfunctions::loadGame, controlCode::leftButton);
            GUImanager::bindActMouseButtonFunction("saveSlot5", GUIfunctions::loadGame, controlCode::leftButton);


            // Finish connecting some objects.
            mainWindow_->playerCamera() = playerCamera_;

            glfwSetMouseButtonCallback(mainWindow_->windowAPIpointer(), player::mouseButtonCallback);
            glfwSetWindowSizeCallback(mainWindow_->windowAPIpointer(), window::windowSizeCallback);


            // Bind the currently used VAO, shaders and atlases for 3D rendering.
            defaultShader_->bind();
            blockTextureAtlas_->bind();

            graphicalModeInitialised_ = true;
        
        }
    
    }

    void game::mainLoop() {
    
        do {

            switch (loopSelection_) {

            case engineMode::AIMENULOOP:  // AI game menu.

                aiMenuLoop();

                break;

            case engineMode::GRAPHICALMENU: // Main menu/game loop.

                mainMenuLoop();

                break;

            case engineMode::INITLEVEL: // Level game loop.

                gameLoop();

                break;

            default:
                break;

            }

        } while (loopSelection_ != engineMode::EXIT);
    
    }

    void game::aiMenuLoop() {
    
        if (loopSelection_ == engineMode::AIMENULOOP) {
        
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

                    AIAPI::aiGame::selectGame(chosenOption - 1);
                    AIAPI::aiGame::startGame();

                    // Once back from the selected game, display options again.
                    logger::say("AI menu. Please select one of the following options.");
                    nGames = AIAPI::aiGame::listAIGames();
                    logger::say(std::to_string(nGames + 1) + "). Enter level editor mode");
                    logger::say(std::to_string(nGames + 2) + "). Exit");

                }
                else {

                    if (chosenOption == nGames + 1)
                        setLoopSelection(engineMode::GRAPHICALMENU);
                    else
                        setLoopSelection(engineMode::EXIT);

                }

            } while (loopSelection_ == engineMode::AIMENULOOP);

        }

    }

    void game::mainMenuLoop() {

        if (game::loopSelection_ == engineMode::GRAPHICALMENU) {

            // Set 3D rendering mode uniforms.
            defaultShader_->setUniform1i("u_renderMode", 1);
            graphics::setDepthTest(false);

            /*
            Rendering loop.
            */
            unsigned int nVertices = 0;
            while (game::loopSelection_ == engineMode::GRAPHICALMENU) {

                // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
                // after the player has stopped resizing the window.
                if (mainWindow_->wasResized())
                    mainWindow_->resizeHeavyProcessing();

                // Clear the screen to draw the next frame.
                renderer::clearWindow();


                /*
                2D rendering.
                */
                GUImanager::drawGUI(true);

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

    void game::gameLoop(const std::string& terrainFile) {
        
        if (loopSelection_ == engineMode::INITLEVEL || loopSelection_ == engineMode::INITRECORD) {

            if (!graphicalModeInitialised_)
                initGraphicalMode();

            chunkManager::setNChunksToCompute(DEF_N_CHUNKS_TO_COMPUTE);

            /*
            Level loading.
            */
            
            // Start the terrain management and loading of the world.
            if (chunkManagementThread_)
                delete chunkManagementThread_;
            if (priorityChunkUpdatesThread_)
                delete priorityChunkUpdatesThread_;
            chunkManagementThread_ = new std::thread(&chunkManager::manageChunks);
            priorityChunkUpdatesThread_ = new std::thread(&chunkManager::manageChunkPriorityUpdates);

            if (loopSelection_ == engineMode::INITRECORD) {

                if (!GUImanager::isLevelGUIElementRegistered("pauseIcon")) {

                    GUImanager::addGUIBox("pauseIcon", 0.15, 0.85, 0.1, 0.1, 1021);
                    input::setControlAction(controlCode::rightArrow, inputFunctions::recordForward, false);
                    input::setControlAction(controlCode::downArrow, inputFunctions::recordPause, false);
                    input::setControlAction(controlCode::leftArrow, inputFunctions::recordBackwards, false);
                    input::setControlAction(controlCode::x, inputFunctions::exitRecord, false);

                    world::addGlobalTickFunction("playRecordTick", TickFunctions::playRecordTick);

                }

                // Things to apply when the terrain is loaded.
                chunkManager::waitInitialTerrainLoaded();

                setLoopSelection(engineMode::PLAYINGRECORD);

            }
            else {

                if (!GUImanager::isLevelGUIElementRegistered("blockPreview")) {
                
                    GUImanager::addGUIBox("blockPreview", 0.15, 0.85, 0.1, 0.1, 1);
                    input::setControlAction(controlCode::alpha1, inputFunctions::selectBlockSlot1, false);
                    input::setControlAction(controlCode::alpha2, inputFunctions::selectBlockSlot2, false);
                    input::setControlAction(controlCode::alpha3, inputFunctions::selectBlockSlot3, false);
                    input::setControlAction(controlCode::alpha4, inputFunctions::selectBlockSlot4, false);
                    input::setControlAction(controlCode::alpha5, inputFunctions::selectBlockSlot5, false);
                    input::setControlAction(controlCode::alpha6, inputFunctions::selectBlockSlot6, false);
                    input::setControlAction(controlCode::alpha7, inputFunctions::selectBlockSlot7, false);
                    input::setControlAction(controlCode::alpha8, inputFunctions::selectBlockSlot8, false);
                    input::setControlAction(controlCode::alpha9, inputFunctions::selectBlockSlot9, false);
                    input::setControlAction(controlCode::p, inputFunctions::intentionalCrash, false);

                }

                // Things to apply when the terrain is loaded.
                chunkManager::waitInitialTerrainLoaded();

                setLoopSelection(engineMode::EDITLEVEL);

            }

            // Start threads that require the world to be loaded first.
            if (!AIAPI::aiGame::playingRecord())
                playerInputThread_ = new std::thread(&player::processSelectionRaycast);
            tickManagementThread_ = new std::thread(&world::processWorldTicks);


            /*
            Rendering loop.
            */

            // Configure game window's settings.
            mainWindow_->changeStateMouseLock(false);

            // Set shader options.
            vec3 lightpos{ 10.0f, 150.0f, -10.0f };
            defaultShader_->setUniformVec3f("u_sunLightPos", lightpos);
            defaultShader_->setUniform1i("u_useComplexLighting", 0);

            // Time/FPS related stuff.
            double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
                   lastFrameTime = lastSecondTime,
                   actualTime;
            int nFramesDrawn = 0; 
            unsigned int nVertices = 0;
            while (loopSelection_ == engineMode::EDITLEVEL || loopSelection_ == engineMode::PLAYINGRECORD) {

                // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
                // after the player has stopped resizing the window.
                if (mainWindow_->wasResized())
                    mainWindow_->resizeHeavyProcessing();

                if (!mainWindow_->isMouseFree())
                    player::updateTransform(game::timeStep());
                playerCamera_->updateView();

                MVPmatrix_ = playerCamera_->projectionMatrix() * playerCamera_->viewMatrix();
                defaultShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
                defaultShader_->setUniformVec3f("u_viewPos", playerCamera_->globalPos());

                // Times calculation.
                actualTime = glfwGetTime();
                timeStep_ = actualTime - lastFrameTime;
                lastFrameTime = actualTime;

                // Clear the window to draw the next frame.
                renderer::clearWindow();

                // ms/frame calculation and display.
                nFramesDrawn++;
                if (actualTime - lastSecondTime >= 1.0) {

                    //std::cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame";
                    nFramesDrawn = 0;
                    lastSecondTime = glfwGetTime();

                }

                /*
                3D rendering.
                */
                defaultShader_->setUniform1i("u_renderMode", 0); // renderMode = 0 stands for 3D rendering mode.
                vao_->bind();
                chunksVbo_->bind();

                // Receive updated chunk meshes when possible.
                if (chunkManager::priorityManagerThreadMutex().try_lock()) {
                
                    chunkManager::swapChunkMeshesBuffers();
                    chunksToDraw_ = chunkManager::drawableChunksRead();

                    chunkManager::priorityManagerThreadMutex().unlock();
                    chunkManager::priorityManagerThreadCV().notify_one();
                
                }
                else if (chunkManager::managerThreadMutex().try_lock()) {

                    chunkManager::swapChunkMeshesBuffers();
                    chunksToDraw_ = chunkManager::drawableChunksRead(); 

                    chunkManager::managerThreadMutex().unlock();
                    chunkManager::managerThreadCV().notify_one();

                }

                // Coordinate rendering thread and the thread in charge of generating entity render data if necessary.
                if (entityManager::syncMutex().try_lock()) {

                    entityManager::swapReadWrite();
                    batchesToDraw_ = entityManager::renderingData();

                    entityManager::syncMutex().unlock();
                    entityManager::entityManagerCV().notify_one();

                }

                // Render chunks.
                if (chunksToDraw_) {

                    // chunk.first refers to the chunk's postion.
                    // chunk.second refers to the chunk's vertex data.
                    for (auto const& chunk : *chunksToDraw_) {

                        if (nVertices = chunk.second.size()) {

                            chunksVbo_->setDynamicData(chunk.second.data(), 0, nVertices * sizeof(vertex));

                            renderer::draw3D(nVertices);

                        }

                    }

                }

                entitiesVbo_->bind();

                // Render batches.
                if (batchesToDraw_) {

                    for (auto const& batch : *batchesToDraw_) {

                        if (nVertices = batch.size()) {

                            entitiesVbo_->prepareStatic(batch.data(), sizeof(vertex) * nVertices);

                            renderer::draw3D(nVertices);

                        }

                    }

                }


                /*
                2D rendering.
                */
                graphics::setDepthTest(false);
                defaultShader_->setUniform1i("u_renderMode", 1);

                GUImanager::drawGUI();

                graphics::setDepthTest(true);

                // Swap front and back buffers.
                glfwSwapBuffers(mainWindow_->windowAPIpointer());

                // Poll for and process events.
                glfwPollEvents();

                // Handle user inputs.
                input::handleInputs();

            }

        }
    
    }

    void game::setLoopSelection(engineMode mode) {
    
        switch (loopSelection_) {

            case engineMode::AIMENULOOP:

                switch (mode) {
            
                    case engineMode::EXIT:

                        input::inputMutex().lock(); // Do not accept input until all is cleared.
                        reset();

                        loopSelection_ = engineMode::EXIT;
                        input::inputMutex().unlock();

                        break;

                    case engineMode::AIMENULOOP:
                        break;

                    case engineMode::GRAPHICALMENU:

                        setAImode(false);

                        initGraphicalMode();

                        loopSelection_ = engineMode::GRAPHICALMENU;

                        break;

                    case engineMode::INITRECORD:

                        setAImode(false);

                        initGraphicalMode();

                        loopSelection_ = engineMode::INITRECORD;
                        threadsExecute[0] = true;
                        threadsExecute[1] = true;
                        threadsExecute[2] = true;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");
            
                }

                break;

            case engineMode::GRAPHICALMENU:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::AIMENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::AIMENULOOP:

                        resetGraphicalMode();
                        setAImode(true);

                        if (!chunkManager::openedTerrainFileName().empty())
                            chunkManager::openedTerrainFileName("");

                        loopSelection_ = VoxelEng::engineMode::AIMENULOOP;

                        break;

                    case engineMode::GRAPHICALMENU:
                        break;

                    case engineMode::INITLEVEL:

                        threadsExecute[0] = true;
                        threadsExecute[1] = true;
                        threadsExecute[2] = true;
                        loopSelection_ = engineMode::INITLEVEL;
                        
                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");

                }

                break;

            case engineMode::INITLEVEL:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::GRAPHICALMENU);
                        setLoopSelection(engineMode::EXITLEVEL);
                        setLoopSelection(engineMode::AIMENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::INITLEVEL:
                        break;

                    case engineMode::EDITLEVEL:

                        loopSelection_ = engineMode::EDITLEVEL;

                        break;

                    case engineMode::EXITLEVEL:

                        stopAuxiliaryThreads();
                        loopSelection_ = engineMode::EXITLEVEL;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");

                }

                break;

            case engineMode::EDITLEVEL:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::EXITLEVEL);
                        setLoopSelection(engineMode::GRAPHICALMENU);
                        setLoopSelection(engineMode::AIMENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::EDITLEVEL:
                        break;

                    case engineMode::EXITLEVEL:

                        stopAuxiliaryThreads();
                        chunkManager::clear();
                        resetLevel();
                        loopSelection_ = engineMode::EXITLEVEL;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");

                }

                break;

            case engineMode::EXITLEVEL:

                switch (mode) {
                
                    case engineMode::GRAPHICALMENU:

                        loopSelection_ = engineMode::GRAPHICALMENU;

                        break;

                    case engineMode::INITLEVEL:

                        threadsExecute[0] = true;
                        threadsExecute[1] = true;
                        threadsExecute[2] = true;
                        loopSelection_ = engineMode::INITLEVEL;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");
                
                }

                break;

            case engineMode::INITRECORD:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::EXITRECORD);
                        setLoopSelection(engineMode::AIMENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::AIMENULOOP:

                        resetLevel();
                        resetGraphicalMode();
                        setAImode(true);

                        if (!chunkManager::openedTerrainFileName().empty())
                            chunkManager::openedTerrainFileName("");

                        loopSelection_ = engineMode::AIMENULOOP;

                        break;

                    case engineMode::INITRECORD:
                        break;

                    case engineMode::PLAYINGRECORD:

                        loopSelection_ = engineMode::PLAYINGRECORD;

                        break;

                    case engineMode::EXITRECORD:

                        game::stopAuxiliaryThreads();
                        loopSelection_ = engineMode::EXITRECORD;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");

                }

                break;

            case engineMode::PLAYINGRECORD:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::EXITRECORD);
                        setLoopSelection(engineMode::AIMENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::PLAYINGRECORD:
                        break;

                    case engineMode::EXITRECORD:

                        game::stopAuxiliaryThreads();
                        loopSelection_ = engineMode::EXITRECORD;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");

                }

                break;

            case engineMode::EXITRECORD:

                switch (mode) {

                    case engineMode::AIMENULOOP:

                        resetLevel();
                        resetGraphicalMode();
                        setAImode(true);

                        if (!chunkManager::openedTerrainFileName().empty())
                            chunkManager::openedTerrainFileName("");

                        loopSelection_ = engineMode::AIMENULOOP;

                        break;
                
                    default:
                        logger::errorLog("Unsupported engine mode transition");
                
                }

                break;

            default:
                logger::errorLog("Unspecified current engine mode selected");
        
        }
    
    }

    void game::switchComplexLighting() {

        defaultShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
        useComplexLighting_ = !useComplexLighting_;

    }

    void game::setAImode(bool ON) {

        engineMode mode = game::selectedEngineMode();
        if (mode == engineMode::EDITLEVEL)
            logger::errorLog("Cannot change chunkManager's AI mode while in a level");
        else if (mode == engineMode::PLAYINGRECORD)
            logger::errorLog("Cannot change chunkManager's AI mode while playing a record");
        else
            AImodeON_ = ON;

        entityManager::setAImode(AImodeON_);

    }

    void game::stopAuxiliaryThreads() {
        
        if (chunkManagementThread_ && priorityChunkUpdatesThread_ && threadsExecute[2]) {

            // Notify the chunk management and the high priority update threads to stop.
            // In case a thread is waiting on its corresponding condition variable, send a notification to unblock it.
            threadsExecute[2] = false;
            {

                std::unique_lock<std::mutex> lock(chunkManager::managerThreadMutex());
                chunkManager::managerThreadCV().notify_all();

            }

            chunkManagementThread_->join();
            delete chunkManagementThread_;
            chunkManagementThread_ = nullptr;

            {

                chunkManager::priorityNewChunkMeshesCV().notify_all();
                std::unique_lock<std::mutex> priorityUpdatesLock(chunkManager::priorityManagerThreadMutex());
                chunkManager::priorityManagerThreadCV().notify_all();
                
            }

            priorityChunkUpdatesThread_->join();
            delete priorityChunkUpdatesThread_;
            priorityChunkUpdatesThread_ = nullptr;

        }

        if (playerInputThread_ && threadsExecute[0]) {

            {

                threadsExecute[0] = false;
                std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());

            }

            playerInputThread_->join();
            delete playerInputThread_;
            playerInputThread_ = nullptr;

        }

        if (tickManagementThread_ && threadsExecute[1]) {

            // Notify the tick management thread to stop.
            // In case it is waiting on its corresponding condition variable, send a notification to unblock it.
            {
            
                threadsExecute[1] = false;
                std::unique_lock<std::mutex> syncLock(entityManager::syncMutex());
            
            }
            entityManager::entityManagerCV().notify_one();

            tickManagementThread_->join();
            delete tickManagementThread_;
            tickManagementThread_ = nullptr;

        }
    
    }

    void game::resetLevel() {
    
        worldGen::clear();
        
    }

    void game::reset() {

        std::unique_lock<std::recursive_mutex> lock(input::inputMutex());
        input::shouldProcessInputs(false);

        stopAuxiliaryThreads();

        // Clear everything related to the engine that is not related to the engine's graphical mode.

        if (worldGen::initialised())
            worldGen::reset();

        if (graphicalModeInitialised_)
            resetGraphicalMode();

        initialised_ = false;

        input::shouldProcessInputs(true);

        if (block::initialised())
            block::reset();

    }

    void game::resetGraphicalMode() {

        std::unique_lock<std::recursive_mutex> lock(input::inputMutex());

        // Deallocate everything that is related to the engine's graphical mode.

        GUImanager::setMMGUIChanged(true);

        GUImanager::reset();

        player::reset();

        input::reset();

        inputFunctions::reset();

        chunk::reset();

        chunkManager::reset();

        models::reset();

        entityManager::reset();

        world::reset();

        graphics::reset();

        if (blockTextureAtlas_) {
        
            delete blockTextureAtlas_;
            blockTextureAtlas_ = nullptr;
        
        }

        if (defaultShader_) {

            delete defaultShader_;
            defaultShader_ = nullptr;

        }

        // These pointers where not the original owners of the object they point to.
        // They were allocated in the 'graphics' class and they have been properly deallocated before this.
        chunksVbo_ = nullptr;
        entitiesVbo_ = nullptr;
        vao_ = nullptr;
        playerCamera_ = nullptr;
        chunksToDraw_ = nullptr;

        if (mainWindow_) {

            delete mainWindow_;
            mainWindow_ = nullptr;

        }

       

        if (batchesToDraw_) {

            delete batchesToDraw_;
            batchesToDraw_ = nullptr;

        }

        graphicalModeInitialised_ = false;

    }

}