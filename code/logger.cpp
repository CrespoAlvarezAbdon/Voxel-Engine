#include "logger.h"
#include <iostream>
#include <ostream>
#include "game.h"


namespace VoxelEng {

	void logger::say(const std::string& msg) {

		std::cout << msg << std::endl;

	}

	void logger::log(const std::string& msg) {
	
		std::cout << "[log]: " << msg << std::endl;
	
	}

	void logger::debugLog(const std::string& msg) {

		std::cout << "[DEBUG]: " << msg << std::endl;

	}

	void logger::warningLog(const std::string& msg) {

		std::cout << "[WARNING]: " << msg << std::endl;

	}

	void logger::errorLog(const std::string& msg) {

		std::string errorMsg = "[ERROR]:" + msg;

		std::cout << errorMsg << std::endl;

		if (game::loopSelection() == GRAPHICALLEVEL) // Save a separate copy of the level just when the error happened just in case.
			chunkManager::saveAllChunks(chunkManager::openedTerrainFileName() + "ERRORED");

		game::cleanUp();

		throw std::runtime_error(errorMsg);

	}

	void logger::errorOutOfRange(const std::string& msg) {

		std::string errorMsg = "[ERROR]: Out of range. " + msg;

		std::cout << errorMsg << std::endl;

		game::cleanUp();

		throw std::out_of_range(errorMsg);

	}

}