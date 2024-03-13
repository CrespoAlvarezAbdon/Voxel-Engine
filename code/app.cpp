// Built-in includes.
#include "AIAPI.h"
#include "block.h"
#include "game.h"
#include "logger.h"

// Users' includes.
#include "AI/AIGameEx1.h"


int main() {

    try {

        // Start engine.
        VoxelEng::game::init();
        
        // Load the engine's basic resources like registered blocks, items...
        VoxelEng::block::init();

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