// Built-in includes.
#include "AIAPI.h"
#include "block.h"
#include "game.h"
#include <Utilities/Logger/logger.h>

// Users' includes.
#include "AI/AIGameEx1.h"

// TODO.
// - REALMENTE CON ESTE SETUP NO HACE FALTA EL LIGHTINDEXTYPE EN LIGHTINSTANCE VERDAD???
// - LAS LUCES REALMENTE ESTÁN EN RENDERINGDATA PERO TAMBIÉN SE DEBERÍAN GUARDAR AL GUARDAR EL CHUNK EN DISCO.

int main() {

    try {

        // Start engine.
        VoxelEng::game::init();
        
        

        // Initialize AI API and register AI games.
        VoxelEng::AIAPI::aiGame::init();
        VoxelEng::AIAPI::aiGame::registerGame<AIExample::miningAIGame>("MiningAIGame");

        VoxelEng::game::mainLoop();

        // Exit game.
        VoxelEng::game::reset();
        VoxelEng::AIAPI::aiGame::reset();

        return 0;

    } catch (std::exception e) {
    
        VoxelEng::logger::errorLog(e.what());
        VoxelEng::logger::say("Error was detected during engine execution. Shutting down.");
        return 1;
    
    }

}