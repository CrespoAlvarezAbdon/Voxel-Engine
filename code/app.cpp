// Built-in includes.
#include "AIAPI.h"
#include "block.h"
#include "game.h"
#include "logger.h"

// Users' includes.
#include "AI/AIGameEx1.h"


int main() {

    try {

        // THINGS TO DO.
        // 1º. QUE EN LA CLASE WORLD SE PONGA LOS ATRIBUTOS CORRESPONDIENTES AL PASO 2.
        // 
        // HASTA AQUÍ HECHO
        // 
        // 1.0001º. METER GLOBAL TICK FUNCTION CHECK ON PLAYER PARA CHECKEAR CUALQUIER COSA SUYA COMO LO SIGUIENTE QUE VAMOS A METER DE QUE SE HAGA
        // TP SI SE SALE DE UN MUNDO FINITO.
        // 1.001º. QUE SI PONES EN UNA KEYBIND CAMBIAR DE MUNDO INFINITO A NO FINITO NO TE DEJE SALIR DE UNA CIERTA ALTURA MÁXIMA Y SI LO HACES
        // TE META TP PARA DENTRO (YA DECIDES SI QUIERES QUE SEA AL 0,superficie,0 o te ponga las coordenadas que se salen de los límites a justo en el límite.
        // 
        // 1.01º. METER MÉTODO SAVE Y LOAD EN WORLD PARA ASI PODER GUARDAR COSAS COMO LO DE QUE SEA INFINITO O NO Y OTROS ATRIBUTOS DEL MUNDO
        // ANTES DE GUARDAR LOS CHUNKS-
        // 1.1º. QUE SE GUARDEN Y CARGUEN LOS SAVES.
        // 2º. QUE SE GUARDE Y CARGUE EN LOS SAVES SI UN MUNDO ES INFINITO O NO Y SI NO ES INFINITO CUANTO AREA Y ALTURA MAXIMA TIENEN
        // 4º. QUE EN LOS AIGAMES SE USE LO DE LOS PASOS 1 Y 2 PARA QUE CUANDO GRABES UN RECORD DE UN JUEGO NO INFINITO Y LO CARGUES
        // SE PONGA EL CHUNKMANAGER EN MODO CHUNKS NO INFINITOS.

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

    } catch (...) {
    
        VoxelEng::logger::say("Error was detected during engine execution. Shutting down.");
        return 1;
    
    }

}