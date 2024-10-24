#include "worldGen.h"
#include <cstdlib>
#include "game.h"


namespace VoxelEng {

	// 'worldGen' class.

	vec3 worldGen::playerSpawnPos_ = vec3Zero;
	unsigned int worldGen::seed_ = 0;
	std::random_device worldGen::RD_;
	std::mt19937 worldGen::generator_(worldGen::RD_());
	std::uniform_int_distribution<unsigned int> worldGen::uDistribution_(0, std::numeric_limits<unsigned int>::max());
	std::uniform_int_distribution<unsigned int>::param_type worldGen::flatWorldBlockDistribution_(1, 3);
	std::atomic<bool> worldGen::isCreatingAllowed_ = false; // To restrict constructor's use.
	bool worldGen::initialised_ = false;
	std::unordered_map<std::string, worldGen*> worldGen::generators_;
	worldGen* worldGen::defaultGen_ = nullptr;
	worldGen* worldGen::selectedGen_ = nullptr;

	

	void worldGen::init() {
	
		if (initialised_)
			logger::errorLog("World generator system is already initialised");
		else {
		
			isCreatingAllowed_ = false;

			generators_ = {

				{"default", new defaultWorldGen()}

			};

			defaultGen_ = generators_["default"];
			selectedGen_ = defaultGen_;

			playerSpawnPos_ = vec3Zero;

			initialised_ = true;

		}
	
	}

	void worldGen::selectGenAt(const std::string& genName) {

		if (generators_.find(genName) == generators_.cend())
			logger::errorLog("World generator named " + genName + " is not registered");
		else
			selectGen(genName);

	}

	void worldGen::setSeed() {
	
		seed_ = uDistribution_(generator_);
		generator_.seed(seed_);
			
		logger::debugLog("World generator seed: " + std::to_string(seed_));

	}

	void worldGen::setSeed(unsigned int seed) {
	
		seed_ = seed;
		generator_.seed(seed_);
			
	}

	void worldGen::unregisterGen(const std::string& genName) {
	
		if (genName == "default")
			logger::errorLog("Cannot delete the default wolrd generator");
		else {
		
			if (generators_.find(genName) == generators_.cend())
				logger::errorLog("World generator named " + genName + " is not registered");
			else {

				worldGen* genToDelete = generators_[genName];

				if (genToDelete == selectedGen_)
					selectedGen_ = defaultGen_;

				delete genToDelete;
				generators_.erase(genName);

			}
		
		}
			
	}

	void worldGen::cleanUp() {

		for (auto it = generators_.begin(); it != generators_.cend(); it++)
			delete it->second;
		generators_.clear();

		defaultGen_ = nullptr;
		selectedGen_ = nullptr;

		initialised_ = false;
	
	}


	// 'defaultWorldGen' class.

	void defaultWorldGen::prepareGen_() {

		playerSpawnPos_.x = 0;
		playerSpawnPos_.y = 150;
		playerSpawnPos_.z = 0;

		setSeed();

	}

	void defaultWorldGen::generate_(chunk& chunk) {

		const vec3 chunkPos = chunk.chunkPos();

		for (GLbyte x = 0; x < SCX; x++)
			for (GLbyte y = 0; y < SCY; y++)
				for (GLbyte z = 0; z < SCZ; z++)
					chunk.setBlock(x, y, z, (chunkPos.y <= 8) * (uDistribution_(generator_, flatWorldBlockDistribution_)));

		chunk.setLoadLevel(VoxelEng::chunkLoadLevel::DECORATED);

	}

}