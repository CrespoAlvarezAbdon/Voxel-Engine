#include "world.h"

#include <cstring>

#include "block.h"
#include "entity.h"
#include "palette.h"
#include "player.h"
#include "game.h"
#include "logger.h"
#include "utilities.h"
#include "worldGen.h"


namespace VoxelEng {

	// 'world' class.

	bool world::initialised_ = false;
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

		{

			using namespace std::chrono_literals;

			std::this_thread::sleep_for(1ms);

		}
	
	}

	void world::saveAll() {

		saveMainData();
		
		saveAllChunks_();

	}

	void world::saveMainData() {

		std::ofstream mainFile(currentWorldPath_ + "mainData.world", std::ios::binary | std::ios::out | std::ios::trunc);
		std::string data;

		data += std::to_string(player::globalPos()) + '|' + std::to_string(player::viewDirection()) + '|' + std::to_string(worldGen::getSeed()) + '|';

		mainFile.write(data.c_str(), data.size());
		mainFile.close();

	}

	void world::saveChunk(chunk* c) {

		if (regions_)
			regions_->insert(std::to_string(c->chunkPos()), chunkManager::serializeChunk(c));
		else
			logger::errorLog("The chunk cannot be saved becaused the regions database is not opened");

	}

	std::string world::loadChunk(const vec3& chunkPos) {
	
		if (regions_)
			return regions_->get(std::to_string(chunkPos));
		else
			logger::errorLog("The chunk cannot be saved becaused the regions database is not opened");
	}

	void world::loadMainData() {
	
		std::ifstream mainFile(currentWorldPath_ + "mainData.world", std::ios::binary);
		
		// 0 = Reading player's pos.
		// 1 = Reading player's rotation.
		// 2 = Reading world's seed.
		unsigned int state = 0;
		unsigned char fillingVec3Pos = 0;
		vec3 auxVec3;
		char c = mainFile.get();
		std::string word;
		while (!mainFile.eof()) {
		
			if (c == '|') {
			
				switch (state) {
				
				case 0:
					if (fillingVec3Pos != 2)
						logger::errorLog("The vector was not read properly");

					auxVec3[fillingVec3Pos] = sto<float>(word);
					fillingVec3Pos = 0;

					player::globalPos(auxVec3);
					break;

				case 1:
					if (fillingVec3Pos != 2)
						logger::errorLog("The vector was not read properly");

					auxVec3[fillingVec3Pos] = sto<float>(word);
					fillingVec3Pos = 0;

					player::viewDirection(auxVec3);
					break;

				case 2:
					worldGen::setSeed(sto<unsigned int>(word));
					break;

				default:
					logger::errorLog("Unknown reading state");
				
				}

				state++;
				word = "";

			}
			else if (c == ',') {
			
				auxVec3[fillingVec3Pos] = sto<float>(word);
				word = "";

				fillingVec3Pos++;
			
			}
			else {
			
				word += c;
			
			}

			c = mainFile.get();

		}

		mainFile.close();
	
	}

	void world::setupSaveDirectory() {

		currentWorldSlot_ = game::selectedSaveSlot();
		currentWorldPath_ = "saves/slot" + std::to_string(currentWorldSlot_) + '/';
		std::filesystem::create_directory(currentWorldPath_);

		if (regions_) {
		
			delete regions_;
			regions_ = nullptr;
		
		}

		regions_ = new database(currentWorldPath_ + "regions");

	}

	void world::clearSlot() {

		std::string path = "saves/slot" + std::to_string(game::selectedSaveSlot());
		if (std::filesystem::exists(path))
			std::filesystem::remove_all(path);
	
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

	

	void world::saveAllChunks_() {

		const std::unordered_map<vec3, chunk*>& chunks = chunkManager::chunks();
		for (auto it = chunks.cbegin(); it != chunks.cend(); it++)
			if (it->second->modified())
				saveChunk(it->second);

	}

}