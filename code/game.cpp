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
#include <Entities/plane.h>
#include <Registry/registries.h> // This header also includes the classes that derive from 'registeredElement'.
#include <Registry/registry.h>
#include <Graphics/graphics.h>
#include <Graphics/Frustum/frustum.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/Materials/materials.h>
#include <Graphics/Lighting/Lights/light.h>
#include <Graphics/Vertex/vertex.h>
#include <Graphics/Vertex/ChunkVertexBuffer/chunkVertexBuffer.h>
#include <Time/Timer/timer.h>
#include <Utilities/Var/var.h>
#include <World/WorldGen/worldGen.h>

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

    std::atomic<bool> game::threadsExecute[3] = { false, false, false };

    bool game::initialised_ = false,
         game::graphicalModeInitialised_ = false,
         game::useComplexLighting_ = false;

    window* game::mainWindow_ = nullptr;

    std::thread* game::chunkManagementThread_ = nullptr,
               * game::priorityChunkUpdatesThread_ = nullptr,
               * game::playerInputThread_ = nullptr,
               * game::tickManagementThread_ = nullptr;

    std::unique_ptr<settings> game::settings_;
    std::atomic<engineMode> game::loopSelection_ = engineMode::MENULOOP;
    std::atomic<double> game::timeStep_ = 0.0f;

    skybox game::defaultSkybox_{ 140, 170, 255, 1.0f };

    unsigned int game::saveSlot_ = 0,
        game::blockReachRange_ = 5,
        game::nMeshingThreads_ = 0;
    float game::FOV_ = 110.0f,
          game::zNear_ = 0.1f,
          game::zFar_ = 500.0f;

    camera* game::playerCamera_ = nullptr;
    texture* game::blockTextureAtlas_ = nullptr;

    std::unordered_map<vec3, chunkRenderingData> const * game::chunksRenderingData_ = nullptr;
    std::unordered_map<vec3, chunkVBOoperation> const * game::chunksVBOoperations_ = nullptr;

    const std::vector<model>* game::batchesToDraw_ = nullptr;

    shader* game::shadowShader_ = nullptr;
    shader* game::translucentShadowShader_ = nullptr;
    shader* game::opaqueShader_ = nullptr;
    shader* game::translucidShader_ = nullptr;
    shader* game::compositeShader_ = nullptr;
    shader* game::screenShader_ = nullptr;
    chunkVertexBuffer* game::chunksVbo_ = nullptr;
    vertexBuffer* game::entitiesVbo_ = nullptr;
    vertexBuffer* game::screenVbo_ = nullptr;
    vertexArray* game::vao_ = nullptr;
    vertexArray* game::entitiesVao_ = nullptr;
    vertexArray* game::screenVao_ = nullptr;
    
    framebuffer* game::shadowFB_ = nullptr;
    framebuffer* game::translucentShadowFB_ = nullptr;
    framebuffer* game::opaqueFB_ = nullptr;
    framebuffer* game::translucidFB_ = nullptr;
    framebuffer* game::screenFB_ = nullptr;

    SSBO<lightInstance>* game::directionalLightsInstances_ = nullptr;
    SSBO<lightInstance>* game::pointLightsInstances_ = nullptr;
    SSBO<lightInstance>* game::spotLightsInstances_ = nullptr;

    std::unordered_set<vec3> game::opaqueChunkGeometryToDraw;
    std::unordered_set<vec3> game::translucentChunkGeometryToDraw;
    float game::screenShaderQuad[24] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    double game::lastSecondTime = 0.0;
    double game::lastFrameTime = 0.0;
    double game::actualTime = 0.0;
    int game::nFramesDrawn = 0;
    unsigned int game::nVertices = 0;
    unsigned int game::nTranslucentVertices = 0;

    
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

            // Settings.
            settings_ = std::make_unique<settings>("settings/settings.json");

            // General variables.
            loopSelection_ = engineMode::MENULOOP;
            timeStep_ = 0.0f;

            // Registrable elements initialisation.
            registryElement::init("registryElement");
            material::init("material");
            light::init("light");
            var::init("var");

            // Registries collection initialisation.
            registries::init();

            // Register materials.
            registryInsOrdered<std::string, material>* materialsRegistry = registries::getInsOrdered("Materials")->pointer<registryInsOrdered<std::string, material>>();
            materialsRegistry->insert("OmegaRed",
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                32.0f);

            materialsRegistry->insert("AlphaBlue",
                0.0f, 0.0f, 10.0f,
                0.0f, 0.0f, 10.0f,
                0.0f, 0.0f, 10.0f,
                32.0f);

            materialsRegistry->insert("DeltaGreen",
                0.0f, 10.0f, 0.0f,
                0.0f, 10.0f, 0.0f,
                0.0f, 10.0f, 0.0f,
                32.0f);

            materialsRegistry->insert("RedOnlyIfLit",
                1.0f, 1.0f, 1.0f,
                10.0f, 0.0f, 0.0f,
                10.0f, 0.0f, 0.0f,
                32.0f);

            materialsRegistry->insert("UltraShiny",
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                256.0f);

            // Light types registration.
            registryInsOrdered<std::string, directionalLight>* directionalLightsRegistry = registries::getInsOrdered("DirectionalLights")->pointer<registryInsOrdered<std::string, directionalLight>>();
            directionalLightsRegistry->insert("BlueDirectionalLight",
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f, 
                0.0f, 0.0f, 1.0f);

            registryInsOrdered<std::string, pointLight>* pointLightsRegistry = registries::getInsOrdered("PointLights")->pointer<registryInsOrdered<std::string, pointLight>>();
            pointLightsRegistry->insert("RedPointLight",
                8.0f, 0.0f, 0.0f,
                8.0f, 0.0f, 0.0f,
                8.0f, 0.0f, 0.0f,
                8.0f);
            pointLightsRegistry->insert("BluePointLight",
                0.0f, 0.0f, 8.0f,
                0.0f, 0.0f, 8.0f,
                0.0f, 0.0f, 8.0f,
                8.0f);
            pointLightsRegistry->insert("NegativeRedPointLight",
                -8.0f, 0.0f, 0.0f,
                -8.0f, 0.0f, 0.0f,
                -8.0f, 0.0f, 0.0f,
                8.0f);
            pointLightsRegistry->insert("NegativePointLight",
                -8.0f, -8.0f, -8.0f,
                -8.0f, -8.0f, -8.0f,
                -8.0f, -8.0f, -8.0f,
                8.0f);

            registryInsOrdered<std::string, spotLight>* spotLightsRegistry = registries::getInsOrdered("SpotLights")->pointer<registryInsOrdered<std::string, spotLight>>();
            spotLightsRegistry->insert("GreenSpotLight",
                0.0f, 5.0f, 0.0f,
                0.0f, 10.0f, 0.0f, 
                0.0f, 10.0f, 0.0f,
                25.0f, 35.0f, 16.0f);

            // Block registration.
            block::init();
            // TODO. CONVERT THIS INTO A REGISTRY OF BLOCKS.
            block::registerBlock("starminer::grass", blockOpacity::OPAQUEBLOCK, { {"all", 1} }, "UltraShiny"); // TODO. Manual texture ID assignment is temporary.
            block::registerBlock("starminer::stone", blockOpacity::OPAQUEBLOCK, { {"all", 2} });
            block::registerBlock("starminer::sand", blockOpacity::OPAQUEBLOCK, { {"all", 3} });
            block::registerBlock("starminer::marbleBlock", blockOpacity::OPAQUEBLOCK, { {"all", 4}}, "AlphaBlue", "PointLight:BluePointLight");
            block::registerBlock("starminer::dirt", blockOpacity::OPAQUEBLOCK, { {"all", 6} });
            block::registerBlock("starminer::coalOre", blockOpacity::OPAQUEBLOCK, { {"all", 7} });
            block::registerBlock("starminer::ironOre", blockOpacity::OPAQUEBLOCK, { {"all", 8} });
            block::registerBlock("starminer::goldOre", blockOpacity::OPAQUEBLOCK, { {"all", 9} });
            block::registerBlock("starminer::diamondOre", blockOpacity::OPAQUEBLOCK, { {"all", 10}});
            block::registerBlock("starminer::log", blockOpacity::OPAQUEBLOCK, { {"all", 11}, {"faceY+", 12}, {"faceY-", 12}});
            block::registerBlock("starminer::glass", blockOpacity::FULLTRANSPARENT, { {"all", 13} });
            block::registerBlock("starminer::glassRed", blockOpacity::TRANSLUCENTBLOCK, { {"all", 14} });
            block::registerBlock("starminer::glassBlue", blockOpacity::TRANSLUCENTBLOCK, { {"all", 15} });
            block::registerBlock("starminer::marbleBlock2", blockOpacity::OPAQUEBLOCK, { {"all", 16} }, "OmegaRed", "PointLight:RedPointLight");
            block::registerBlock("starminer::water", blockOpacity::TRANSLUCENTBLOCK, { {"all", 17 } });

            // Worldgen initialisation.
            worldGen::init();

            initialised_ = true;

        }

    }
    
    void game::initGraphicalMode() {
    
        if (graphicalModeInitialised_)
            logger::errorLog("Engine's graphical mode already initialised");
        else {

            mainWindow_ = new window(settings_->windowWidth(), settings_->windowHeight(), "VoxelEng");

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

            registry<std::string, var>* SSBORegistry = registries::get("SSBOs")->pointer<registry<std::string, var>>();
            directionalLightsInstances_ = SSBORegistry->get("DirectionalLightsInstances")->pointer<SSBO<lightInstance>>();
            pointLightsInstances_ = SSBORegistry->get("PointLightsInstances")->pointer<SSBO<lightInstance>>();
            spotLightsInstances_ = SSBORegistry->get("SpotLightsInstances")->pointer<SSBO<lightInstance>>();

            chunksVbo_ = static_cast<chunkVertexBuffer*>(graphics::pVbo("chunks"));
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
            shadowFB_ = new framebuffer(4096, 4096, { textureType::DEPTH });
            translucentShadowFB_ = new framebuffer(4096, 4096, { textureType::COLOR, textureType::DEPTH});
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
            shadowShader_ = &graphics::shadowShader();
            translucentShadowShader_ = &graphics::translucentShadowShader();
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

            case engineMode::MENULOOP:  // AI game menu.

                MenuLoop();

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

    void game::MenuLoop() {
    
        if (loopSelection_ == engineMode::MENULOOP) {
        
            unsigned int chosenOption = 0;

            logger::say("Welcome to Deep Dive Engine! Please select one of the following options.");
            logger::say("1). Enter level editor mode");
            logger::say("2). Exit");

            do {

                while (!validatedCinInput<unsigned int>(chosenOption))
                    logger::say("Invalid option. Please try again");

                switch (chosenOption) 
                {
                case 1:
                    setLoopSelection(engineMode::GRAPHICALMENU);
                    break;
                case 2:
                    setLoopSelection(engineMode::EXIT);
                    break;
                default:
                    logger::say("Invalid option. Please try again");
                    break;
                }

            } while (loopSelection_ == engineMode::MENULOOP);

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

    void game::setupGameLoop() {
    
        // Variables used only on this loop.
        opaqueChunkGeometryToDraw.clear();
        translucentChunkGeometryToDraw.clear();
        lastSecondTime = glfwGetTime(); // How much time has passed since the last second passed.
        lastFrameTime = lastSecondTime;
        actualTime = 0;
        nFramesDrawn = 0;
        nVertices = 0;
        nTranslucentVertices = 0;

        if (!graphicalModeInitialised_)
            initGraphicalMode();

        chunkManager::setNChunksToCompute(game::getSettings().chunkXZRenderDistance());

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
            input::setControlAction(controlCode::q, inputFunctions::getCurrentChunk, false);

        }

        // Things to apply when the terrain is loaded.
        chunkManager::waitInitialTerrainLoaded();

        setLoopSelection(engineMode::EDITLEVEL);

        // Start threads that require the world to be loaded first.
        playerInputThread_ = new std::thread(&player::processSelectionRaycast);
        tickManagementThread_ = new std::thread(&world::processWorldTicks);

        /*
        Rendering loop.
        */

        // Configure game window's settings.
        mainWindow_->changeStateMouseLock(false);

        // Set shader options.
        opaqueShader_->setUniform1i("u_useComplexLighting", 0);

        // Spawn test entities here.

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //TODO. ADD THIS AS AN OPTION.
    
    }

    void game::preRenderingSetup() {
   
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

        if (mainWindow_->wasResized())
            mainWindow_->resizeHeavyProcessing();
        if (!mainWindow_->isMouseFree())
            player::updateTransform(game::timeStep());

        playerCamera_->updateView();
        MVPmatrix_ = playerCamera_->projectionMatrix() * playerCamera_->viewMatrix();

        blockTextureAtlas_->bind(0);

        // Set the sun's directional light MVP matrix.
        SSBO<lightInstance>* directionalLightsInstances = registries::get("SSBOs")->pointer<registry<std::string, var>>()->get("DirectionalLightsInstances")->pointer<SSBO<lightInstance>>();
        lightInstance& instance = directionalLightsInstances->get(0);
        instance.pos = vec4(CHUNK_SIZE * 20 * -1, 200.0f, .0f, 0.0f);
        instance.dir = vec4(0.7f, -0.7f, 0.0f, 0.0f);
        glm::mat4 proj = glm::ortho(-320.0f, 320.0f, -384.0f, 384.0f, zNear_, zFar_);
        glm::mat4 view = glm::lookAt(instance.pos, vec3Zero, vec3FixedUp);
        instance.MVP = proj * view;
        directionalLightsInstances->reuploadElement(0);
    
    }

    void game::syncWithMeshingThreads() {
    
        // Receive updated chunk meshes when possible.
        if (chunkManager::priorityManagerThreadMutex().try_lock()) {

            chunkManager::swapChunkMeshesBuffers();
            chunksRenderingData_ = chunkManager::drawableChunksRead();
            chunksVBOoperations_ = chunkManager::chunkVBOoperationsRead();

            chunkManager::priorityManagerThreadMutex().unlock();
            chunkManager::priorityManagerThreadCV().notify_one();

        }
        else if (chunkManager::managerThreadMutex().try_lock()) {

            chunkManager::swapChunkMeshesBuffers();
            chunksRenderingData_ = chunkManager::drawableChunksRead();
            chunksVBOoperations_ = chunkManager::chunkVBOoperationsRead();

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
    
    }

    void game::shadowPass() {
    
        graphics::setDepthTest(true);
        graphics::setOpaquePassConfig();
        glViewport(0, 0, shadowFB_->width(), shadowFB_->height());

        // Opaque shadowmap.
        shadowFB_->bind();
        shadowShader_->bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        if (chunksRenderingData_) {

            for (vec3 const& chunkPos : opaqueChunkGeometryToDraw) {

                // Draw terrain.
                const chunkVertexBufferZone& bufferZone = chunksVbo_->bufferZone(chunkPos, false);
                renderer::draw3D(bufferZone.startPos / sizeof(vertex), bufferZone.size / sizeof(vertex));

            }

        }
        shadowFB_->unbind();

        // Translucent shadowmap.
        translucentShadowFB_->bind();
        translucentShadowShader_->bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        if (chunksRenderingData_) {

            for (vec3 const& chunkPos : translucentChunkGeometryToDraw) {

                // Draw terrain.
                const chunkVertexBufferZone& bufferZone = chunksVbo_->bufferZone(chunkPos, true);
                renderer::draw3D(bufferZone.startPos / sizeof(vertex), bufferZone.size / sizeof(vertex));

            }

        }
        translucentShadowFB_->unbind();

        glViewport(0, 0, mainWindow_->width(), mainWindow_->height());
    
    }

    void game::opaquePass() {
    
        // Terrain rendering.
        graphics::setDepthTest(true);
        graphics::setOpaquePassConfig();
        opaqueFB_->bind();
        opaqueFB_->clearAllTextures();
        opaqueShader_->bind();
        opaqueShader_->setUniformVec3f("u_viewPos", playerCamera_->globalPos());
        opaqueShader_->setUniform1i("u_renderMode", 0); // TODO. ENUM FOR RENDER MODE.
        opaqueShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
        opaqueShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
        shadowFB_->getTexture(textureType::DEPTH, 0)->bind(1);
        translucentShadowFB_->getTexture(textureType::COLOR, 0)->bind(2);
        translucentShadowFB_->getTexture(textureType::DEPTH, 0)->bind(3);
        if (chunksRenderingData_) {

            for (vec3 const& chunkPos : opaqueChunkGeometryToDraw) {

                const chunkRenderingData& chunk = chunksRenderingData_->at(chunkPos);

                if (playerCamera_->isInsideFrustum(chunk.globalChunkPos)) {

                    int nPointLightsChunk = chunk.pointLights.size();

                    // Upload dynamic lights.
                    if (nPointLightsChunk)
                        pointLightsInstances_->setContentsAndReupload(chunk.pointLights);
                    if (!chunk.spotLights.empty())
                        spotLightsInstances_->setContentsAndReupload(chunk.spotLights);

                    // Draw terrain.
                    const chunkVertexBufferZone& bufferZone = chunksVbo_->bufferZone(chunkPos, false);
                    renderer::draw3D(bufferZone.startPos / sizeof(vertex), bufferZone.size / sizeof(vertex));

                }

            }

        }

        // Entity rendering. // TODO. HAY QUE METER TAMBIÉN LO DE VERTICES TRANSLÚCIDOS PARA LAS ENTIDADES.
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
    
    }

    void game::translucentPass() {

        graphics::setDepthTest(true);
        graphics::setTranslucidPassConfig();
        translucidFB_->bind();
        translucidFB_->clearTextures({ vec4Zeroes, vec4Ones });
        translucidShader_->bind();
        translucidShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
        translucidShader_->setUniformMatrix4f("u_MVP", MVPmatrix_);
        translucidShader_->setUniformVec3f("u_viewPos", playerCamera_->globalPos());

        // Terrain rendering.
        vao_->bind(); // Restore chunks vao to bound status since opaque pass ended with entities vao bound.
        chunksVbo_->bind();
        shadowFB_->getTexture(textureType::DEPTH, 0)->bind(1);
        translucentShadowFB_->getTexture(textureType::COLOR, 0)->bind(2);
        translucentShadowFB_->getTexture(textureType::DEPTH, 0)->bind(3);
        if (chunksRenderingData_) {

            for (vec3 const& chunkPos : translucentChunkGeometryToDraw) {

                const chunkRenderingData& chunk = chunksRenderingData_->at(chunkPos);

                if (playerCamera_->isInsideFrustum(chunk.globalChunkPos)) {

                    int nPointLightsChunk = chunk.pointLights.size();

                    // Upload dynamic lights.
                    if (nPointLightsChunk)
                        pointLightsInstances_->setContentsAndReupload(chunk.pointLights);
                    if (!chunk.spotLights.empty())
                        spotLightsInstances_->setContentsAndReupload(chunk.spotLights);

                    // Draw terrain.
                    const chunkVertexBufferZone& bufferZone = chunksVbo_->bufferZone(chunkPos, true);
                    renderer::draw3D(bufferZone.startPos / sizeof(vertex), bufferZone.size / sizeof(vertex));

                }

            }

        }

    }

    void game::compositePass() {

        graphics::setCompositePassConfig();
        opaqueFB_->bind();

        compositeShader_->bind();

        translucidFB_->getTexture(textureType::COLOR, 0)->bind(0);
        translucidFB_->getTexture(textureType::COLOR, 1)->bind(1);

        screenVao_->bind();
        screenVbo_->bind();

        screenVbo_->prepareStatic(screenShaderQuad, 6 * sizeof(float) * 4);
        renderer::draw2D(6);

    }

    void game::GUIpass() {
    
        graphics::setDepthTest(false);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        graphics::blending(true);

        opaqueFB_->bind();

        opaqueShader_->bind();
        opaqueShader_->setUniform1i("u_renderMode", 1);
        blockTextureAtlas_->bind();

        GUImanager::drawGUI();
    
    }

    void game::screenPass() {

        graphics::setScreenPassConfig();
        opaqueFB_->unbind();

        // Clear the window (default framebuffer) to draw the next frame.
        renderer::clearWindow();

        screenShader_->bind();

        opaqueFB_->getTexture(textureType::COLOR, 0)->bind(0);

        screenVbo_->prepareStatic(screenShaderQuad, 6 * sizeof(float) * 4);
        renderer::draw2D(6);

    }

    void game::gameLoop() {
        
        if (loopSelection_ == engineMode::INITLEVEL) {

            setupGameLoop();

            while (loopSelection_ == engineMode::EDITLEVEL) {

                preRenderingSetup();

                syncWithMeshingThreads();

                /*
                3D rendering.
                */
                vao_->bind();
                chunksVbo_->bind();

                // Upload changes in chunk vertex data to chunk VBO.
                if (chunksVBOoperations_) {

                    for (auto it = chunksVBOoperations_->cbegin(); it != chunksVBOoperations_->cend(); it++) {

                        //bool b = chunksRenderingData_->contains(it->first); // DEBUG.

                        if (it->second == chunkVBOoperation::PUSH) {

                            const chunkRenderingData& chunkRenderData = chunksRenderingData_->at(it->first);
                            if (chunkRenderData.vertices.size()) {

                                chunksVbo_->pushDynamicData(it->first,
                                    chunkRenderData.vertices.data(), chunkRenderData.vertices.size() * sizeof(vertex),
                                    false);

                                opaqueChunkGeometryToDraw.insert(it->first);

                            }

                            if (chunkRenderData.translucentVertices.size()) {

                                chunksVbo_->pushDynamicData(it->first,
                                    chunkRenderData.translucentVertices.data(), chunkRenderData.translucentVertices.size() * sizeof(vertex),
                                    true);

                                translucentChunkGeometryToDraw.insert(it->first);

                            }

                        }
                        else if (it->second == chunkVBOoperation::FREE) {

                            chunksVbo_->freeDynamicData(it->first, false);
                            chunksVbo_->freeDynamicData(it->first, true);
                            opaqueChunkGeometryToDraw.erase(it->first);
                            translucentChunkGeometryToDraw.erase(it->first);

                        }

                    }

                }

                shadowPass();

                opaquePass();

                translucentPass();

                compositePass();

                GUIpass();

                screenPass();

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

            case engineMode::MENULOOP:

                switch (mode) {
            
                    case engineMode::EXIT:

                        input::inputMutex().lock(); // Do not accept input until all is cleared.
                        reset();

                        loopSelection_ = engineMode::EXIT;
                        input::inputMutex().unlock();

                        break;

                    case engineMode::MENULOOP:
                        break;

                    case engineMode::GRAPHICALMENU:

                        initGraphicalMode();

                        loopSelection_ = engineMode::GRAPHICALMENU;

                        break;

                    default:
                        logger::errorLog("Unsupported engine mode transition");
            
                }

                break;

            case engineMode::GRAPHICALMENU:

                switch (mode) {

                    case engineMode::EXIT:

                        setLoopSelection(engineMode::MENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::MENULOOP:

                        resetGraphicalMode();

                        if (!chunkManager::openedTerrainFileName().empty())
                            chunkManager::openedTerrainFileName("");

                        loopSelection_ = VoxelEng::engineMode::MENULOOP;

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
                        setLoopSelection(engineMode::MENULOOP);
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
                        setLoopSelection(engineMode::MENULOOP);
                        setLoopSelection(engineMode::EXIT);

                        break;

                    case engineMode::EDITLEVEL:
                        break;

                    case engineMode::EXITLEVEL:

                        stopAuxiliaryThreads();
                        chunkManager::clear();
                        resetLevel();
                        world::unloadWorld();
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

            default:
                logger::errorLog("Unspecified current engine mode selected");
        
        }
    
    }

    void game::switchComplexLighting() {

        opaqueShader_->setUniform1i("u_useComplexLighting", useComplexLighting_ ? 1 : 0);
        useComplexLighting_ = !useComplexLighting_;

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

        if (registries::initialised())
            registries::reset();

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

        if(world::initialised())
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

        chunksRenderingData_ = nullptr;
        chunksVBOoperations_ = nullptr;

        batchesToDraw_ = nullptr;

        graphicalModeInitialised_ = false;

    }

}