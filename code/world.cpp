#include "world.h"

#include "entity.h"
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

	void world::createSaveDirectory_(unsigned int slot) {

		currentWorldPath_ = "saves/slot" + std::to_string(slot);
		std::filesystem::create_directory(currentWorldPath_);
		currentWorldSlot_ = slot;

	}

	void world::saveAll_() {

		saveMainData_();
	
	}

	void world::saveMainData_() {
	
		std::ofstream mainFile(currentWorldPath_ + "mainData.world", std::ios::binary);
		std::string data;

		data += std::to_string(maxDistanceX_) + '|' + std::to_string(maxDistanceY_) + '|' + std::to_string(maxDistanceZ_) + '|' + 
			    std::to_string(player::globalPos());
		// NEXT.
		// 1º. METER DE NUEVO LA FUNCIONALIDAD A LOS BOTONES DE SLOT CUANDO SE GUARDA.
		// 2º. SEPARAR COMPLETAMENTE LA CLASE PLAYER EN SU PROPIO .H Y .CPP.
		// 3º. QUE LA CAMARA DEL PLAYER COJA LOS VALORES DEL TRANSFORM DEL PROPIO DEL PLAYER Y QUE TODO EL CONTROL DE MOVIMIENTO DEL PLAYER
		// ESTÉ EN PLAYER. ASÍ LUEGO SE PUEDEN HACER COSAS COMO EL MODO TERCERA PERSONA MÁS FÁCIL.

		mainFile.write(data.c_str(), data.size());
	
	}

	void world::saveChunk_(const vec3& chunkPos) {
	
	
	
	}

}