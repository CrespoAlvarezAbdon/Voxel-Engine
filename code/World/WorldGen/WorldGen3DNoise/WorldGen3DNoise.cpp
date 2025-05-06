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

		VoxelEng::chunkManager::onChunkLoad().attach(chunkLoadListener_);
		VoxelEng::chunkManager::onChunkUnload().attach(chunkUnloadListener_);

		noise_gen_.SetSeed(seed_);
		noise_gen_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		noise_gen_.SetFrequency(0.01f);
	
	}

	void WorldGen3DNoise::generate_(VoxelEng::chunk& chunk) {

		VoxelEng::vec3 chunkPos = chunk.chunkPos(),
					   blockPos;
		int x, y, z;
		const float threshold = 0.0f;
		const float waterLevel = 64;
		const float perc = 0.002f;

		for (x = 0; x < VoxelEng::CHUNK_SIZE; x++)
			for (z = 0; z < VoxelEng::CHUNK_SIZE; z++)
				for (y = 0; y < VoxelEng::CHUNK_SIZE; y++) {

					blockPos = VoxelEng::getGlobalPos(chunkPos, x, y, z);

					float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
					float density = noise - ((float)blockPos.y - waterLevel) * perc;
					if (density > threshold)
						chunk.setBlock(x, y, z, layer2_, false);

					//std::cout << density << " for " << blockPos.y << std::endl;
				}

		// Set neighbor blocks. TODO. OPTIMIZE THIS.
		// X+
		for (y = 0; y < VoxelEng::CHUNK_SIZE; y++)
			for (z = 0; z < VoxelEng::CHUNK_SIZE; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x+1, chunkPos.y, chunkPos.z, 0, y, z);

				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(y, z, blockViewDir::PLUSX, layer2_, false);

			}

		// X-
		for (y = 0; y < VoxelEng::CHUNK_SIZE; y++)
			for (z = 0; z < VoxelEng::CHUNK_SIZE; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x-1, chunkPos.y, chunkPos.z, VoxelEng::CHUNK_SIZE_LIMIT, y, z);
				
				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(y, z, blockViewDir::NEGX, layer2_, false);

			}

		// Y+
		for (x = 0; x < VoxelEng::CHUNK_SIZE; x++)
			for (z = 0; z < VoxelEng::CHUNK_SIZE; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y+1, chunkPos.z, x, 0, z);
				
				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(x, z, blockViewDir::PLUSY, layer2_, false);

			}

		// Y-
		for (x = 0; x < VoxelEng::CHUNK_SIZE; x++)
			for (z = 0; z < VoxelEng::CHUNK_SIZE; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y-1, chunkPos.z, x, VoxelEng::CHUNK_SIZE_LIMIT, z);
				
				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(x, z, blockViewDir::NEGY, layer2_, false);

			}

		// Z+
		for (x = 0; x < VoxelEng::CHUNK_SIZE; x++)
			for (y = 0; y < VoxelEng::CHUNK_SIZE; y++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z+1, x, y, 0);
				
				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(x, y, blockViewDir::PLUSZ, layer2_, false);

			}

		// Z-
		for (x = 0; x < VoxelEng::CHUNK_SIZE; x++)
			for (y = 0; y < VoxelEng::CHUNK_SIZE; y++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z-1, x, y, VoxelEng::CHUNK_SIZE_LIMIT);
				
				float noise = noise_gen_.GetNoise(blockPos.x, blockPos.y, blockPos.z);
				float density = noise - ((float)blockPos.y - waterLevel) * perc;
				if (density > threshold)
					chunk.setBlockNeighbor(x, y, blockViewDir::NEGZ, layer2_, false);

			}

	}

	void WorldGen3DNoise::clear_() {
	
		VoxelEng::chunkManager::onChunkLoad().detachIfExists(chunkLoadListener_);
		VoxelEng::chunkManager::onChunkUnload().detachIfExists(chunkUnloadListener_);
	
	}


	// 'chunkLoadListener' class.
	
	void chunkLoadListener::onEvent(VoxelEng::event* e) {

		if (e == nullptr)
			VoxelEng::logger::errorLog("The provided event is null");
		else {

			VoxelEng::chunkEvent* aChunkEvent = dynamic_cast<VoxelEng::chunkEvent*>(e);
			if (std::is_polymorphic<VoxelEng::event>() && aChunkEvent == nullptr)
				VoxelEng::logger::errorLog("The chunkLoadListener is attached to the event '" + e->name() + "', which is not a chunkEvent object");
			else {
			
				chunkColHeightMutex_.lock();
				chunkColHeightUses_[aChunkEvent->chunkPosXZ()]++;
				chunkColHeightMutex_.unlock();
			
			}
				
		}


	}


	// 'chunkUnloadListener' class.

	void chunkUnloadListener::onEvent(VoxelEng::event* e) {
	
		if (e == nullptr)
			VoxelEng::logger::errorLog("The provided event is null");
		else {

			VoxelEng::chunkEvent* aChunkEvent = dynamic_cast<VoxelEng::chunkEvent*>(e);
			if (aChunkEvent == nullptr)
				VoxelEng::logger::errorLog("The chunkLoadListener is attached to the event " + e->name() + " , which is not a chunkEvent");
			else
			{

				const VoxelEng::vec2& chunkPosXZ = aChunkEvent->chunkPosXZ();
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