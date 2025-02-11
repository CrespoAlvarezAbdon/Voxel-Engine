// Built-in includes.
#include "AIAPI.h"
#include "block.h"
#include "game.h"
#include <Utilities/Logger/logger.h>

// Users' includes.
#include "AI/AIGameEx1.h"

// TODO.
// - HAY QUE HACER EN EL CHUNKVERTEXBUFFER UN MAP APARTE PARA LA GEOMETRÍA TRANSLUCIDA.
// - QUE SE MANDE LA SOLICITUD DE FREE CUANDO SE QUITE UN BLOQUE Y EL BLOQUE QUEDE VACÍO DE VÉRTICES.

// SANEAR EL INIT Y DEINIT DE GAME

// TODO. SUBIR DE ALGUNA FORMA A VRAM EL BLOCK LIGHT DATA DE CADA CHUNK.

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