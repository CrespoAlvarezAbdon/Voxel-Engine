// Built-in includes.
#include "AIAPI.h"
#include "block.h"
#include "game.h"
#include <Utilities/Logger/logger.h>

// Users' includes.
#include "AI/AIGameEx1.h"

// MAÑANA.
// WE FUCKING COOKIN
// CUANDO PONGAS UNA LU Y LA PROGAGUES, SI LA LU LLEGARÍA A ENTRAR EN UN CHUNK VECINO, HACER UN JOB DE REMESH ESPECIAL DONDE LE PASAS TODAS LAS LUCES
// QUE HAYAN ENTRADO EN ESE CHUNK.

// NEXT
// WE CONTINUE TO COOK
// CUANDO SE MANDE UN PENDINGCHUNKJOB DE MESH DE LIGHT DE NEIGHBOR, EL MANAGECHUNKS DE CHUNK MANAGER REVISARÁ SI ESE CHUNK YA ESTÁ MESHED O NO.
// SI LO ESTÁ O SE VA CARGAR ESE CHUNK (QUE EL CHUNK EXISTA EN CLIENTCHUNKS), HACER OTRO MESH PERO CON LAS LUCES PASADAS POR EL JOB, QUE NO, 
// SE PASA ESE JOB AL FINAL DE LA LISTA DE PENDING JOBS Y SI AL SELECCIONAR UN JOB DE ESTOS SE VE QUE EL CHUNK ESTÁ EN UNA POS FUERA 
// DEL RENDER DISTANCE DEL PLAYER, CANCELAR LA TAREA Y DEVOLVER EL JOB A LA POOL.


// TODO.
// - HAY QUE HACER EN EL CHUNKVERTEXBUFFER UN MAP APARTE PARA LA GEOMETRÍA TRANSLUCIDA.
// - QUE SE MANDE LA SOLICITUD DE FREE CUANDO SE QUITE UN BLOQUE Y EL BLOQUE QUEDE VACÍO DE VÉRTICES.

// SANEAR EL INIT Y DEINIT DE GAME

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