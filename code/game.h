/**
* @file game.h
* @version 1.0
* @date 04/10/2022
* @author Abdon Crespo Alvarez
* @title Game Engine API.
* @brief Game Engine API that contains the most basic and important members for the game engine.
*/

#ifndef _VOXELENG_GAME_
#define _VOXELENG_GAME_

#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>
#include "chunk.h"
#include "definitions.h"
#include "gameWindow.h"
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "vertexArray.h"
#include <Utilities/Logger/logger.h>
#include "world.h"
#include "vec.h"
#include <Graphics/framebuffer.h>
#include <Graphics/Vertex/VertexBufferLayout/vertexBufferLayout.h>
#include <Graphics/SSBO/SSBO.h>
#include <Graphics/Lighting/Lights/LightInstance/lightInstance.h>

namespace VoxelEng {

    //////////////////////////
    // Forward declarations.//
    //////////////////////////

    class chunkManager;


    ////////////////////
    //Classes & enums.//
    ////////////////////

    /**
    * @brief The execution modes of the engine.
    */
    enum class engineMode {EXIT, AIMENULOOP, GRAPHICALMENU, INITLEVEL, EDITLEVEL, EXITLEVEL, INITRECORD, PLAYINGRECORD, EXITRECORD};

    /**
    * @brief Game engine API responsible for all the basic engines operations (startup, menu/level/AI mode loops, access to save slots...).
    * This class also serves as a way to abstract classes that directly implement the engine.
    * For example, instead of storing a reference to the player camera for the engines' graphical mode and polluting the code,
    * said camera can be accessed by calling game::playerCamera().
    */
	class game {

	public:

        /*
        Attributes.
        */

        /**
        * @brief Auxiliary threads' flags that tell if they can be executed or not or if
        * they should stop their execution or not.
        */
        static std::atomic<bool> threadsExecute[3];


        /*
        Methods.
        */

        // Initialisers.

        /**
        * @brief Initialise the game engine and allocate any resources that are needed on initialisation.
        */
        static void init();

        /**
        * @brief Initialise the game engine's graphical part and allocate any resources that are needed on initialisation.
        */
        static void initGraphicalMode();


        // Observers.

        /**
        * @brief Get the currently selected save slot.
        */
        static unsigned int selectedSaveSlot();

        /**
        * @brief Get the actual time step.
        */
        static double timeStep();

        /**
        * @brief Get the currently selected engine mode.
        */
        static engineMode selectedEngineMode();

        /**
        * @brief Returns whether the engine is in AI mode (training, testing AIs, generating a record of an AI game match...
        * without the need for the graphical capabilities of the engine to save resources) or not.
        */
        static bool AImodeON();

		// Modifiers.

        /**
        * @brief The engine's main loop.
        */
        static void mainLoop();

        /**
        * @brief The engine's AI menu loop.
        */
        static void aiMenuLoop();

        /**
        * @brief The engine's graphical main menu loop.
        */
		static void mainMenuLoop();

        /**
        * @brief Graphical mode main loop when in a level.
        * @param terrainFile The file from where to load the terrain for the level. If terrainFile is equal to "", then either a new level will be generated or
        * a level from a slot will be loaded (depending on the selected save slot).
        */
		static void gameLoop(const std::string& terrainFile = "");

        /**
        * @brief Set the engine's current execution mode.
        * If the specified mode is equal to the one currently selected, 
        * then this function does nothing.
        */
        static void setLoopSelection(engineMode mode);

        /**
        * @brief Set the currently selected level save slot.
        * If slot = 0, then a new level will be generated (unless a terrain file is specificated
        * in the corresponding code that generates the level's terrain).
        * Otherwise, the level will contain the save data of the specified save slot.
        * If the specified slot doesn't have any data or doesn't exists, a new level will be generated.
        */
        static void setSaveSlot(unsigned int saveSlot);

        /**
        * @brief Returns the user's camera.
        */
        static camera& playerCamera();

        /**
        * @brief Returns the graphics API context window.
        * WARNING. Locks the mutex guarding said window. Check if locked
        * with game::isWindowMutexLocked() and unlock when finished
        * using it with game::unlockWindowMutex().
        */
        static window& getWindow();

        /**
        * @brief Switch between using the shader lighting test or not (disabled by default).
        */
        static void switchComplexLighting();

        /**
        * @brief Set the engine's AI mode.
        */
        static void setAImode(bool ON);

        /**
        * @brief Safely stop all auxiliary threads used for world generation, rendering, tick/player input processing, etc...
        */
        static void stopAuxiliaryThreads();


        // Clean up.

        /**
        * @brief Cleaning up special data structures allocated only for use
        * when the user is in a level and blocks the caller thread
        * until the function that executes the level game loop
        * has finished it's execution.
        */
        static void resetLevel();

        /**
        * @brief To be called when closing the engine.
        * Cleans up the heap memory allocated and deinitialises it.
        */
        static void reset();
        
        /**
        * @brief Cleans up the heap memory allocated for the engine's graphical part and deinitialises it.
        */
        static void resetGraphicalMode();

	private:

        static bool initialised_,
                    graphicalModeInitialised_,
                    AImodeON_,
                    useComplexLighting_;
        
		static window* mainWindow_;

        static std::thread* chunkManagementThread_,
                          * priorityChunkUpdatesThread_,
                          * playerInputThread_,
                          * tickManagementThread_;

        static std::atomic<engineMode> loopSelection_;
		static std::atomic<double> timeStep_; // How much time has passed since the last frame was drawn. Use this to move entities without caring about FPS.

        static world* world_; // A world defines things as the sky background or other level-related properties.
        static skybox defaultSkybox_;

        static unsigned int saveSlot_,
                            blockReachRange_,
                            nMeshingThreads_; // Number of threads for non-high priority mesh updates with infinite world loading
        static float FOV_,
                     zNear_,
                     zFar_;

        static camera* playerCamera_;

        static texture* blockTextureAtlas_;
        static std::unordered_map<vec3, chunkRenderingData> const* chunksToDraw_;
        static const std::vector<model>* batchesToDraw_;

        static shader* opaqueShader_;
        static shader* translucidShader_;
        static shader* compositeShader_;
        static shader* screenShader_;

        static vertexBuffer* chunksVbo_;
        static vertexBuffer* entitiesVbo_;
        static vertexBuffer* screenVbo_;
        static vertexArray* vao_;
        static vertexArray* entitiesVao_; // TODO. QUE SE USE SOLO UN VAO ENTRE ENTIDADES Y TERRENO. 
        static vertexArray* screenVao_;

        static framebuffer* opaqueFB_;
        static framebuffer* translucidFB_;
        static framebuffer* screenFB_;

        static SSBO<lightInstance>* directionalLightsInstances_;
        static SSBO<lightInstance>* pointLightsInstances_;
        static SSBO<lightInstance>* spotLightsInstances_;

        #if GRAPHICS_API == OPENGL

            static glm::mat4 MVPmatrix_;

        #endif

	};

    inline unsigned int game::selectedSaveSlot() {
    
        return saveSlot_;
    
    }

    inline double game::timeStep() {
    
        return timeStep_;
    
    }

    inline engineMode game::selectedEngineMode() {
    
        return loopSelection_;
    
    }

    inline bool game::AImodeON() {

        return AImodeON_;

    }

    inline void game::setSaveSlot(unsigned int slot) {
    
        saveSlot_ = slot;
    
    }

    inline camera& game::playerCamera() {
    
        return *playerCamera_;
    
    }

    inline window& game::getWindow() {

        return *mainWindow_;

    }

}

#endif