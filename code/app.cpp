// Built-in includes.
#include "game.h"
#include "AIAPI.h"
#include "logger.h"

// Users' includes.
#include "AI/AIGameEx1.h"


int main() {

    try {

        // Start engine.
        VoxelEng::game::init();

        // Initialize AI API and register AI games.
        VoxelEng::AIAPI::aiGame::init();
        VoxelEng::AIAPI::aiGame::registerGame<AIExample::miningAIGame>("MiningAIGame");

        VoxelEng::game::mainLoop();

        // Exit game.
        VoxelEng::game::cleanUp();
        VoxelEng::AIAPI::aiGame::cleanUp();

        return 0;

    } catch (...) {
    
        VoxelEng::logger::say("Error was detected during engine execution. Shutting down.");
        return 1;
    
    }

}