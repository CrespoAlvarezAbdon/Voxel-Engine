#include "AIGameEx1.h"
#include <cmath>
#include <filesystem>
#include <typeinfo>
#include "../logger.h"
#include "../utilities.h"
#include "../glm/gtc/noise.hpp"
#include "../chunk.h"
#include "../worldGen.h"


namespace AIExample {

	/////////////////////////
	//Function definitions.//
	/////////////////////////

	float miningAIGameFitness(unsigned int individualID) {

		if (miningAIGame* game = dynamic_cast<miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())) {

			AI::GeneticNeuralNetwork& individual = game->getGenetic().individual(individualID);
			VoxelEng::vec3 posBox1,
						   posBox2;
			VoxelEng::block blockObtained = 0;
			bool hasObtainedBlock = false;
			unsigned int blockViewDir = 0,
					     depth = game->visionDepth(),
				         radius = game->visionRadius(),
				         action = 0;
			float result = 0.0f;
			std::vector<VoxelEng::block> seenBlocks;
			unsigned int remainingActions = game->nInitialActions(),
				         nActionsNoCostPerformed = 0;


			while (remainingActions) {
			
				// Get input for the neural network.
				// That is, get the blocks in front of where the
				// agent is looking, the agent's position in the y-axis
				// and the direction it is currently looking at.

				posBox1 = game->getEntityPos(individualID);
				posBox2 = posBox1;

				blockViewDir = game->blockViewDir(individualID);
				const VoxelEng::vec3& pos = game->getEntityPos(individualID);

				// Get information related to the direction the agent is looking at.
				switch (blockViewDir) {
				
				case PLUSY:

					posBox1.y += 1;
					posBox1.x += radius;
					posBox1.z += radius;

					posBox2.y += 1 + depth;
					posBox2.x -= radius;
					posBox2.z -= radius;

					break;

				case NEGY:

					posBox1.y -= 1;
					posBox1.x -= radius;
					posBox1.z -= radius;

					posBox2.y -= (1 + depth);
					posBox2.x += radius;
					posBox2.z += radius;

					break;

				case PLUSX:

					posBox1.x += 1;
					posBox1.y += radius;
					posBox1.z += radius;

					posBox2.x += 1 + depth;
					posBox2.y -= radius;
					posBox2.z -= radius;

					break;

				case NEGX:

					posBox1.x -= 1;
					posBox1.y -= radius;
					posBox1.z -= radius;

					posBox2.x -= (1 + depth);
					posBox2.y += radius;
					posBox2.z += radius;

					break;

				case PLUSZ:

					posBox1.z += 1;
					posBox1.x += radius;
					posBox1.y += radius;

					posBox2.z += 1 + depth;
					posBox2.x -= radius;
					posBox2.y -= radius;

					break;

				case NEGZ:

					posBox1.z -= 1;
					posBox1.x -= radius;
					posBox1.y -= radius;

					posBox2.z -= 1 + depth;
					posBox2.x += radius;
					posBox2.y += radius;

					break;
				
				}

				seenBlocks = game->getBlocksBox(posBox1, posBox2);
				std::vector<int> networkInput(seenBlocks.cbegin(), seenBlocks.cend());
				networkInput.push_back(pos.y);
				networkInput.push_back(blockViewDir);

				VoxelEng::logger::debugLog("Network's input true size: " + std::to_string(networkInput.size()));

				// Pass obtained input to the neural network to get
				// the action to perform.
				action = individual.forwardPropagationMax<int>(networkInput);

				switch (action) {
				
				case 0: // Move forward to the direction the agent is looking at.
					game->moveEntity(individualID, VoxelEng::uDirectionToVec3(blockViewDir));
					remainingActions--;
					break;

				case 1: // Rotate agent 90º degrees in the X axis.
					game->rotateAgentViewDir(individualID, PLUSX);
					nActionsNoCostPerformed++;
					break;

				case 2:  // Rotate agent -90º degrees in the X axis.
					game->rotateAgentViewDir(individualID, NEGX);
					nActionsNoCostPerformed++;
					break;

				case 3:  // Rotate agent 90º degrees in the Y axis.
					game->rotateAgentViewDir(individualID, PLUSY);
					nActionsNoCostPerformed++;
					break;

				case 4:  // Rotate agent -90º degrees in the Y axis.
					game->rotateAgentViewDir(individualID, NEGY);
					nActionsNoCostPerformed++;
					break;

				case 5: // Get block in front of the agent and get the points
						// corresponding to the block's type.
	
					switch (blockViewDir) {
					
					case PLUSX:

						if (game->isInWorld(pos.x + 1, pos.y, pos.z)) {
						
							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x + 1, pos.y, pos.z, 0, game->recordAgentModifiedBlocks());
							hasObtainedBlock = true;
						
						}
						
						break;

					case NEGX:

						if (game->isInWorld(pos.x - 1, pos.y, pos.z)) {

							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x - 1, pos.y, pos.z, 0, game->recordAgentModifiedBlocks());
							hasObtainedBlock = true;

						}

						break;

					case PLUSY:

						if (game->isInWorld(pos.x, pos.y + 1, pos.z)) {

							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x, pos.y + 1, pos.z, 0, game->recordAgentModifiedBlocks());
							hasObtainedBlock = true;

						}
						
						break;

					case NEGY:

						if (game->isInWorld(pos.x, pos.y - 1, pos.z)) {

							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x, pos.y - 1, pos.z, 0, game->recordAgentModifiedBlocks());
							hasObtainedBlock = true;

						}
						
						break;

					case PLUSZ:

						if (game->isInWorld(pos.x, pos.y, pos.z + 1)) {

							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x, pos.y, pos.z + 1, 0, game->recordAgentModifiedBlocks());
							hasObtainedBlock = true;

						}
						
						break;

					case NEGZ:

						if (game->isInWorld(pos.x, pos.y, pos.z - 1)) {

							game->selectAIworld(individualID);
							blockObtained = game->setBlock(individualID, pos.x, pos.y, pos.z - 1, 0, game->recordAgentModifiedBlocks());
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

			return result;

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

		playerSpawnPos_.x = 0;
		playerSpawnPos_.y = chunkHeightMap_({ 0, 0 })[0][0] + 10;
		playerSpawnPos_.z = 0;

		AISpawnPos_.x = 0;
		AISpawnPos_.y = chunkHeightMap_({ 0, 0 })[0][0];
		AISpawnPos_.z = 0;

		coalSpreadRange_ = std::uniform_int_distribution<unsigned int>::param_type(1, 8);
		ironSpreadRange_ = std::uniform_int_distribution<unsigned int>::param_type(1, 7);
		goldSpreadRange_ = std::uniform_int_distribution<unsigned int>::param_type(1, 5);
		diamondSpreadRange_ = std::uniform_int_distribution<unsigned int>::param_type(1, 3);

	}


	void miningWorldGen::generate_(VoxelEng::chunk& chunk) {

		VoxelEng::vec3 chunkPos = chunk.chunkPos(),
				       blockPos;
		VoxelEng::vec3 inChunkPos;
		const chunkHeightMap& heightMap = chunkHeightMap_(chunkPos.x, chunkPos.z);

		for (inChunkPos.x = 0; inChunkPos.x < SCX; inChunkPos.x++)
			for (inChunkPos.z = 0; inChunkPos.z < SCZ; inChunkPos.z++)
				for (inChunkPos.y = 0; inChunkPos.y < SCY; inChunkPos.y++) {

					if (!chunk.getBlock(inChunkPos)) {

						blockPos = VoxelEng::chunkManager::getGlobalPos(chunkPos, inChunkPos);

						if (blockPos.y < heightMap[inChunkPos.x][inChunkPos.z] - 3) {

							if (blockPos.y > 30 && floatDice_(generator_) <= 1.05f)
								generateOre_(inChunkPos, chunk, ore::COAL);
							else if (blockPos.y <= 35 && blockPos.y > 10 && floatDice_(generator_) <= 1.05f)
								generateOre_(inChunkPos, chunk, ore::IRON);
							else if (blockPos.y <= 15 && blockPos.y > -10 && floatDice_(generator_) <= 1.01f)
								generateOre_(inChunkPos, chunk, ore::GOLD);
							else if (blockPos.y <= -5 & blockPos.y > -20 && floatDice_(generator_) <= 1.005f)
								generateOre_(inChunkPos, chunk, ore::DIAMOND);
							else
								chunk.setBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, 2);

						}
						else if (blockPos.y >= heightMap[inChunkPos.x][inChunkPos.z] - 3 && blockPos.y < heightMap[inChunkPos.x][inChunkPos.z])
							chunk.setBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, 6);
						else if (blockPos.y == heightMap[inChunkPos.x][inChunkPos.z])
							chunk.setBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, 1);

					}

				}

		chunk.setLoadLevel(VoxelEng::chunkLoadLevel::DECORATED);

	}

	const chunkHeightMap& miningWorldGen::chunkHeightMap_(int chunkX, int chunkZ) {

		VoxelEng::vec2 chunkXZPos(chunkX, chunkZ);

		if (chunkColHeight_.find(chunkXZPos) == chunkColHeight_.cend())
			generateChunkHeightMap_(chunkXZPos);

		return chunkColHeight_[chunkXZPos];
	
	}

	void miningWorldGen::generateChunkHeightMap_(const VoxelEng::vec2& chunkXZPos) {

		chunkHeightMap& heights = chunkColHeight_[chunkXZPos] = std::array<std::array<int, SCZ>, SCX>();
		float softnessFactor = 64.0f,
			  height;
		VoxelEng::vec2 pos,
					   aux;
		VoxelEng::vec3 perlinCoords;
		for (pos[0] = 0u; pos[0] < SCX; pos[0]++) // x
			for (pos[1] = 0u; pos[1] < SCZ; pos[1]++) { // z

				// Height here is between -1.0 and 1.0.
				#if GRAPHICS_API == OPENGL

					aux = VoxelEng::chunkManager::getXZGlobalPos(chunkXZPos, pos) / softnessFactor;

					perlinCoords[0] = aux[0];
					perlinCoords[1] = aux[1];
					perlinCoords[2] = seed_;

					height = glm::perlin(perlinCoords);

				#else



				#endif
			
				// Make height value between 0.0 and 200.0.
				heights[pos[0]][pos[1]] = VoxelEng::translateRange(height, -1.0f, 1.0f, 0.0f, 200.0f);

			}
	
	}

	void miningWorldGen::generateOre_(VoxelEng::vec3 inChunkPos, VoxelEng::chunk& chunk, ore ore) {

		std::uniform_int_distribution<unsigned int>::param_type* oreSpread = nullptr;
		VoxelEng::block oreID = 0;

		switch (ore) {

		case ore::COAL:

			oreSpread = &coalSpreadRange_;
			oreID = 7;

			break;

		case ore::IRON:

			oreSpread = &ironSpreadRange_;
			oreID = 8;

			break;

		case ore::GOLD:

			oreSpread = &goldSpreadRange_;
			oreID = 9;

			break;

		case ore::DIAMOND:

			oreSpread = &diamondSpreadRange_;
			oreID = 10;

			break;

		}

		unsigned int nBlocks = intDice_(generator_, *oreSpread);
		VoxelEng::vec3 cPos;
		for (unsigned int i = 0; i < nBlocks; i++) {

			if (true) {

				chunk.setBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, oreID);

				switch (int6Dice_(generator_)) {

				case 1:

					if (inChunkPos.x + 1 >= SCX) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedNorth))
							cascadeOreGen_(cPos, i, nBlocks, 0, inChunkPos.y, inChunkPos.z, oreID);
						
						i = nBlocks;

					}
					else
						inChunkPos.x++;

					break;

				case 2:

					if (inChunkPos.x == 0) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedSouth))
							cascadeOreGen_(cPos, i, nBlocks, SCX - 1, inChunkPos.y, inChunkPos.z, oreID);
						
						i = nBlocks;

					}
					else
						inChunkPos.x--;

					break;

				case 3:

					if (inChunkPos.y + 1 >= SCY) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedUp))
							cascadeOreGen_(cPos, i, nBlocks, inChunkPos.x, 0, inChunkPos.z, oreID);
						
						i = nBlocks;

					}
					else
						inChunkPos.y++;

					break;

				case 4:

					if (inChunkPos.y == 0) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedDown))
							cascadeOreGen_(cPos, i, nBlocks, inChunkPos.x, SCY - 1, inChunkPos.z, oreID);
						
						i = nBlocks;

					}
					else
						inChunkPos.y--;

					break;

				case 5:

					if (inChunkPos.z + 1 >= SCZ) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedEast))
							cascadeOreGen_(cPos, i, nBlocks, inChunkPos.x, inChunkPos.y, 0, oreID);

						i = nBlocks;

					}
					else
						inChunkPos.z++;

					break;

				case 6:

					if (inChunkPos.z == 0) {

						if (VoxelEng::chunkManager::isChunkInWorld(cPos = chunk.chunkPos() + VoxelEng::vec3FixedWest))
							cascadeOreGen_(cPos, i, nBlocks, inChunkPos.x, inChunkPos.y, SCZ - 1, oreID);
						else
							i = nBlocks;

					}
					else
						inChunkPos.z--;

					break;

				}

			}

		}

	}

	void miningWorldGen::cascadeOreGen_(const VoxelEng::vec3 chunkPos, unsigned int& nBlocksCounter, unsigned int nBlocks,
		unsigned int inChunkX, unsigned int inChunkY, unsigned int inChunkZ, VoxelEng::block oreID) {

		VoxelEng::chunk* cascadeChunk = nullptr;
		if (VoxelEng::chunkManager::getChunkLoadLevel(chunkPos) == VoxelEng::chunkLoadLevel::NOTLOADED)
			cascadeChunk = VoxelEng::chunkManager::createChunk(true, chunkPos);
		else
			cascadeChunk = VoxelEng::chunkManager::selectChunkByChunkPos(chunkPos);
		unsigned int spreadDirection = 0;
		for (nBlocksCounter; nBlocksCounter < nBlocks; nBlocksCounter++) {

			if (!cascadeChunk->getBlock(inChunkX, inChunkY, inChunkZ)) {

				cascadeChunk->setBlock(inChunkX, inChunkY, inChunkZ, oreID);

				switch (spreadDirection = int6Dice_(generator_)) {

					case 1:

						if (inChunkX + 1 >= SCX)
							nBlocksCounter = nBlocks; // Stop generating because we do not want ore clusters					 
						else						  // that expand to 3 chunks or that form a '<' or '>' shape between 2 chunks.
							inChunkX++;

						break;

					case 2:

						if (inChunkX == 0)
							nBlocksCounter = nBlocks;
						else
							inChunkX--;

							break;

					case 3:

						if (inChunkY + 1 >= SCY)
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

						if (inChunkZ + 1 >= SCZ)
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


	// 'miningAIGame' class.

	void miningAIGame::addScore(unsigned int individualID, float score) {

		if (isAgentRegistered(individualID))
			scores_[individualID] += score;

	}

	bool miningAIGame::loadAgentsData(const std::string& path) {

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

	float miningAIGame::blockScore(VoxelEng::block ID) const {

		if (blockScore_.contains(ID))
			return blockScore_.at(ID);
		else
			VoxelEng::logger::errorLog("Block ID " + std::to_string(ID) + " is not registered to have an score");

	}

	void miningAIGame::generalSetUp_() {
	
		blockScore_ = { {0, -1}, // Block with ID 0 means "unreachable block" in the AI game so it's discouraged that the AI agents try to break the rules and get an unreachable block.
						{1, 0},
						{2, 0},
						{7, 5},
						{8, 10},
						{9, 25},
						{10, 50}}; 

		visionDepth_ = 3;
		visionRadius_ = 5;

		genetic_.setFitnessFunction(miningAIGameFitness);

		if (!VoxelEng::worldGen::isGenRegistered("miningWorldGen")) {

			VoxelEng::worldGen::registerGen<AIExample::miningWorldGen>("miningWorldGen");
			VoxelEng::worldGen::selectGen("miningWorldGen");

		}
		VoxelEng::worldGen::setSeed();
	
	}

	void miningAIGame::displayMenu_() {

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

		unsigned int chosenOption = 0;
		do {

			while (!VoxelEng::validatedCinInput<unsigned int>(chosenOption) || chosenOption == 0 || chosenOption > 8)
				VoxelEng::logger::say("Invalid option. Please try again");

			std::string recordFileName,
						agentDataPath;
			bool repeat,
				 overwrite = false,
				 overwriteRecordName = false,
				 writeAgentPath = true;;
			char response = '\0';
			unsigned int opExitStatus = 0;
			switch (chosenOption) {
			
			case 1:

				setUpTraining_();
				train_();

				break;

			case 2:

				setUpTraining_();

				VoxelEng::logger::say("Type the name of the file that holds the AI agent data.");
				VoxelEng::validatedCinInput<std::string>(agentDataPath);

				while (!trainLoadedAgents_(agentDataPath)) {
				
					VoxelEng::logger::say("File not found. Please type another file name.");
					VoxelEng::validatedCinInput<std::string>(agentDataPath);
				
				}

				break;

			case 3:

				setUpTest_();
				test_();

				break;

			case 4:

				setUpTest_();

				VoxelEng::logger::say("Type the name of the file that holds the AI agent data.");
				VoxelEng::validatedCinInput<std::string>(agentDataPath);

				while (!testLoadedAgents_(agentDataPath)) {

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

					opExitStatus = generateRecord_("records/" + name_ + '/', recordFileName);

					if (opExitStatus == 2) { // 'recordFileName' is invalid.
					
						repeat = true;
						overwrite = false;
					
					}
					else if (opExitStatus == 3) { // There is already a file at "records/" + 'name_' + "/" + 'recordFilename' + ".rec".

						repeat = true;

						VoxelEng::logger::say("Overwrite (Y/N, case insensitive)?");
						
						while (!VoxelEng::validatedCinInput<char>(response) || response != 'Y' || response != 'N' || response != 'y' || response != 'n') {

							VoxelEng::logger::say("Invalid response. Type 'Y' for 'yes' and 'N' for 'no' (case insensitive).");
							VoxelEng::validatedCinInput<char>(response);

						}

						if (response == 'Y' || response == 'y') {
						
							overwrite = true;
							std::filesystem::remove("records/" + name_ + '/' + recordFileName);

						}
						else
							overwrite = false;
							
					}
				
				} while (repeat);

				break;

			case 6:

				do {

					repeat = false;

					if (!overwriteRecordName) {

						VoxelEng::logger::say("Type the record's file name.");
						VoxelEng::validatedCinInput<std::string>(recordFileName);
						overwriteRecordName = true;

					}

					if (writeAgentPath) {

						VoxelEng::logger::say("Type the AI agents' data file path.");
						VoxelEng::validatedCinInput<std::string>(agentDataPath);
						writeAgentPath = false;

					}

					opExitStatus = generateRecordLoadedAgents_("records/" + name_ + '/', recordFileName, agentDataPath);

					if (opExitStatus == 2) {
					
						repeat = true;
						overwriteRecordName = false;

					}
					else if (opExitStatus == 3) { // Error: there is already a file in the place where the record is going to be saved.

						repeat = true;

						VoxelEng::logger::say("Overwrite (Y/N, case insensitive)?");

						while (!VoxelEng::validatedCinInput<char>(response) || response != 'Y' || response != 'N' || response != 'y' || response != 'n') {

							VoxelEng::logger::say("Invalid response. Type 'Y' for 'yes' and 'N' for 'no' (case insensitive).");
							VoxelEng::validatedCinInput<char>(response);

						}

						if (response == 'Y' || response == 'y') {

							overwriteRecordName = true;
							std::filesystem::remove("records/" + name_ + '/' + recordFileName);

						}
						else
							overwriteRecordName = false;

					}
					else if (opExitStatus == 4) { // No file found in 'agentDataPath'.

						repeat = true;
						overwriteRecordName = true; // Do not ask again for record file name.
						writeAgentPath = true;
					
					}

				} while (repeat);

				break;

			case 7:

				VoxelEng::logger::say("Type the record's file name.");
				VoxelEng::validatedCinInput<std::string>(recordFileName);
				while (playRecord_(recordFileName) == 1) {
				
					VoxelEng::logger::say("Record not found. Try again.");
					VoxelEng::validatedCinInput<std::string>(recordFileName);
				
				}

				break;
			
			}

		} while (chosenOption != 8);

	}

	void miningAIGame::setUpTraining_() {
	
		popSize_ = 100;
		scores_ = std::vector<float>(popSize_, 0.0f);
		epochForNewWorld_ = 3;
		genetic_.setCrossoverSplitPoint(3);
		genetic_.setMutationParameters(1.0f/popSize_, 0.5f, 0.5f);

		// Options.
		nInputs_ = visionDepth_ * (unsigned int)pow((2 * visionRadius_), 2) + 2; // + 2 -> height position (position in the y-axis) and the fixed direction (unsigned int) the bot is looking at.
		nEpochs_ = 100;
		nEpochsBetweenSaves_ = 20;
		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		layerSize_ = { nInputs_, 50, 50, 6 };

	}

	void miningAIGame::train_() {

		// Begin training.
		genetic_.genInitPop(popSize_, layerSize_, -5.0f, 5.0f);
		genetic_.train(nEpochs_, nEpochsBetweenSaves_);
	
	}

	void miningAIGame::setUpTest_() {

		popSize_ = 100;
		scores_ = std::vector<float>(popSize_, 0.0f);
		epochForNewWorld_ = 3;
		genetic_.setCrossoverSplitPoint(3);
		genetic_.setMutationParameters(1.0f / popSize_, 0.5f, 0.5f);

		// Options.
		nInputs_ = visionDepth_ * (unsigned int)pow((2 * visionRadius_), 2) + 2; // + 2 -> height position (position in the y-axis) and the fixed direction (unsigned int) the bot is looking at.
		nEpochs_ = 100;
		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		layerSize_ = { nInputs_, 50, 50, 6 };
		
	}

	void miningAIGame::test_() {

		float* averageFitness = nullptr;
		unsigned int averageFitnessSize;


		genetic_.genInitPop(popSize_, layerSize_, -5.0f, 5.0f);
		genetic_.test(nEpochs_, averageFitness, averageFitnessSize);

		for (unsigned int i = 0; i < averageFitnessSize; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(i) + " average fitness is: " + std::to_string(*(averageFitness + i)));

	}

	void miningAIGame::setUpRecord_() {

		popSize_ = 5;
		scores_ = std::vector<float>(popSize_, 0.0f);

		// Options.
		nInputs_ = visionDepth_ * (unsigned int)pow((2 * visionRadius_), 2) + 2, // + 2 -> height position (position in the y-axis) and the fixed direction (unsigned int) the bot is looking at.
		nEpochs_ = 1,
		VoxelEng::logger::debugLog("Number of inputs for neural network: " + std::to_string(nInputs_));
		layerSize_ = { nInputs_, 50, 50, 6 };

	}

	void miningAIGame::record_() {

		genetic_.genInitPop(popSize_, layerSize_, -5.0f, 5.0f);
		genetic_.test(nEpochs_);
	
	}
	
	void miningAIGame::spawnAgents_(unsigned int nAgents) {

		try {
		
			const VoxelEng::vec3& spawnPos = dynamic_cast<miningWorldGen&>(VoxelEng::worldGen::selectedGen()).spawnPos();

			if (AIagentID_.empty())
				for (unsigned int i = 0; i < nAgents; i++) {

					createAgent(agentsModelID_, spawnPos);
					scores_[i] = 0;

				}
			else
				for (unsigned int i = 0; i < nAgents; i++)
					scores_[i] = 0;

		}
		catch (const std::bad_cast e) {
		
			VoxelEng::logger::errorLog("Selected world generator is not a 'miningWorldGen' object");
		
		}

	}

	bool miningAIGame::trainLoadedAgents_(const std::string& path) {

		if (!loadAgentsData_(path))
			return false;

		genetic_.train(nEpochs_, nEpochsBetweenSaves_);

		return true;

	}

	bool miningAIGame::testLoadedAgents_(const std::string& path) {

		unsigned int averageFitnessSize;
		float* averageFitness = nullptr;

		if (!loadAgentsData_(path))
			return false;

		genetic_.test(nEpochs_, averageFitness, averageFitnessSize);

		for (unsigned int i = 0; i < averageFitnessSize; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(i) + " average fitness is: " + std::to_string(*(averageFitness + i)));

		return true;

	}


	bool miningAIGame::recordLoadedAgents_(const std::string& path) {

		if (!loadAgentsData(path))
			return false;

		genetic_.test(nEpochs_);

		return true;

	}
	
}