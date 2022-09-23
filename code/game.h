#ifndef _VOXELENG_GAME_
#define _VOXELENG_GAME_
#include <atomic>
#include "gameWindow.h"
#include "world.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_array.h"
#include "vertex_buffer_layout.h"
#include "renderer.h"
#include "definitions.h"
#include "logger.h"
#include "chunk.h"


namespace VoxelEng {

    //////////////////////////
    // Forward declarations.//
    //////////////////////////

    class chunkManager;


     ////////////
    //Defines.//
    ////////////

    #define EXIT 0
    #define AIMENULOOP 1
    #define GRAPHICALMENU 2
    #define GRAPHICALLEVEL 3


    ////////////////////
    //Classes & enums.//
    ////////////////////

    // This enum class will enable us to add more functionality that affects the save slots
    // change slot name (if slot names are added), delete slot, 
    // create new slot (if support for an 'infinite' number of save slots is supported)...
    enum class slotAccessType {

        save,
        load

    };


    /*
    This class serves as an API to abstract classes that directly implement the engine.
    For example, instead of going to the camera class and directly modifying its attributes to update it
    according to the player input, a transparent updatePlayerCamera() method is provided.
    */
	class game {

	public:

        // Initialisers.

        static void init();

        static void initGraphicalMode();


        // Observers.

        static slotAccessType getSlotAccessType();

        static unsigned int selectedSaveSlot();

        static bool mainWindowClosed();

        static double timeStep();

        static unsigned int loopSelection();


		// Modifiers.

        static void mainLoop();

        static void aiMenuLoop();

		static void mainMenuLoop();

        /*
        If 'terrainFile' is equal to "", then either a new level will be generated or
        a level from a slot will be loaded (depending on the selected save slot).
        */
		static void gameLoop(bool playingAIRecord, const std::string& terrainFile = "");

        static void goToGraphicalMenu();

        static void goToAIMenu();

        /*
        If slot = 0, then a new level will be generated (unless a terrain file is specificated
        in the corresponding code that generates the level's terrain).
        Otherwise, the level will contain the save data of the specified save slot.
        If the specified slot doesn't have any data or doesn't exists, a new level will be generated.
        */
        static void setSaveSlot(unsigned int saveSlot);

        static void setSlotAccessType(slotAccessType type);

        static camera& playerCamera();

        static window& getWindow();

        static void enterLevel();

        static void switchComplexLighting();


        // Clean up.

        /*
        To be called when closing the engine.
        Cleans up the heap memory allocated by this system.
        */
        static void cleanUp();

        static void cleanUpGraphicalMode();

	private:

        static bool initialised_,
                    graphicalModeInitialised_;
        
		static window* mainWindow_;

        static std::atomic<unsigned int> loopSelection_;
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

        static bool useComplexLighting_;
        
        static texture* blockTextureAtlas_;
        static std::unordered_map<vec3, std::vector<vertex>> const* chunksToDraw_;
        static const std::vector<const model*>* batchesToDraw_;
        static shader* defaultShader_;
        static vertexBuffer* vbo_;
        static vertexArray* va_;
        static vertexBufferLayout* layout_;
        static renderer* renderer_;

        #if GRAPHICS_API == OPENGL

            static glm::mat4 MVPmatrix_;

        #else



        #endif
        

        static slotAccessType slotAccessType_;

	};

    inline slotAccessType game::getSlotAccessType() {
    
        return slotAccessType_;
    
    }

    inline unsigned int game::selectedSaveSlot() {
    
        return saveSlot_;
    
    }

    inline bool game::mainWindowClosed() {
    
        return mainWindow_->isClosing();
    
    }

    inline double game::timeStep() {
    
        return timeStep_;
    
    }

    inline unsigned int game::loopSelection() {
    
        return loopSelection_;
    
    }

    inline void game::setSaveSlot(unsigned int slot) {
    
        saveSlot_ = slot;
    
    }

    inline void game::setSlotAccessType(slotAccessType type) {
    
        slotAccessType_ = type;
    
    }

    inline camera& game::playerCamera() {
    
        return *playerCamera_;
    
    }

    inline window& game::getWindow() {
    
        return *mainWindow_;
    
    }

}

#endif