#include "AIGameEx1.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <typeinfo>
#include <type_traits>

#include "../definitions.h"
#include "../logger.h"
#include "../utilities.h"
#include "../game.h"
#include "../noise.h"

namespace AIExample {

	/////////////////////////
	//Function definitions.//
	/////////////////////////

	float miningAIGameFitness(unsigned int individualID) {

		if (miningAIGame* game = dynamic_cast<miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())) {

			GeneticNeuralNetwork& individual = game->getGenetic().individual(individualID);
			VoxelEng::vec3 posBox1,
						   posBox2;
			short blockObtained = 0;
			bool hasObtainedBlock = false;
			VoxelEng::blockViewDir blockViewDir = VoxelEng::blockViewDir::NONE;
			unsigned int depth = game->visionDepth(),
				         radius = game->visionRadius(),
				         action = 0;
			std::vector<VoxelEng::numericShortID> seenBlocks;
			unsigned int remainingActions = game->nInitialActions(),
				         nActionsNoCostPerformed = 0;


			while (remainingActions) {

				// Get input for the neural network.
				// That is, get the blocks in front of where the
				// agent is looking, the agent's position in the y-axis
				// and the direction it is currently looking at.

				posBox1 = game->getEntityPos(individualID);
				posBox2 = posBox1;

				blockViewDir = game->getBlockViewDir(individualID);
				const VoxelEng::vec3& pos = game->getEntityPos(individualID);

				// Get information related to the direction the agent is looking at.
				switch (blockViewDir) {
				
					case VoxelEng::blockViewDir::PLUSY:

						posBox1.y += 1;
						posBox1.x += radius;
						posBox1.z += radius;

						posBox2.y += depth;
						posBox2.x -= radius - 1;
						posBox2.z -= radius - 1;

						break;

					case VoxelEng::blockViewDir::NEGY:

						posBox1.y -= 1;
						posBox1.x -= radius;
						posBox1.z -= radius;

						posBox2.y -= depth;
						posBox2.x += radius - 1;
						posBox2.z += radius - 1;

						break;

					case VoxelEng::blockViewDir::PLUSX:

						posBox1.x += 1;
						posBox1.y += radius;
						posBox1.z += radius;

						posBox2.x += depth;
						posBox2.y -= radius - 1;
						posBox2.z -= radius - 1;

						break;

					case VoxelEng::blockViewDir::NEGX:

						posBox1.x -= 1;
						posBox1.y -= radius;
						posBox1.z -= radius;

						posBox2.x -= depth;
						posBox2.y += radius - 1;
						posBox2.z += radius - 1;

						break;

					case VoxelEng::blockViewDir::PLUSZ:

						posBox1.z += 1;
						posBox1.x += radius;
						posBox1.y += radius;

						posBox2.z += depth;
						posBox2.x -= radius - 1;
						posBox2.y -= radius - 1;

						break;

					case VoxelEng::blockViewDir::NEGZ:

						posBox1.z -= 1;
						posBox1.x -= radius;
						posBox1.y -= radius;

						posBox2.z -= depth;
						posBox2.x += radius - 1;
						posBox2.y += radius - 1;

						break;
				
				}

				// Get blocks seen by the agent.
				seenBlocks.clear();
				const std::vector<const VoxelEng::block*> namespacedBlockBox = game->getBlocksBox(posBox1, posBox2);
				for (auto it = namespacedBlockBox.cbegin(); it != namespacedBlockBox.cend(); it++)
					seenBlocks.push_back(game->getInternalID(**it));

				// Insert agent's network input parameters.
				std::vector<int> networkInput(seenBlocks.cbegin(), seenBlocks.cend());
				networkInput.push_back(pos.y);
				networkInput.push_back(static_cast<unsigned int>(blockViewDir));
				
				// Pass obtained input to the neural network to get
				// the action to perform.
				action = individual.forwardPropagationMax<int>(networkInput);

				switch (action) {
				
					case 0: // Move forward to the direction the agent is looking at.
						game->moveEntity(individualID, VoxelEng::uDirectionToVec3(blockViewDir));
						remainingActions--;
						break;

					case 1: // Rotate agent 90º degrees in the X axis.
						game->rotateAgentViewDir(individualID, VoxelEng::blockViewDir::PLUSX);
						nActionsNoCostPerformed++;
						break;

					case 2:  // Rotate agent -90º degrees in the X axis.
						game->rotateAgentViewDir(individualID, VoxelEng::blockViewDir::NEGX);
						nActionsNoCostPerformed++;
						break;

					case 3:  // Rotate agent 90º degrees in the Y axis.
						game->rotateAgentViewDir(individualID, VoxelEng::blockViewDir::PLUSY);
						nActionsNoCostPerformed++;
						break;

					case 4:  // Rotate agent -90º degrees in the Y axis.
						game->rotateAgentViewDir(individualID, VoxelEng::blockViewDir::NEGY);
						nActionsNoCostPerformed++;
						break;

					case 5: // Get block in front of the agent and get the points
							// corresponding to the block's type.
	
						switch (blockViewDir) {
					
						case VoxelEng::blockViewDir::PLUSX:

							if (game->isInWorld(pos.x + 1, pos.y, pos.z)) {
						
								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x + 1, pos.y, pos.z,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;
						
							}
						
							break;

						case VoxelEng::blockViewDir::NEGX:

							if (game->isInWorld(pos.x - 1, pos.y, pos.z)) {

								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x - 1, pos.y, pos.z,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;

							}

							break;

						case VoxelEng::blockViewDir::PLUSY:

							if (game->isInWorld(pos.x, pos.y + 1, pos.z)) {

								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x, pos.y + 1, pos.z,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;

							}
						
							break;

						case VoxelEng::blockViewDir::NEGY:

							if (game->isInWorld(pos.x, pos.y - 1, pos.z)) {

								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x, pos.y - 1, pos.z,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;

							}
						
							break;

						case VoxelEng::blockViewDir::PLUSZ:

							if (game->isInWorld(pos.x, pos.y, pos.z + 1)) {

								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x, pos.y, pos.z + 1,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;

							}
						
							break;

						case VoxelEng::blockViewDir::NEGZ:

							if (game->isInWorld(pos.x, pos.y, pos.z - 1)) {

								game->selectAIworld(individualID);
								blockObtained = game->getInternalID(game->setBlock(individualID, pos.x, pos.y, pos.z - 1,
									VoxelEng::block::emptyBlock(), game->recordAgentModifiedBlocks()));
								hasObtainedBlock = true;

							}
						
							break;
					
						}

						if (hasObtainedBlock) {
					
							game->addScore(individualID, game->blockScore(blockObtained));
							hasObtainedBlock = false;
					
						}

						remainingActions--;

						break;
				
				}

				// End of "turn".
				if (nActionsNoCostPerformed >= 10) {
				
					remainingActions--;

					nActionsNoCostPerformed = 0;
				
				}
			
			}

			return game->getScore(individualID);

		}
		else
			VoxelEng::logger::errorLog("Loaded AI game is not a mining AI game");
	}


	// 'miningWorldGen' class.

	const std::uniform_int_distribution<unsigned int> miningWorldGen::int6Dice_(1, 6);
	std::uniform_int_distribution<unsigned int> miningWorldGen::intDice_(1, 100);
	std::uniform_real_distribution<float> miningWorldGen::floatDice_(1.0f, 100.f);


	void miningWorldGen::prepareGen_() {

		chunkColHeight_.clear();
		chunkColHeightUses_.clear();

		minHeight_ = 0.0f;
		maxHeight_ = 200.0f;

		playerSpawnPos_.x = 0;
		playerSpawnPos_.y = chunkHeightMap_({ 0, 0 }, false)[0][0] + 10;
		playerSpawnPos_.z = 0;

		AISpawnPos_.x = 0;
		AISpawnPos_.y = chunkHeightMap_({ 0, 0 }, false)[0][0];
		AISpawnPos_.z = 0;

		VoxelEng::chunkManager::onChunkLoad().attach(chunkLoadListener_);
		VoxelEng::chunkManager::onChunkUnload().attach(chunkUnloadListener_);
	
	}

	void miningWorldGen::generate_(VoxelEng::chunk& chunk) {

		VoxelEng::vec3 chunkPos = chunk.chunkPos(),
					   blockPos;
		const chunkHeightMap& heightMap = chunkHeightMap_({ chunkPos.x, chunkPos.z });
		const chunkHeightMap& heightMapPlusX = chunkHeightMap_({ chunkPos.x+1, chunkPos.z });
		const chunkHeightMap& heightMapMinusX = chunkHeightMap_({ chunkPos.x-1, chunkPos.z });
		const chunkHeightMap& heightMapPlusZ = chunkHeightMap_({ chunkPos.x, chunkPos.z+1 });
		const chunkHeightMap& heightMapMinusZ = chunkHeightMap_({ chunkPos.x, chunkPos.z-1 });
		int height;
		int x, y, z;
		// TODO. Optimise when terrainFeature class and ore subclass are properly defined.

		for (x = 0; x < VoxelEng::SCX; x++)
			for (z = 0; z < VoxelEng::SCZ; z++)
				for (y = 0; y < VoxelEng::SCY; y++) {

					blockPos = VoxelEng::getGlobalPos(chunkPos, x, y, z);
					height = heightMap[x][z];

					if (blockPos.y < height - 3)
						chunk.setBlock(x, y, z, layer2_, false);
					else if (blockPos.y < height)
						chunk.setBlock(x, y, z, layer1_, false);
					else if (blockPos.y == height)
						chunk.setBlock(x, y, z, layer0_, false);

				}

		// Set neighbor blocks.
		// X+
		for (y = 0; y < VoxelEng::SCY; y++)
			for (z = 0; z < VoxelEng::SCZ; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x+1, chunkPos.y, chunkPos.z, 0, y, z);
				height = heightMapPlusX[0][z];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::PLUSX, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::PLUSX, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::PLUSX, layer0_);

			}

		// X-
		for (y = 0; y < VoxelEng::SCY; y++)
			for (z = 0; z < VoxelEng::SCZ; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x-1, chunkPos.y, chunkPos.z, VoxelEng::SCX-1, y, z);
				height = heightMapMinusX[VoxelEng::SCX-1][z];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::NEGX, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::NEGX, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(y, z, VoxelEng::blockViewDir::NEGX, layer0_);

			}

		// Y+
		for (x = 0; x < VoxelEng::SCX; x++)
			for (z = 0; z < VoxelEng::SCZ; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y+1, chunkPos.z, x, 0, z);
				height = heightMap[x][z];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::PLUSY, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::PLUSY, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::PLUSY, layer0_);

			}

		// Y-
		for (x = 0; x < VoxelEng::SCX; x++)
			for (z = 0; z < VoxelEng::SCZ; z++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y-1, chunkPos.z, x, VoxelEng::SCY-1, z);
				height = heightMap[x][z];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::NEGY, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::NEGY, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(x, z, VoxelEng::blockViewDir::NEGY, layer0_);

			}

		// Z+
		for (x = 0; x < VoxelEng::SCX; x++)
			for (y = 0; y < VoxelEng::SCY; y++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z+1, x, y, 0);
				height = heightMapPlusZ[x][0];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::PLUSZ, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::PLUSZ, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::PLUSZ, layer0_);

			}

		// Z-
		for (x = 0; x < VoxelEng::SCX; x++)
			for (y = 0; y < VoxelEng::SCY; y++) {

				blockPos = VoxelEng::getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z-1, x, y, VoxelEng::SCZ - 1);
				height = heightMapMinusZ[x][VoxelEng::SCZ-1];

				if (blockPos.y < height - 3)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::NEGZ, layer2_);
				else if (blockPos.y < height)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::NEGZ, layer1_);
				else if (blockPos.y == height)
					chunk.setBlockNeighbor(x, y, VoxelEng::blockViewDir::NEGZ, layer0_);

			}

		

	}

	const chunkHeightMap& miningWorldGen::chunkHeightMap_(int chunkX, int chunkZ, bool countUse) {

		VoxelEng::vec2 chunkXZPos{ chunkX, chunkZ };

		chunkColHeightMutex_.lock();
		bool found = chunkColHeight_.find(chunkXZPos) != chunkColHeight_.cend();
		chunkColHeightMutex_.unlock();
		if (!found)
			generateChunkHeightMap_(chunkXZPos);
			
		std::unique_lock<std::mutex> lock(chunkColHeightMutex_);

		return chunkColHeight_.at(chunkXZPos);
	
	}

	void miningWorldGen::generateChunkHeightMap_(const VoxelEng::vec2& chunkXZPos) {

		chunkHeightMap heightsAux = std::array<std::array<int, VoxelEng::SCZ>, VoxelEng::SCX>();
		float softnessFactor = 64,
			  height;
		VoxelEng::vec2 pos,
			           aux;
		glm::vec3 perlinCoords;
		for (pos.x = 0u; pos.x < VoxelEng::SCX; pos.x++) // x
			for (pos.y = 0u; pos.y < VoxelEng::SCZ; pos.y++) { // z

				// Height here is between -1.0 and 1.0.
				#if GRAPHICS_API == OPENGL

					aux = VoxelEng::getXZGlobalPos(chunkXZPos, pos) / softnessFactor;

					perlinCoords.x = aux.x;
					perlinCoords.y = aux.y;
					perlinCoords.z = seed_ + 0.1;

					height = VoxelEng::noise::perlin3D(perlinCoords.x, perlinCoords.y, perlinCoords.z);

				#else



				#endif
			
				// Make height value between 0.0 and 200.0.
				heightsAux[pos.x][pos.y] = VoxelEng::translateRange(height, -1.0f, 1.0f, 0.0f, maxHeight_);

			}

		chunkColHeightMutex_.lock();
		chunkColHeight_[chunkXZPos] = heightsAux;
		chunkColHeightMutex_.unlock();
	
	}

	void miningWorldGen::generateOre_(VoxelEng::vec3 inChunkPos, VoxelEng::chunk& chunk, const VoxelEng::block& ore) {

		std::uniform_int_distribution<unsigned int>::param_type* oreSpread = nullptr;

		// TODO. Optimise when terrainFeature class and ore subclass are properly defined.

		if (ore == ore1_)
			oreSpread = &ore1SpreadRange_;
		else {
			
			if (ore == ore2_)
				oreSpread = &ore2SpreadRange_;
			else {
			
				if (ore == ore3_)
					oreSpread = &ore3SpreadRange_;
				else {
				
					if (ore == ore4_)
						oreSpread = &ore4SpreadRange_;
					else
						VoxelEng::logger::errorLog("Block " + ore.name() + " is not registered as an ore block");

				}
					
			}
		
		}

		unsigned int nBlocks = intDice_(generator_, *oreSpread);
		for (unsigned int i = 0; i < nBlocks; i++) {

			chunk.setBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, ore);

			switch (int6Dice_(generator_)) {

			case 1:

				if (inChunkPos.x + 1 >= VoxelEng::SCX) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedNorth, i, nBlocks, 0, inChunkPos.y, inChunkPos.z, ore);
						
					i = nBlocks;

				}
				else
					inChunkPos.x++;

				break;

			case 2:

				if (inChunkPos.x == 0) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedSouth, i, nBlocks, VoxelEng::SCX - 1, inChunkPos.y, inChunkPos.z, ore);
						
					i = nBlocks;

				}
				else
					inChunkPos.x--;

				break;

			case 3:

				if (inChunkPos.y + 1 >= VoxelEng::SCY) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedUp, i, nBlocks, inChunkPos.x, 0, inChunkPos.z, ore);
						
					i = nBlocks;

				}
				else
					inChunkPos.y++;

				break;

			case 4:

				if (inChunkPos.y == 0) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedDown, i, nBlocks, inChunkPos.x, VoxelEng::SCY - 1, inChunkPos.z, ore);
						
					i = nBlocks;

				}
				else
					inChunkPos.y--;

				break;

			case 5:

				if (inChunkPos.z + 1 >= VoxelEng::SCZ) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedEast, i, nBlocks, inChunkPos.x, inChunkPos.y, 0, ore);

					i = nBlocks;

				}
				else
					inChunkPos.z++;

				break;

			case 6:

				if (inChunkPos.z == 0) {

					cascadeOreGen_(chunk.chunkPos() + VoxelEng::vec3FixedWest, i, nBlocks, inChunkPos.x, inChunkPos.y, VoxelEng::SCZ - 1, ore);
					
					i = nBlocks;

				}
				else
					inChunkPos.z--;

				break;

			}

		}

	}

	// TODO. FIX THIS
	void miningWorldGen::cascadeOreGen_(const VoxelEng::vec3 chunkPos, unsigned int& nBlocksCounter, unsigned int nBlocks,
		unsigned int inChunkX, unsigned int inChunkY, unsigned int inChunkZ, const VoxelEng::block& oreID) {

		VoxelEng::chunk* cascadeChunk = nullptr;

		if (!VoxelEng::chunkManager::isChunkInWorld(chunkPos)) {
			//cascadeChunk = VoxelEng::chunkManager::createChunk(false, chunkPos);
		}
		else {
		
			cascadeChunk = VoxelEng::chunkManager::selectChunk(chunkPos);

			if (cascadeChunk->status() == VoxelEng::chunkStatus::NOTLOADED)
				worldGen::generate(*cascadeChunk);
		
		}

		unsigned int spreadDirection = 0;
		for (nBlocksCounter; nBlocksCounter < nBlocks; nBlocksCounter++) {

			if (!cascadeChunk->isEmptyBlock(inChunkX, inChunkY, inChunkZ)) {

				cascadeChunk->setBlock(inChunkX, inChunkY, inChunkZ, oreID);

				switch (spreadDirection = int6Dice_(generator_)) {

					case 1:

						if (inChunkX + 1 >= VoxelEng::SCX)
							nBlocksCounter = nBlocks; // Stop generating because we do not want ore clusters					 
						else						  // that expand to 3 chunks or that returns the generation between 2 chunks more than one time.
							inChunkX++;

						break;

					case 2:

						if (inChunkX == 0)
							nBlocksCounter = nBlocks;
						else
							inChunkX--;

							break;

					case 3:

						if (inChunkY + 1 >= VoxelEng::SCY)
							nBlocksCounter = nBlocks;
						else
							inChunkY++;

							break;

					case 4:

						if (inChunkY == 0)
							nBlocksCounter = nBlocks;
						else
							inChunkY--;

							break;

					case 5:

						if (inChunkZ + 1 >= VoxelEng::SCZ)
							nBlocksCounter = nBlocks;
						else
							inChunkZ++;

							break;

					case 6:

						if (inChunkZ == 0)
							nBlocksCounter = nBlocks;
						else
							inChunkZ--;

							break;

					}

			}

		}

	}

	void miningWorldGen::clear_() {
	
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


	// 'miningAIGame' class.

	void miningAIGame::addScoreAt(unsigned int individualID, float score) {

		if (isAgentRegistered(individualID))
			addScore(individualID, score);

	}

	float miningAIGame::getScoreAt(unsigned int individualID) const {
	
		if (isAgentRegistered(individualID))
			return getScore(individualID);
		else
			VoxelEng::logger::errorOutOfRange("Attempted to get score from individual " + std::to_string(individualID) + " which is not registered");
	
	}

	int miningAIGame::loadAgentsData(const std::string& path) {

		if (genetic_.simInProgress())
			VoxelEng::logger::errorLog("Cannot load individual data while a simulation is in progress");
		else
			return loadAgentsData_(path);

	}

	void miningAIGame::saveAgentsData(const std::string& path) {

		if (genetic_.simInProgress())
			saveAgentsData_(path);
		else
			VoxelEng::logger::errorLog("Cannot save individual data while no simulation is in progress");

	}

	void miningAIGame::registerBlock(const std::string& block) {
	
		if (VoxelEng::block::isBlockRegistered(block)) {

			if (blockPalette_.contains(block))
				VoxelEng::logger::errorLog("Block " + block + " is already registered in this AI game");
			else {
			
				short internalID = 0;

				if (freeInternalIDs_.empty())
					internalID = blockPalette_.size();
				else {

					internalID = *freeInternalIDs_.cbegin();
					freeInternalIDs_.erase(internalID);

				}

				blockPalette_[block] = internalID;
				inverseBlockPalette_[internalID] = block;
			
			}
			
		}
		else
			VoxelEng::logger::errorLog("Block " + block + " is not registered in the engine");
	
	}

	void miningAIGame::registerBlockWithScore(const std::string& block, float score) {

		if (VoxelEng::block::isBlockRegistered(block)) {

			if (blockPalette_.contains(block))
				VoxelEng::logger::errorLog("Block " + block + " is already registered in this AI game");
			else {

				short internalID = 0;

				if (freeInternalIDs_.empty())
					internalID = blockPalette_.size();
				else {

					internalID = *freeInternalIDs_.cbegin();
					freeInternalIDs_.erase(internalID);

				}

				blockPalette_[block] = internalID;
				inverseBlockPalette_[internalID] = block;
				blockScore_[internalID] = score;

			}

		}
		else
			VoxelEng::logger::errorLog("Block " + block + " is not registered in the engine");

	}

	void miningAIGame::unregisterBlock(const std::string& block) {

		if (blockPalette_.contains(block)) {

			short internalID = blockPalette_[block];
			freeInternalIDs_.insert(internalID);
			blockPalette_.erase(block);
			inverseBlockPalette_.erase(internalID);
			blockScore_.erase(internalID);

		}
		else
			VoxelEng::logger::errorLog("Block " + block + " is not registered in this AI game");

	}

	void miningAIGame::setBlockScore(const std::string& block, float score) {
	
		if (isBlockRegistered(block)) {

			short internalID = blockPalette_[block];

			if (blockScore_.contains(internalID))
				blockScore_[internalID] = score;
			else
				VoxelEng::logger::errorLog("Block " + block + " is not registered to have an score");

		}
		else
			VoxelEng::logger::errorLog("Block " + block + " is not registered in this AI game");
		
	
	}

	float miningAIGame::blockScore(short internalBlockID) const {

		if (blockScore_.contains(internalBlockID))
			return blockScore_.at(internalBlockID);
		else
			VoxelEng::logger::errorLog("Block " + inverseBlockPalette_.at(internalBlockID) + " is not registered to have an score");

	}

	

	void miningAIGame::generalSetUp_() {

		registerBlockWithScore(VoxelEng::block::emptyBlock().name(), 0.0f);
		registerBlockWithScore("starminer::grass", 0.5f);
		registerBlockWithScore("starminer::dirt", 1.0f);
		registerBlockWithScore("starminer::stone", 2.5f);
		registerBlockWithScore("starminer::coalOre", 5.0f);
		registerBlockWithScore("starminer::ironOre", 10.0f);
		registerBlockWithScore("starminer::goldOre", 25.0f);

		visionDepth_ = 3;
		visionRadius_ = 3;
		nInputs_ = visionDepth_ * 2 * visionRadius_ * 2 * visionRadius_ + 2; // + 2 -> height position (position in the y-axis) and the fixed direction (unsigned int) the bot is looking at.

		genetic_.setGame();
		genetic_.setFitnessFunction(miningAIGameFitness);
		genetic_.setNThreads();

		VoxelEng::worldGen::setSeed();
		if (!VoxelEng::worldGen::isGenRegistered("miningWorldGen"))
			VoxelEng::worldGen::registerGen<AIExample::miningWorldGen>("miningWorldGen");
		VoxelEng::worldGen::selectGen("miningWorldGen");
		
	
	}

	void miningAIGame::displayMenu_() {

		unsigned int chosenOption = 0;
		do {

			VoxelEng::logger::say("[Mining AI game]\n"
								  "Select an option:\n"
								  "1). Train AI agents starting with random weights\n"
								  "2). Train AI agents starting with some stored AI agent data\n"
								  "3). Test AI agents with random weights\n"
								  "4). Test AI agents with some stored AI agent data\n"
								  "5). Generate a record of a match with agents with random weights\n"
								  "6). Generate a record of a match with agents with some stored AI agent data\n"
								  "7). Play the record of a match\n"
								  "8). Go back");

			while (!VoxelEng::validatedCinInput<unsigned int>(chosenOption) || chosenOption == 0 || chosenOption > 8)
				VoxelEng::logger::say("Invalid option. Please try again");

			std::string recordFileName,
						agentDataPath,
				        AIDataDirectory = "AIData/" + name_ + '/',
				        recordsDirectory = "records/" + name_ + '/';
			bool repeat,
				 overwrite = false,
				 overwriteRecordName = false,
				 writeAgentPath = true;
			std::string response;
			unsigned int opExitStatus = 0,
				         nEpochs = 0,
				         nAgents = 0;
			switch (chosenOption) {
			
				case 1:

					VoxelEng::logger::say("Enter how many agents are to be created");
					while (!VoxelEng::validatedCinInput<unsigned int>(nAgents))
						VoxelEng::logger::say("Invalid option. Please try again");

					VoxelEng::logger::say("Enter how many epochs are to be computed");
					while (!VoxelEng::validatedCinInput<unsigned int>(nEpochs))
						VoxelEng::logger::say("Invalid option. Please try again");

					setUpTraining_(nAgents, nEpochs);
					train_();

					break;

				case 2:


					VoxelEng::logger::say("Enter how many epochs are to be computed");
					while (!VoxelEng::validatedCinInput<unsigned int>(nEpochs))
						VoxelEng::logger::say("Invalid option. Please try again");

					VoxelEng::logger::say("Type the file path that holds the AI agent data (starting from inside the correspondent "
										  "AIData directory to this AI game).");
					VoxelEng::validatedCinInput<std::string>(agentDataPath);

					setUpTraining_(0, nEpochs); // nAgents is determined from the loaded .aidata file.
					while (!trainLoadedAgents_(AIDataDirectory + agentDataPath)) {
				
						VoxelEng::logger::say("AIData file not found. Please type another file name.");
						VoxelEng::validatedCinInput<std::string>(agentDataPath);
				
					}

					break;

				case 3:

					VoxelEng::logger::say("Enter how many agents are to be created");
					while (!VoxelEng::validatedCinInput<unsigned int>(nAgents))
						VoxelEng::logger::say("Invalid option. Please try again");

					VoxelEng::logger::say("Enter how many epochs are to be computed");
					while (!VoxelEng::validatedCinInput<unsigned int>(nEpochs))
						VoxelEng::logger::say("Invalid option. Please try again");
					setUpTest_(nAgents, nEpochs);
					test_();

					break;

				case 4:


					VoxelEng::logger::say("Enter how many epochs are to be computed");
					while (!VoxelEng::validatedCinInput<unsigned int>(nEpochs))
						VoxelEng::logger::say("Invalid option. Please try again");
					setUpTest_(nAgents, nEpochs);

					VoxelEng::logger::say("Type the file path that holds the AI agent data (starting from inside the correspondent "
										  "AIData directory to this AI game).");
					VoxelEng::validatedCinInput<std::string>(agentDataPath);

					while (!testLoadedAgents_(AIDataDirectory + agentDataPath)) {

						VoxelEng::logger::say("File not found. Please type another file name.");
						VoxelEng::validatedCinInput<std::string>(agentDataPath);

					}

					break;

				case 5:

					do {
				
						repeat = false;

						if (!overwrite) {
					
							VoxelEng::logger::say("Type the record's file name.");
							VoxelEng::validatedCinInput<std::string>(recordFileName);
							overwrite = true;
					
						}

						opExitStatus = generateRecord_(recordsDirectory, recordFileName);
						resetMatch_();

						if (opExitStatus == 2) { // 'recordFileName' is invalid.
					
							repeat = true;
							overwrite = false;
					
						}
						else if (opExitStatus == 3) { // There is already a file at 'recordsDirectory' + 'recordFilename' + ".rec".

							repeat = true;

							VoxelEng::logger::say("Overwrite (Y/N, case insensitive)?");
						
							while (!VoxelEng::validatedCinInput<std::string>(response) || (response != "Y" && response != "N" && response != "y" && response != "n"))
								VoxelEng::logger::say("Invalid response. Type 'Y' for 'yes' and 'N' for 'no' (case insensitive).");

							if (response == "Y" || response == "y") {
						
								overwrite = true;
								std::filesystem::remove(recordsDirectory + recordFileName + ".rec");

							}
							else
								overwrite = false;
							
						}
				
					} while (repeat);

					break;

				case 6:

					do {

						repeat = false;

						if (writeAgentPath) {

							VoxelEng::logger::say("Type the AI agents' data file path (starting from inside the correspondent "
								"AIData directory to this AI game).");
							VoxelEng::validatedCinInput<std::string>(agentDataPath);
							writeAgentPath = false;

						}

						if (!overwriteRecordName) {

							VoxelEng::logger::say("Type the record's file name.");
							VoxelEng::validatedCinInput<std::string>(recordFileName);
							overwriteRecordName = true;

						}				

						opExitStatus = generateRecordLoadedAgents_(recordsDirectory, recordFileName, AIDataDirectory + agentDataPath);
						resetMatch_();

						if (opExitStatus == 2) { // Record file path is not valid.
					
							repeat = true;
							overwriteRecordName = false;
							writeAgentPath = false;

						}
						else if (opExitStatus == 3) { // Error: there is already a file in the place where the record is going to be saved.

							repeat = true;

							VoxelEng::logger::say("Record with the typed name found. Overwrite (Y/N, case insensitive)?");

							while (!VoxelEng::validatedCinInput<std::string>(response) || (response != "Y" && response != "N" && response != "y" && response != "n"))
								VoxelEng::logger::say("Invalid response. Type 'Y' for 'yes' and 'N' for 'no' (case insensitive).");

							if (response == "Y" || response == "y") {

								overwriteRecordName = true;
								std::filesystem::remove(recordsDirectory + recordFileName + ".rec");

							}
							else
								overwriteRecordName = false;

						}
						else if (opExitStatus == 4) { // "AI data file path is not valid"

							repeat = true;
							overwriteRecordName = true; // Do not ask again for record file name.
							writeAgentPath = true;
					
						}

					} while (repeat);

					break;

				case 7:

					VoxelEng::logger::say("Type the record's file path (starting from inside the correspondent "
										  "records directory to this AI game).");
					VoxelEng::validatedCinInput<std::string>(recordFileName);
					while (playRecord_(recordsDirectory + recordFileName) == 1) {
				
						VoxelEng::logger::say("Record not found. Try again.");
						VoxelEng::validatedCinInput<std::string>(recordFileName);
				
					}

					// Unique cleaning process after playback.
					AIagentEntityID_.clear();

					break;
			
			}

		} while (chosenOption != 8);

	}

	void miningAIGame::setUpTraining_(unsigned int nAgents, unsigned int nEpochs) {

		// Initialise required engine systems.
		if (!VoxelEng::chunkManager::initialised()) {
		
			VoxelEng::chunkManager::init();
			// TODOAI. if (!chunkManager::infiniteWorlds())
						// VoxelEng::chunkManager::setNChunksToCompute(VoxelEng::DEF_N_CHUNKS_TO_COMPUTE);
			VoxelEng::entityManager::init();
		
		}

		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		popSize_ = nAgents;
		scores_ = std::vector<float>(popSize_ * 2u, 0.0f);

		genetic_.setNetworkTaxonomy({ nInputs_, 50, 50, 50, 50, 50, 6 });
		genetic_.setCrossoverSplitPoint(0);

		if (popSize_) // This does not execute when loading agents from an .aidata file.
			genetic_.setMutationParameters(1.0f/popSize_, -0.5f, 0.5f);

		nEpochs_ = nEpochs;
		nEpochsBetweenSaves_ = 3;
		epochForNewWorld_ = 3;

	}

	void miningAIGame::train_() {

		// Begin training.
		setAISeed();
		genetic_.genInitPop(popSize_, -5.0f, 5.0f, true);
		genetic_.train(nEpochs_, nEpochsBetweenSaves_, epochForNewWorld_);

		resetMatch_();
	
	}

	void miningAIGame::setUpTest_(unsigned int nAgents, unsigned int nEpochs) {

		// Initialise required engine systems.
		if (!VoxelEng::chunkManager::initialised()) {

			VoxelEng::chunkManager::init();
			// TODOAI. if (!chunkManager::infiniteWorlds())
						// VoxelEng::chunkManager::setNChunksToCompute(VoxelEng::DEF_N_CHUNKS_TO_COMPUTE);
			VoxelEng::entityManager::init();

		}

		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		popSize_ = nAgents;
		scores_ = std::vector<float>(popSize_ * 2u, 0.0f);

		genetic_.setNetworkTaxonomy({ nInputs_, 50, 50, 50, 50, 50, 6 });

		nEpochs_ = nEpochs;
		epochForNewWorld_ = 3;
		
	}

	void miningAIGame::test_() {

		float* averageFitness = nullptr;
		unsigned int averageFitnessSize;


		setAISeed();
		genetic_.genInitPop(popSize_, -5.0f, 5.0f, false);
		genetic_.test(nEpochs_, averageFitness, averageFitnessSize, epochForNewWorld_);

		for (unsigned int i = 0; i < averageFitnessSize; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(i) + " average fitness is: " + std::to_string(*(averageFitness + i)));

		resetMatch_();

	}

	void miningAIGame::setUpRecord_(unsigned int nAgents) {

		// Initialise required engine systems.
		if (!VoxelEng::chunkManager::initialised()) {

			VoxelEng::chunk::init();
			VoxelEng::chunkManager::init();
			// TODOAI. if (!chunkManager::infiniteWorlds())
						// VoxelEng::chunkManager::setNChunksToCompute(VoxelEng::DEF_N_CHUNKS_TO_COMPUTE);
			VoxelEng::entityManager::init();
			
		}

		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		popSize_ = nAgents;
		scores_ = std::vector<float>(popSize_ * 2u, 0.0f);

		genetic_.setNetworkTaxonomy({ nInputs_, 50, 50, 50, 50, 50, 6 });

		nEpochs_ = 1;

	}

	void miningAIGame::record_() {

		setAISeed();
		genetic_.genInitPop(popSize_, -5.0f, 5.0f, false);
		genetic_.record(aiGame::recordFilename());
	
	}
	
	void miningAIGame::spawnAgents_(unsigned int nAgents) {

		try {
		
			const VoxelEng::vec3& spawnPos = dynamic_cast<miningWorldGen&>(VoxelEng::worldGen::selectedGen()).spawnPos();

			if (AIagentEntityID_.empty())
				for (unsigned int i = 0; i < nAgents; i++) {

					createAgent(agentsModelID_, spawnPos);
					scores_[i] = 0;

				}
			else // Reuse already created entity objects to spawn the agents if they exist.
				for (unsigned int i = 0; i < nAgents; i++) {
				
					scores_[i] = 0;
					setEntityPos(AIagentEntityID_[i], spawnPos);
					rotateAgentViewDir(i, VoxelEng::blockViewDir::PLUSX);
				
				}
				
		}
		catch (const std::bad_cast e) {
		
			VoxelEng::logger::errorLog("Selected world generator is not a 'miningWorldGen' object");
			VoxelEng::game::setLoopSelection(VoxelEng::engineMode::EXIT);
		
		}

	}
	
	void miningAIGame::resetMatch_() {

		for (std::size_t i = 0; i < AIagentEntityID_.size(); i++)
			VoxelEng::entityManager::deleteEntity(AIagentEntityID_[i]);

		entityIDIsAgent.clear();
		AIagentEntityID_.clear();
		AIagentLookDirection_.clear();
		freeAIagentID_.clear();
		scores_.clear();
		VoxelEng::chunk::reset();
		VoxelEng::chunkManager::clear();
		VoxelEng::entityManager::clear();

	}

	void miningAIGame::resetGame_() {

		blockPalette_.clear();
		inverseBlockPalette_.clear();
		blockScore_.clear();
		freeInternalIDs_.clear();
		VoxelEng::worldGen::selectGen("default");
		VoxelEng::chunkManager::reset();
		VoxelEng::entityManager::reset();

	}

	bool miningAIGame::trainLoadedAgents_(const std::string& path) {

		if (!(popSize_ = loadAgentsData_(path)))
			return false;
		else if (popSize_ % 2 != 0)
			VoxelEng::logger::errorLog("The number of individuals to load must be even when training.");

		// Setup that must be performed here instead of in its usual method.
		scores_ = std::vector<float>(popSize_ * 2u, 0.0f);
		genetic_.setMutationParameters(1.0f / popSize_, -0.5f, 0.5f);

		genetic_.train(nEpochs_, nEpochsBetweenSaves_, epochForNewWorld_);

		resetMatch_();

		return true;

	}

	bool miningAIGame::testLoadedAgents_(const std::string& path) {

		unsigned int averageFitnessSize;
		float* averageFitness = nullptr;

		if (!(popSize_ = loadAgentsData_(path)))
			return false;

		// Setup that must be performed here instead of in its usual method.
		scores_ = std::vector<float>(popSize_ * 2u, 0.0f);
		genetic_.setMutationParameters(1.0f / popSize_, -0.5f, 0.5f);

		genetic_.test(nEpochs_, averageFitness, averageFitnessSize);

		for (unsigned int i = 0; i < averageFitnessSize; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(i) + " average fitness is: " + std::to_string(*(averageFitness + i)));

		resetMatch_();
		delete averageFitness;

		return true;

	}

	bool miningAIGame::recordLoadedAgents_(const std::string& path) {

		unsigned int nLoadedAgents = 0;
		if ((nLoadedAgents = loadAgentsData(path)) == 0)
			return false;
		else {
		
			setUpRecord_(nLoadedAgents);
			genetic_.record(aiGame::recordFilename());

			return true;
		
		}

	}
	
}