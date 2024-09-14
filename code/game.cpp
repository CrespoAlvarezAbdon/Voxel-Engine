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
#include <utility>

#include "AIAPI.h"
#include "batch.h"
#include "block.h"
#include "camera.h"
#include "entity.h"
#include "player.h"
#include "tickFunctions.h"
#include "gui.h"
#include "GUIfunctions.h"
#include "input.h"
#include "inputFunctions.h"
#include "renderer.h"
#include "utilities.h"
#include "vertex.h"
#include "worldGen.h"
#include "Entities/plane.h"
#include "Registry/registries.h" // This header also includes the classes that derive from 'registeredElement'.
#include "Registry/registry.h"
#include "Graphics/graphics.h"
#include "Graphics/Textures/texture.h"
#include "Graphics/Frustum/frustum.h"

#include "timer.h"


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

    std::unordered_map<vec3, chunkRenderingData> const* game::chunksToDraw_ = nullptr;
    const std::vector<model>* game::batchesToDraw_ = nullptr;

    shader* game::opaqueShader_ = nullptr;
    shader* game::translucidShader_ = nullptr;
    shader* game::compositeShader_ = nullptr;
    shader* game::screenShader_ = nullptr;
    vertexBuffer* game::chunksVbo_ = nullptr;
    vertexBuffer* game::entitiesVbo_ = nullptr;
    vertexBuffer* game::screenVbo_ = nullptr;
    vertexArray* game::vao_ = nullptr;
    vertexArray* game::entitiesVao_ = nullptr;
    vertexArray* game::screenVao_ = nullptr;
    
    framebuffer* game::opaqueFB_ = nullptr;
    framebuffer* game::translucidFB_ = nullptr;
    framebuffer* game::screenFB_ = nullptr;

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

            // Registrable elements initialisation.
            registryElement::init("registryElement");
            material::init("material");

            // Registries collection initialisation.
            registries::init();

            // Register materials.
            registryInsOrdered<std::string, material>& materialsRegistry = registries::materials();
            materialsRegistry.insert("OmegaRed",
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                32.0f);

            materialsRegistry.insert("AlphaBlue",
                0.0f, 0.0f, 10.0f,
                0.0f, 0.0f, 10.0f,
                0.0f, 0.0f, 10.0f,
                32.0f);

            materialsRegistry.insert("DeltaGreen",
                0.0f, 10.0f, 0.0f,
                0.0f, 10.0f, 0.0f,
                0.0f, 10.0f, 0.0f,
                32.0f);

            materialsRegistry.insert("RedOnlyIfLit",
                1.0f, 1.0f, 1.0f,
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                32.0f);

            materialsRegistry.insert("UltraShiny",
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                256.0f);

            // Block registration.
            block::registerBlock("starminer::grass", blockOpacity::OPAQUEBLOCK, { {"all", 1} }, "UltraShiny"); // TODO. Manual texture ID assignment is temporary.
            block::registerBlock("starminer::stone", blockOpacity::OPAQUEBLOCK, { {"all", 2} });
            block::registerBlock("starminer::sand", blockOpacity::OPAQUEBLOCK, { {"all", 3} });
            block::registerBlock("starminer::marbleBlock", blockOpacity::OPAQUEBLOCK, { {"all", 4}}, "AlphaBlue");
            block::registerBlock("starminer::dirt", blockOpacity::OPAQUEBLOCK, { {"all", 6} });
            block::registerBlock("starminer::coalOre", blockOpacity::OPAQUEBLOCK, { {"all", 7} });
            block::registerBlock("starminer::ironOre", blockOpacity::OPAQUEBLOCK, { {"all", 8} });
            block::registerBlock("starminer::goldOre", blockOpacity::OPAQUEBLOCK, { {"all", 9} });
            block::registerBlock("starminer::diamondOre", blockOpacity::OPAQUEBLOCK, { {"all", 10}});
            block::registerBlock("starminer::log", blockOpacity::OPAQUEBLOCK, { {"all", 11}, {"faceY+", 12}, {"faceY-", 12}});
            block::registerBlock("starminer::glass", blockOpacity::FULLTRANSPARENT, { {"all", 13} });
            block::registerBlock("starminer::glassRed", blockOpacity::TRANSLUCENTBLOCK, { {"all", 14} }, "DeltaGreen");
            block::registerBlock("starminer::glassBlue", blockOpacity::TRANSLUCENTBLOCK, { {"all", 15} });
            block::registerBlock("starminer::marbleBlock2", blockOpacity::OPAQUEBLOCK, { {"all", 16} }, "OmegaRed");

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
            screenVbo_ = &graphics::vbo("screen");
            vao_ = &graphics::vao("3D");
            entitiesVao_ = &graphics::vao("3Dentities");
            screenVao_ = &graphics::vao("screen");

            // Load model system.
            models::init();

            // Custom model loading.
            models::loadCustomModel("resources/Models/Warden.obj", 2);

            // Create framebuffers.
            opaqueFB_ = new framebuffer(mainWindow_->width(), mainWindow_->height(), {textureType::COLOR, textureType::DEPTH_AND_STENCIL});
            translucidFB_ = new framebuffer(mainWindow_->width(), mainWindow_->height(), {textureType::COLOR, textureType::COLOR});

            translucidFB_->bind();
            translucidFB_->pushBack(opaqueFB_->getTexture(textureType::DEPTH_AND_STENCIL, 0));

            screenFB_ = new framebuffer(mainWindow_->width(), mainWindow_->height(), {textureType::COLOR});

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
            opaqueShader_ = &graphics::opaqueShader();
            translucidShader_ = &graphics::translucidShader();
            compositeShader_ = &graphics::compositeShader();
            screenShader_ = &graphics::screenShader();


            /*
            GUI initialization and GUI elements registration.
            WARNING. The engine currently only supports initialization of GUIElements
            before the main menu loop starts for the first time in the game's execution.
            */
            GUImanager::init(*mainWindow_, *opaqueShader_);

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


            // Bind the default shaders and atlases for 3D and 2D rendering before entering a level.
            opaqueShader_->bind();
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

            framebuffer::unbindAll();
            opaqueShader_->bind();
            blockTextureAtlas_->bind();

            // Set 3D rendering mode uniforms.
            opaqueShader_->setUniform1i("u_renderMode", 1);
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

            // NEXT HAY UNO DE LOS EJES DEL LOD 1_2 O DEL 2 QUE TIENE UN FALLITO DE COPIADO SEGURAMENTE. ES EL EJE X O EL Z.

            /*
            Rendering loop.
            */

            // Configure game window's settings.
            mainWindow_->changeStateMouseLock(false);

            // Set shader options.
            vec3 lightpos{ 10.0f, 150.0f, -10.0f };
            opaqueShader_->setUniformVec3f("u_sunLightPos", lightpos);
            opaqueShader_->setUniform1i("u_useComplexLighting", 0);

            // Frame buffer things go here.
            float screenShaderQuad[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
            };

            // Time/FPS related stuff.
            double lastSecondTime = glfwGetTime(), // How much time has passed since the last second passed.
                   lastFrameTime = lastSecondTime,
                   actualTime;
            int nFramesDrawn = 0; 
            unsigned int nVertices = 0;
            unsigned int nTranslucentVertices = 0;

            // LOD related stuff.
            bool inLODborder = false;
            blockViewDir dirX = blockViewDir::NONE;
            blockViewDir dirY = blockViewDir::NONE;
            blockViewDir dirZ = blockViewDir::NONE;

            // Spawn test entities here.
            
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //TODO. ADD THIS AS AN OPTION.
            while (loopSelection_ == engineMode::EDITLEVEL || loopSelection_ == engineMode::PLAYINGRECORD) {

                graphics::setDepthTest(true);
                graphics::setOpaquePassConfig();

                opaqueFB_->bind();
                opaqueFB_->clearAllTextures();

                opaqueShader_->bind();
                opaqueShader_->setUniform1i("blockTexture", 0);
                opaqueShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
                blockTextureAtlas_->bind();

                // The window size callback by GLFW gets called every time the user is resizing the window so the heavy resize processing is done here
                // after the player has stopped resizing the window.
                if (mainWindow_->wasResized())
                    mainWindow_->resizeHeavyProcessing();

                if (!mainWindow_->isMouseFree())
                    player::updateTransform(game::timeStep());
                playerCamera_->updateView();

                MVPmatrix_ = playerCamera_->projectionMatrix() * playerCamera_->viewMatrix();
                opaqueShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
                opaqueShader_->setUniformVec3f("u_viewPos", playerCamera_->globalPos());

                //lightpos = playerCamera_->globalPos();
                opaqueShader_->setUniformVec3f("u_sunLightPos", lightpos);


                /*
                3D rendering.
                */
                opaqueShader_->setUniform1i("u_renderMode", 0); // renderMode = 0 stands for 3D rendering mode.
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

                // TODO. METER KEYBIND PARA HACER REMESH DEL CHUNK DONDE ESTÁ EL PLAYER ACTUALMENTE FOR DEBUGGING PURPOSES.
                // Coordinate rendering thread and the thread in charge of generating entity render data if necessary.
                if (entityManager::syncMutex().try_lock()) {

                    entityManager::swapReadWrite();
                    batchesToDraw_ = entityManager::renderingData();

                    entityManager::syncMutex().unlock();
                    entityManager::entityManagerCV().notify_one();

                }

                /*
                Opaque pass
                */

                // Terrain rendering.
                if (chunksToDraw_) {

                    // chunk.first refers to the chunk's center global postion.
                    // chunk.second refers to the chunk's vertex data.
                    for (auto const& chunk : *chunksToDraw_) {

                        if (playerCamera_->isInsideFrustum(chunk.second.globalChunkPos)) {

                            // NEXT.
                            // 1º. EN SETBLOCK DE PLAYER HAY QUE PONER QUE SE ACTUALIZEN LOS DATOS DE NEIGHBORS MINUS DEL LOD2

                            // LOD 1
                            if (chunkManager::chunkInLODDistance(chunk.first, 1, inLODborder, dirX, dirY, dirZ)) {

                                if (inLODborder) {
                                
                                    if (nVertices = chunk.second.vertices.size() + chunk.second.verticesLOD1_2Boundary.size()) {

                                        chunksVbo_->setDynamicData(chunk.second.vertices.data(), 0, chunk.second.vertices.size() * sizeof(vertex));
                                        chunksVbo_->setDynamicData(chunk.second.verticesLOD1_2Boundary.data(), chunk.second.vertices.size() * sizeof(vertex), chunk.second.verticesLOD1_2Boundary.size() * sizeof(vertex));
                                    
                                    }
                                
                                }
                                else {
                                
                                    if (nVertices = chunk.second.vertices.size() + chunk.second.verticesBoundary.size()) {

                                        chunksVbo_->setDynamicData(chunk.second.vertices.data(), 0, chunk.second.vertices.size() * sizeof(vertex));
                                        chunksVbo_->setDynamicData(chunk.second.verticesBoundary.data(), chunk.second.vertices.size() * sizeof(vertex), chunk.second.verticesBoundary.size() * sizeof(vertex));

                                    }
                                
                                }

                                renderer::draw3D(nVertices);

                            }
                            else if (chunkManager::chunkInLODDistance(chunk.first, 2, inLODborder, dirX, dirY, dirZ)) { // Probably will add LOD levels 3 and 4 in the future
                            
                                if(nVertices = chunk.second.verticesLOD2.size() + chunk.second.verticesLOD2Boundary.size()) {

                                    chunksVbo_->setDynamicData(chunk.second.verticesLOD2.data(), 0, chunk.second.verticesLOD2.size() * sizeof(vertex));
                                    chunksVbo_->setDynamicData(chunk.second.verticesLOD2Boundary.data(), chunk.second.verticesLOD2.size() * sizeof(vertex), chunk.second.verticesLOD2Boundary.size() * sizeof(vertex));

                                    renderer::draw3D(nVertices);

                                }
                            
                            }
                        
                        } 

                    }

                }

                // Times calculation.
                actualTime = glfwGetTime();
                timeStep_ = actualTime - lastFrameTime;
                lastFrameTime = actualTime;

                // ms/frame calculation and display.
                nFramesDrawn++;
                if (actualTime - lastSecondTime >= 1.0) {

                    //std::cout << "\r" << 1000.0 / nFramesDrawn << "ms/frame and total vertices is " << std::to_string(totalVertices);
                    logger::debugLog(std::to_string(1000.0 / nFramesDrawn) + "ms/frame");
                    nFramesDrawn = 0;
                    lastSecondTime = glfwGetTime();

                }

                // Entity rendering. // TODO. HAY QUE METER AQUÍ TAMBIÉN LO DE VERTICES TRANSLÚCIDOS PARA LAS ENTIDADES.
                entitiesVao_->bind();
                entitiesVbo_->bind();

                if (batchesToDraw_) {

                    for (auto const& batch : *batchesToDraw_) {

                        if (nVertices = batch.size()) {

                            entitiesVbo_->prepareStatic(batch.data(), sizeof(vertex) * nVertices);

                            renderer::draw3D(nVertices);

                        }

                    }

                }

                /*
                Transparent pass.
                */

                graphics::setDepthTest(true);
                graphics::setTranslucidPassConfig();
                translucidFB_->bind();
                translucidFB_->clearTextures({ vec4Zeroes, vec4Ones });
                translucidShader_->bind();
                translucidShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
                translucidShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
                translucidShader_->setUniformVec3f("u_viewPos", playerCamera_->globalPos());
                translucidShader_->setUniformVec3f("u_sunLightPos", lightpos);


                // Terrain rendering.
                vao_->bind();
                chunksVbo_->bind();
                if (chunksToDraw_) {

                    // chunk.first refers to the chunk's center global postion.
                    // chunk.second refers to the chunk's vertex data.
                    for (auto const& chunk : *chunksToDraw_) {

                        if (playerCamera_->isInsideFrustum(chunk.second.globalChunkPos)) {

                            // LOD 1.
                            if (chunkManager::chunkInLODDistance(chunk.first, 1, inLODborder, dirX, dirY, dirZ)) {

                                if (inLODborder) {

                                    if (nVertices = chunk.second.translucentVertices.size() + chunk.second.translucentVerticesLOD1_2Boundary.size()) {

                                        chunksVbo_->setDynamicData(chunk.second.translucentVertices.data(), 0, chunk.second.translucentVertices.size() * sizeof(vertex));
                                        chunksVbo_->setDynamicData(chunk.second.translucentVerticesLOD1_2Boundary.data(), chunk.second.translucentVertices.size() * sizeof(vertex), chunk.second.translucentVerticesLOD1_2Boundary.size() * sizeof(vertex));

                                    }

                                }
                                else {

                                    if (nVertices = chunk.second.translucentVertices.size() + chunk.second.translucentVerticesBoundary.size()) {

                                        chunksVbo_->setDynamicData(chunk.second.translucentVertices.data(), 0, chunk.second.translucentVertices.size() * sizeof(vertex));
                                        chunksVbo_->setDynamicData(chunk.second.translucentVerticesBoundary.data(), chunk.second.translucentVertices.size() * sizeof(vertex), chunk.second.translucentVerticesBoundary.size() * sizeof(vertex));

                                    }

                                }

                                renderer::draw3D(nVertices);

                            }
                            else if (chunkManager::chunkInLODDistance(chunk.first, 2, inLODborder, dirX, dirY, dirZ)) { // Probably will add LOD levels 3 and 4 in the future

                                if (nVertices = chunk.second.translucentVerticesLOD2.size() + chunk.second.translucentVerticesLOD2Boundary.size()) {

                                    chunksVbo_->setDynamicData(chunk.second.translucentVerticesLOD2.data(), 0, chunk.second.translucentVerticesLOD2.size() * sizeof(vertex));
                                    chunksVbo_->setDynamicData(chunk.second.translucentVerticesLOD2Boundary.data(), chunk.second.translucentVerticesLOD2.size() * sizeof(vertex), chunk.second.translucentVerticesLOD2Boundary.size() * sizeof(vertex));

                                    renderer::draw3D(nVertices);

                                }

                            }

                        }

                    }

                }


                /*
                Composite pass.
                */
                graphics::setCompositePassConfig();
                opaqueFB_->bind();

                compositeShader_->bind();

                translucidFB_->getTexture(textureType::COLOR, 0)->bind(0);
                translucidFB_->getTexture(textureType::COLOR, 1)->bind(1);

                screenVao_->bind();
                screenVbo_->bind();

                screenVbo_->prepareStatic(screenShaderQuad, 6 * sizeof(float) * 4);
                renderer::draw2D(6);


                /*
                GUI rendering
                */
                graphics::setDepthTest(false);
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
                graphics::blending(true);

                opaqueFB_->bind();

                opaqueShader_->bind();
                opaqueShader_->setUniform1i("u_renderMode", 1);
                blockTextureAtlas_->bind();

                GUImanager::drawGUI();

                /*
                Screen pass.
                */
                graphics::setScreenPassConfig();
                opaqueFB_->unbind();

                // Clear the window (default framebuffer) to draw the next frame.
                renderer::clearWindow(); // TODO. IS THIS UNNCECESARRY?

                screenShader_->bind();
                //screenShader_->setUniform1i("screenTexture", 0);

                opaqueFB_->getTexture(textureType::COLOR, 0)->bind();

                screenVbo_->prepareStatic(screenShaderQuad, 6*sizeof(float)*4);
                renderer::draw2D(6);


                // Swap front and back buffers.
                glfwSwapBuffers(mainWindow_->windowAPIpointer());

                // Poll for and process events.
                glfwPollEvents();

                // Handle user inputs.
                input::handleInputs();

                // Reset things for the next iteration.
                graphics::setDepthTest(true);

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

        opaqueShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
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

        // These pointers where not the original owners of the object they point to.
        // They were allocated in the 'graphics' class and they have been properly deallocated before this.
        opaqueShader_ = nullptr;
        translucidShader_ = nullptr;
        compositeShader_ = nullptr;
        screenShader_ = nullptr;
        chunksVbo_ = nullptr;
        entitiesVbo_ = nullptr;
        screenVbo_ = nullptr;
        vao_ = nullptr;
        screenVao_ = nullptr;
        playerCamera_ = nullptr;

        if (translucidFB_) {

            delete translucidFB_;
            translucidFB_ = nullptr;

        }

        if (screenFB_) {
        
            delete screenFB_;
            screenFB_ = nullptr;
        
        }

        if (mainWindow_) {

            delete mainWindow_;
            mainWindow_ = nullptr;

        }

        chunksToDraw_ = nullptr; // TODO. IGUAL ESTO DA ERROR??
        batchesToDraw_ = nullptr;

        graphicalModeInitialised_ = false;

        
    }

}