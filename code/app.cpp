#include "block.h"
#include "game.h"
#include <Utilities/Logger/logger.h>

// TODO.
// - HAY QUE HACER EN EL CHUNKVERTEXBUFFER UN MAP APARTE PARA LA GEOMETRÍA TRANSLUCIDA.
// - QUE SE MANDE LA SOLICITUD DE FREE CUANDO SE QUITE UN BLOQUE Y EL BLOQUE QUEDE VACÍO DE VÉRTICES.

// SANEAR EL INIT Y DEINIT DE GAME

int main() {

    try {
       
        // Start engine.
        VoxelEng::game::init();

        VoxelEng::game::mainLoop();

        // Exit game.
        VoxelEng::game::reset();

        return 0;

    } catch (std::exception e) {
    
        VoxelEng::logger::errorLog(e.what());
        VoxelEng::logger::say("Error was detected during engine execution. Shutting down.");
        return 1;
    
    }

}