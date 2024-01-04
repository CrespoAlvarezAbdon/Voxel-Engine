#include "world.h"

#include <cstring>

#include "entity.h"
#include "player.h"
#include "game.h"
#include "logger.h"


namespace VoxelEng {

	// 'world' class.

	bool world::initialised_ = false;
	unsigned int world::maxDistanceX_ = 0,
				 world::maxDistanceY_ = 0,
				 world::maxDistanceZ_ = 0;
	std::unordered_map<std::string, tickFunc> world::globalTickFunctions_;
	std::unordered_set<std::string> world::activeTickFunctions_;
	std::mutex world::tickFunctionsMutex_;
	unsigned int world::currentWorldSlot_ = 0;
	std::string world::currentWorldPath_;
	database* world::regions_ = nullptr;


	void world::init() {
	
		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (initialised_)
			logger::errorLog("The world class is already initialised");
		else {

			maxDistanceX_ = 0;
			maxDistanceY_ = 0;
			maxDistanceZ_ = 0;

			currentWorldSlot_ = 0;
			currentWorldPath_.clear();

			regions_ = nullptr;

			initialised_ = true;
		
		}

	}

	void world::addGlobalTickFunction(const std::string& name, tickFunc func, bool active) {
	
		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (globalTickFunctions_.find(name) == globalTickFunctions_.cend()) {
		
			globalTickFunctions_[name] = func;

			if (active)
				activeTickFunctions_.insert(name);
		
		}
		else
			logger::errorLog("A global tick function with the name " + name + " is already registered.");
	
	}

	void world::changeStateGlobalTickFunction(const std::string& name) {

		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (globalTickFunctions_.find(name) == globalTickFunctions_.cend())
			logger::errorLog("A global tick function with the name " + name + " is not registered.");
		else {
		
			if (activeTickFunctions_.find(name) != activeTickFunctions_.cend())
				activeTickFunctions_.erase(name);
			else
				activeTickFunctions_.insert(name);
		
		}

	}

	void world::changeStateGlobalTickFunction(const std::string& name, bool active) {

		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (globalTickFunctions_.find(name) == globalTickFunctions_.cend())
			logger::errorLog("A global tick function with the name " + name + " is not registered.");
		else {

			if (active)
				activeTickFunctions_.insert(name);
			else
				activeTickFunctions_.erase(name);

		}

	}

	void world::deleteGlobalTickFunction(const std::string& name) {

		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (globalTickFunctions_.find(name) == globalTickFunctions_.cend())
			logger::errorLog("A global tick function with the name " + name + " is not registered.");
		else {

			globalTickFunctions_.erase(name);
			activeTickFunctions_.erase(name);

		}

	}

	void world::processWorldTicks() {

		try {

			// Lock any mutexes required for synchronisation with the rendering thread and used in the functions inside the loop below.
			entityManager::syncMutex().lock();

			while (game::threadsExecute[1]) {

				processGlobalTickFunctions();

				entityManager::manageEntities();

			}

			// Unlock any mutexes required for synchronisation with the rendering thread and used in the functions inside the loop below.
			entityManager::syncMutex().unlock();

		}
		catch (...) {
		
			VoxelEng::logger::say("Error was detected during engine execution. Shutting down entity and tick functions management thread.");
			game::setLoopSelection(engineMode::EXIT);
		
		}
	
	}

	void world::processGlobalTickFunctions() {
	
		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		for (auto it = activeTickFunctions_.cbegin(); it != activeTickFunctions_.cend(); it++)
			globalTickFunctions_[*it]();
	
	}

	void world::saveAll() {

		saveMainData_();

		saveAllChunks_();

	}

	void world::createSaveDirectory(unsigned int slot) {

		currentWorldSlot_ = slot;
		currentWorldPath_ = "saves/slot" + std::to_string(currentWorldSlot_) + '/';
		std::filesystem::create_directory(currentWorldPath_);

	}

	void world::reset() {
	
		std::unique_lock<std::mutex> lock(tickFunctionsMutex_);

		if (initialised_) {

			globalTickFunctions_.clear();
			activeTickFunctions_.clear();

			initialised_ = false;

		}
		else
			logger::errorLog("The world class is not initialised");
	
	}

	void world::saveMainData_() {
	
		std::ofstream mainFile(currentWorldPath_ + "mainData.world", std::ios::binary | std::ios::out | std::ios::app);
		std::string data;

		data += std::to_string(maxDistanceX_) + '|' + std::to_string(maxDistanceY_) + '|' + std::to_string(maxDistanceZ_) + '|' + 
			    std::to_string(player::globalPos()) + '|' + std::to_string(player::rotation()) + '|';

		mainFile.write(data.c_str(), data.size());
		mainFile.close();
	
	}

	void world::saveAllChunks_() {

		// NEXT.
		// 0º. QUE EL BOTON NEW PIDA ELEGIR EL SLOT EN EL QUE IR GUARDANDOLO TODO.
		// 1º. IMPLEMENTAR Y HACER PRUEBA DE GUARDADO CON TODOS LOS CHUNKS CARGADOS,
		// INCLUYENDO QUE SE GUARDEN LOS CHUNKS QUE SE DESCARGAN POR LEJANÍA AL JUGADOR.

		//if (regions)

		const std::unordered_map<vec3, chunk*>& chunks = chunkManager::chunks();
		for (auto it = chunks.cbegin(); it != chunks.cend(); it++)
			saveChunk_(it->second);

	}

	void world::saveChunk_(chunk* c) {

		c->blockDataMutex().lock();

		leveldb::Slice data((const char*)c->getBlocks(), sizeof(block*) * nBlocksChunk);

		c->blockDataMutex().unlock();

	}

}