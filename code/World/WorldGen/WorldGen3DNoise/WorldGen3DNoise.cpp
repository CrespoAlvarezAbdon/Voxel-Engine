#include "WorldGen3DNoise.h"
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <typeinfo>
#include <type_traits>

#include <definitions.h>
#include <game.h>
#include <utilities.h>
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	// 'WorldGen3DNoise' class.

	std::uniform_int_distribution<unsigned int> WorldGen3DNoise::int6Dice_(1, 6);
	std::uniform_int_distribution<unsigned int> WorldGen3DNoise::intDice_(1, 100);
	std::uniform_real_distribution<float> WorldGen3DNoise::floatDice_(1.0f, 100.f);


	void WorldGen3DNoise::prepareGen_() {

		chunkColHeight_.clear();
		chunkColHeightUses_.clear();

		minHeight_ = 0.0f;
		maxHeight_ = 200.0f;

		playerSpawnPos_.x = 0;
		playerSpawnPos_.y = 120;
		playerSpawnPos_.z = 0;

		AISpawnPos_.x = 0;
		AISpawnPos_.y = 120;
		AISpawnPos_.z = 0;

		chunkManager::onChunkLoad().attach(chunkLoadListener_);
		chunkManager::onChunkUnload().attach(chunkUnloadListener_);

		noise_gen_.SetSeed(seed_);
		noise_gen_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		noise_gen_.SetFrequency(0.028f);
	
	}

	void WorldGen3DNoise::generate_(chunk& chunk) {

		noiseLayer(chunk);

		surfaceLayer(chunk);

		chunk.clearBlockLight();

		chunk.recalculateBlockLight();

	}

	void WorldGen3DNoise::genPass2_(chunk& chunk) {

		// PLACEHOLDER FOR STRUCTURE GENERATION

	}

	void WorldGen3DNoise::clear_() {
	
		chunkManager::onChunkLoad().detachIfExists(chunkLoadListener_);
		chunkManager::onChunkUnload().detachIfExists(chunkUnloadListener_);
	
	}

	void WorldGen3DNoise::noiseLayer(chunk& chunk) {
	
		vec3 chunkPos = chunk.chunkPos(),
			 blockPos;
		int x, y, z;
		for (x = -1; x <= CHUNK_SIZE; x++)
			for (z = -1; z <= CHUNK_SIZE; z++)
				for (y = -1; y <= CHUNK_SIZE; y++) {

					blockPos = getGlobalPos(chunkPos, x, y, z);
					if (noiseMakesBlockAt(blockPos.x, blockPos.y, blockPos.z))
						chunk.setBlock(x, y, z, layer2_, false);
					
				}

	}

	void WorldGen3DNoise::surfaceLayer(chunk& chunk) {

		vec3 chunkPos = chunk.chunkPos(),
			 blockPos;
		int x, y, z;
		bool isAboveWaterLevel = false;
		bool isBlockEmpty = false;

		for (x = -1; x <= CHUNK_SIZE; x++)
			for (z = -1; z <= CHUNK_SIZE; z++)
				for (y = -1; y <= CHUNK_SIZE; y++) {

					blockPos = getGlobalPos(chunkPos, x, y, z);
					isAboveWaterLevel = blockPos.y > waterLevel_;
					isBlockEmpty = chunk.isEmptyBlock(x, y, z);
					if (isAboveWaterLevel && !isBlockEmpty) {
					
						if(!noiseMakesBlockAt(blockPos.x, blockPos.y+1, blockPos.z))
							chunk.setBlock(x, y, z, (x == 0 && z == 0) ? lightBlock_ : layer0_, false);
						else if(!noiseMakesBlockAt(blockPos.x, blockPos.y+2, blockPos.z) || !noiseMakesBlockAt(blockPos.x, blockPos.y+3, blockPos.z))
							chunk.setBlock(x, y, z, layer1_, false);

					}
					else if (!isAboveWaterLevel)
						chunk.setBlock(x, y, z, isBlockEmpty ? waterBlock_ : beachBlock_, false);
					
				}

		// TODO. METER QUE NBLOCKS DE LOS NEIGHBORS SEA SOLO PARA BLOQUES OPACOS Y CAMBIARLE EL NOMBRE A NOCCLUDINGNEIGHBORBLOCKS.

	}

	// 'chunkLoadListener' class.
	
	void chunkLoadListener::onEvent(event* e) {

		if (e == nullptr)
			logger::errorLog("The provided event is null");
		else {

			chunkEvent* aChunkEvent = dynamic_cast<chunkEvent*>(e);
			if (std::is_polymorphic<event>() && aChunkEvent == nullptr)
				logger::errorLog("The chunkLoadListener is attached to the event '" + e->name() + "', which is not a chunkEvent object");
			else {
			
				chunkColHeightMutex_.lock();
				chunkColHeightUses_[aChunkEvent->chunkPosXZ()]++;
				chunkColHeightMutex_.unlock();
			
			}
				
		}


	}


	// 'chunkUnloadListener' class.

	void chunkUnloadListener::onEvent(event* e) {
	
		if (e == nullptr)
			logger::errorLog("The provided event is null");
		else {

			chunkEvent* aChunkEvent = dynamic_cast<chunkEvent*>(e);
			if (aChunkEvent == nullptr)
				logger::errorLog("The chunkLoadListener is attached to the event " + e->name() + " , which is not a chunkEvent");
			else
			{

				const vec2& chunkPosXZ = aChunkEvent->chunkPosXZ();
				chunkColHeightMutex_.lock();
				if (--chunkColHeightUses_.at(chunkPosXZ) == 0) {

					chunkColHeightUses_.erase(chunkPosXZ);
					chunkColHeight_.erase(chunkPosXZ);

				}
				chunkColHeightMutex_.unlock();

			}

		}

	}
	
}