#include "AIAPI.h"
#include <filesystem>
#include <ios>
#include "utilities.h"
#include "game.h"


namespace VoxelEng {

	namespace AIAPI {

		// 'AIagentAction' class.

		AIagentAction::AIagentAction(void(*action)(), std::initializer_list<agentActionArg::type> paramTypes)
			: action_(action), paramTypes_(paramTypes)
		{}

		agentActionArg::type AIagentAction::paramType(unsigned int index) const {

			if (index < paramTypes_.size())
				return paramTypes_[index];
			else
				logger::errorOutOfRange("Parameter type index " + std::to_string(index));

		}


		// 'agentActionArg' class.

		agentActionArg::agentActionArg(int value)
		: tag (type::INT), i(value)
		{}

		agentActionArg::agentActionArg(unsigned int value)
		: tag(type::UINT), ui(value)
		{}

		agentActionArg::agentActionArg(float value)
		: tag(type::FLOAT), f(value)
		{}

		agentActionArg::agentActionArg(char value)
		: tag(type::CHAR), c(value)
		{}

		agentActionArg::agentActionArg(bool value)
		: tag(type::BOOL), b(value)
		{}

		agentActionArg::agentActionArg(block value)
		: tag(type::BLOCK), bl(value)
		{}


		// 'aiGame' class.

		std::atomic<bool>aiGame::recording_ = false;
		std::ofstream aiGame::saveFile_;
		std::string aiGame::saveDataBuffer_;

		bool aiGame::initialised_ = false;
		aiGame* aiGame::selectedGame_ = nullptr;
		std::atomic<bool> aiGame::gameInProgress_ = false,
						  aiGame::playingRecord_ = false;
		std::unordered_map<std::string, aiGame*> aiGame::aiGames_;
		std::vector<aiGame*> aiGame::gamesRegisterOrder_;
		std::vector<AIagentAction> aiGame::aiRecordActions_;
		std::unordered_map<std::string, unsigned int> aiGame::AIactionsName_;
		std::string aiGame::playbackTerrainFile_;
		std::ifstream aiGame::loadedFile_;
		std::string aiGame::readFileData_,
					aiGame::readWord_;
		char aiGame::readCharacter_;
		unsigned int aiGame::nFileCharsRead_ = 0,
					 aiGame::readActionCode_ = 0,
					 aiGame::readParamTypeInd_ = 0,
					 aiGame::readState_ = 0,
					 aiGame::lastParamInd_ = 0,
					 aiGame::recordingPlayerEntity_ = 0;
		std::vector<agentActionArg> aiGame::params_;
		std::deque<std::string> aiGame::rawParams_;


		void aiGame::init() {
		
			if (initialised_)
				logger::errorLog("AI game system already initialised");
			else {
			
				registerAction("blockViewDir", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					game->blockViewDir(game->getParam<unsigned int>(0));

				}, {agentActionArg::type::UINT}));

				registerAction("setBlock", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward()) {

						if (!game->recordAgentModifiedBlocks())
							logger::errorLog("The recording of agents' terrain modification is disabled");

						game->setBlock(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getParam<block>(4), true);

					}
					else {

						unsigned int agentID = game->getParam<unsigned int>(0);
						game->setBlock(agentID, game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getLastModifiedBlock(agentID, true), false);

					}

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::BLOCK }));

				registerAction("getBlocksBox", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					game->getBlocksBox(game->getParam<int>(0), game->getParam<int>(1), game->getParam<int>(2),
									  game->getParam<int>(3), game->getParam<int>(4), game->getParam<int>(5));

				}, {agentActionArg::type::INT, agentActionArg::type::INT,  agentActionArg::type::INT,
					agentActionArg::type::INT,  agentActionArg::type::INT,  agentActionArg::type::INT }));

				registerAction("moveEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->moveEntity(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3));
					else
						game->moveEntity(game->getParam<unsigned int>(0), -game->getParam<int>(1), -game->getParam<int>(2), -game->getParam<int>(3));

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT }));

				registerAction("rotateAgentViewDir", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->rotateAgentViewDir(game->getParam<unsigned int>(0), game->getParam<unsigned int>(1));
					else
						game->rotateAgentViewDir(game->getParam<unsigned int>(0), inverseUDirection(game->getParam<unsigned int>(1)));

				}, {agentActionArg::type::UINT, agentActionArg::type::UINT }));

				registerAction("rotateEntity", AIagentAction([]() {
				
					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->rotateEntity(game->getParam<unsigned int>(0), game->getParam<float>(1), game->getParam<float>(2), game->getParam<float>(3));
					else
						game->inverseRotateEntity(game->getParam<unsigned int>(0), -game->getParam<float>(1), -game->getParam<float>(2), -game->getParam<float>(3));
				
				}, {agentActionArg::type::UINT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("inverseRotateEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->inverseRotateEntity(game->getParam<unsigned int>(0), game->getParam<float>(1), game->getParam<float>(2), game->getParam<float>(3));
					else
						game->rotateEntity(game->getParam<unsigned int>(0), -game->getParam<float>(1), -game->getParam<float>(2), -game->getParam<float>(3));

				}, {agentActionArg::type::UINT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("createEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->createEntity(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3),
										  game->getParam<float>(4), game->getParam<float>(5), game->getParam<float>(6));
					else
						game->deleteEntity(game->getParam<unsigned int>(0));

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT,
					agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("createAgent", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->createAgent(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getParam<unsigned int>(4));
					else
						game->deleteAgent(game->getParam<unsigned int>(0));

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::UINT}));

				registerAction("getEntityPos", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					game->getEntityPos(game->getParam<unsigned int>(0));

				}, {agentActionArg::type::UINT}));

				registerAction("changeActiveState", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();

					if (game->playingRecordForward())
						game->changeEntityActiveState(game->getParam<unsigned int>(0), game->getParam<bool>(1));
					else
						game->changeEntityActiveState(game->getParam<unsigned int>(0), !game->getParam<bool>(1));

				}, {agentActionArg::type::UINT, agentActionArg::type::BOOL}));

			}

		}

		aiGame* aiGame::selectedGame() {

			if (selectedGame_)
				return selectedGame_;
			else
				logger::errorLog("No AI game is currently loaded");

		}

		bool aiGame::entityIsAgent(unsigned int entityID) const {

			if (entityManager::isEntityRegistered(entityID))
				return activeEntityID_.find(entityID) != activeEntityID_.cend();
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

		}

		template <>
		int aiGame::getParam<int>(unsigned int index) const {
		
			if (index < lastParamInd_)
				return params_[index].i;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");
		
		}

		template <>
		unsigned int aiGame::getParam<unsigned int>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].ui;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		template <>
		float aiGame::getParam<float>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].f;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		template <>
		char aiGame::getParam<char>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].c;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		template <>
		bool aiGame::getParam<bool>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].b;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		template <>
		block aiGame::getParam<block>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].bl;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		bool aiGame::hasModifiedLevel(unsigned int agentID) const {

			if (isAgentRegistered(agentID))
				return !agentModifiedBlocks_[agentID].empty();
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		void aiGame::selectGame(const std::string& gameName) {

			if (gameInProgress_)
				logger::errorLog("Cannot change AI game while the current one is currently in progress");
			else {
			
				if (aiGames_.find(gameName) == aiGames_.cend())
					logger::errorLog("There is no registered AI game with the name " + gameName);
				else
					selectedGame_ = aiGames_[gameName];
			
			}

		}

		void aiGame::selectGame(unsigned int index) {

			if (gameInProgress_)
				logger::errorLog("Cannot change AI game while the current one is currently in progress");
			else {

				std::size_t size = gamesRegisterOrder_.size();

				if (index >= size)
					logger::errorLog("There are only " + std::to_string(size) + " registered games and the game number " + std::to_string(index + 1) + " was specified");
				else
					selectedGame_ = gamesRegisterOrder_[index];

			}

		}

		void aiGame::startGame() {

			if (gameInProgress_)
				logger::errorLog("Cannot start a new AI game without finishing the one that is currently in progress");
			else {
			
				if (selectedGame_) {

					gameInProgress_ = true;
					selectedGame_->generalSetUp_();
					selectedGame_->displayMenu_();

				}
				else
					logger::errorLog("There is no selected AI game to start");

			}

		}

		void aiGame::finishGame() {

			if (gameInProgress_) {
			
				gameInProgress_ = false;
			
			}
			else
				logger::errorLog("There is no AI game in progress to finish");

		}

		unsigned int aiGame::listAIGames() {

			unsigned int i = 1;
			for (auto it = aiGames_.cbegin(); it != aiGames_.cend(); it++)
				logger::say(std::to_string(i) + "). " + it->first);

			return i;
		
		}

		unsigned int aiGame::generateRecord_(const std::string& path, const std::string& filename) {
		
			if (recording_)
				logger::errorLog("A recording is already being made");
			else {
			
				if (path.empty()) {
				
					logger::say("Recording's file path is empty");
					return 1;
				
				}
				else {
				
					if (path.back() != '/') {
					
						logger::say("The last character of the recording's file path is not \'/\'");
						return 1;
					
					}	
					else {
					
						if (filename.empty()) {
						
							logger::say("Recording's file name is empty");
							return 2;
						
						}	
						else {

							if (isalnum(filename)) {

								std::string recordingPath = path + filename + ".rec";

								if (std::filesystem::exists(recordingPath)) {
								
									logger::say("There is already a file named " + recordingPath);
									return 3;
								
								}
								else {

									recording_ = true;
									saveFile_.open(recordingPath);

									saveDataBuffer_ += name_ + '|';
									saveDataBuffer_ += "saves/recordings/" + filename + "Recording.terrain|";
									saveDataBuffer_ += std::to_string(chunkManager::nChunksToCompute()) + '|';

									setUpRecord_();
									record_();

									saveFile_ << saveDataBuffer_;
									saveDataBuffer_.clear();
									saveFile_.close();
									recording_ = false;

									return 0;

								}
								
							}
							else {
							
								logger::say("Recording's file name contains no alphanumeric characters");
								return 2;
							
							}

						}
					
					}

				}
			
			}

		}

		unsigned int aiGame::playRecord_(const std::string& path) {
		
			if (path.find('|') == std::string::npos) {

				std::string truePath = path + ".rec";
				if (std::filesystem::exists(truePath)) {

					loadedFile_.open(truePath);

					bool readFirstLines = false;
					while (!readFirstLines) {

						if (readCharacter_ = loadedFile_.get()) {

							if (readCharacter_ == '|') { // End of action code/action parameter.

								switch (readState_) {

								case 0: // End of aiGame line.

									if (readWord_ != name_)
										logger::errorLog("The loaded recording file belongs to the AI game " + readWord_ + ". It does not belong to " + name_);

									readState_++;
									break;

								case 1: // End of terrain line.

									playbackTerrainFile_ = readWord_;

									readState_++;
									break;

								case 2: // End of nChunksToCompute line.

									chunkManager::setNChunksToCompute(sto<unsigned int>(readWord_));

									readFirstLines = true;
									break;

								}

								readWord_ = "";

							}
							else
								readWord_ += readCharacter_;

						}
						else
							logger::errorLog("Could not read first lines from record file " + truePath);

					}
					readState_ = 0;

					initBlockModRecording();

					// Prepare variables that are used in aiGame::playRecordTick_().
					nFileCharsRead_ = 0; // Use this as and Beginning Of File indicator and use seekg with negative numbers and std::ios::end, std::ios::current to start with
					playingRecord_ = true;

					// Execute engine's graphical mode and play the record with the specified level.
					game::gameLoop(true, playbackTerrainFile_);

					clearBlockModRecording();
					loadedFile_.close();

					return 0;

				}
				else {

					logger::say("Recording file " + truePath + " not found.");
					return 1;

				}

			}
			else {
			
				logger::say("Recording file path may not contain the character \'|\'");
				return 1;
			
			}

		}

		bool aiGame::playRecordTick() {
		
			if (playingRecord_) {

				// Starting from the last played action (or the beginning of the record file)...

				bool continuePlaying = true;
				if (playingRecordForward_) { // Play the next action forward.
				
					bool readingActionCode = true;
					do {

						readCharacter_ = loadedFile_.peek();

						if (readCharacter_ == EOF) // End of AI action lines reached. Do not advance and return.
							return true;
						else {

							if (readCharacter_ == '#') { // New action found.

								if (!readingActionCode) { // Finished reading the parameters of a previously found action. Execute said action and return. 

									playActionRecordForward_(readActionCode_);
									continuePlaying = false;

								}

								readingActionCode = true;
								readParamTypeInd_ = 0;
								lastParamInd_ = 0;

							}
							else if (readCharacter_ == '|') { // End of action code/action parameter.

								if (readingActionCode) { // End of action code.

									readActionCode_ = sto<unsigned int>(readWord_);

									readingActionCode = false;

								}
								else // End of action parameter.
									pushParam_(readWord_, readActionCode_, readParamTypeInd_++);

								readWord_ = "";

							}
							else
								readWord_ += readCharacter_;

							if (continuePlaying) {
							
								loadedFile_.seekg(1, std::ios::cur);
								nFileCharsRead_++;
							
							}
							
						}

					} while (continuePlaying);
				
				}
				else { // Play the previous action backwards.
				
					nFileCharsRead_ = 0;
					bool readActionEnd = false,
						 readingAction = false;
					do {

						readCharacter_ = loadedFile_.peek();

						if (nFileCharsRead_ == 0) // Beginning of file reached. Do not go back and return.
							return true;
						else {

							if (readCharacter_ == '#') { // New action found.

								if (readActionEnd) {

									readActionCode_ = sto<unsigned int>(readWord_);
									pushParams_(readActionCode_);
									playActionRecordBackwards_(readActionCode_);
									continuePlaying = false;
									readWord_ = "";
									rawParams_.clear(); // The parsed parameters are no longer needed.

								}
								else
									readActionEnd = true;

							}
							else if (readCharacter_ == '|') { // End of action code/action parameter.

								if (readingAction) {

									rawParams_.push_front(readWord_); // We are reading the parameters in reverse order so we need to use push_front().
									readWord_ = "";

								}
								else
									readingAction = true;

							}
							else
								readWord_ += readCharacter_;

							// As we are not at beginning of file, go back one position in the recording file.
							loadedFile_.seekg(-1, std::ios::cur);
							nFileCharsRead_--;

						}

					} while (continuePlaying);
				
				}

				return false;

			}
			else
				logger::errorLog("Can only execute aiGame::playRecordTick() when playing a record");

		}

		void aiGame::registerAction(const std::string& actionName, const AIagentAction& action) {
		
			if (AIactionsName_.find(actionName) == AIactionsName_.cend()) {

				AIactionsName_[actionName] = aiRecordActions_.size();
				aiRecordActions_.push_back(action);

			}
			else
				logger::errorLog("Action name is already registered");
		
		}

		void aiGame::recordAction(const std::string& actionName, std::initializer_list<agentActionArg> args) {

			if (recording_) {

				if (AIactionsName_.find(actionName) == AIactionsName_.cend()) {

					saveDataBuffer_ += "#" + std::to_string(AIactionsName_[actionName]) + "|";

					for (auto it = args.begin(); it != args.end(); it++) {
					
						const agentActionArg& arg = *it;

						switch (arg.tag) {

							case agentActionArg::type::INT:

								saveDataBuffer_ += std::to_string(arg.i) + "|";

								break;

							case agentActionArg::type::UINT:

								saveDataBuffer_ += std::to_string(arg.ui) + "|";

								break;

							case agentActionArg::type::FLOAT:

								saveDataBuffer_ += std::to_string(arg.f) + "|";

								break;

							case agentActionArg::type::CHAR:

								saveDataBuffer_ += std::to_string(arg.c) + "|";

								break;

							case agentActionArg::type::BOOL:

								saveDataBuffer_ += std::to_string(arg.b) + "|";

								break;

							case agentActionArg::type::BLOCK:

								saveDataBuffer_ += std::to_string(arg.bl) + "|";

								break;

						}
					
					}

				}
				else
					logger::errorLog("No registered AI agent action named " + actionName + " was found");

			}
			else
				logger::errorLog("Recording mode is disabled so no AI agent action can be recorded");
		}

		void aiGame::pushParam_(const std::string& param, unsigned int actionCode, unsigned int paramTypeInd) {

			if (actionRegistered_(actionCode)) {

				switch (aiRecordActions_[actionCode].paramType(paramTypeInd)) {
					
					// Parse parameter and push it into 'params_' to make it available for the corresponding
					// action that is going to be executed.
					case agentActionArg::type::INT:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<int>(param);
						else 
							params_.emplace_back(sto<int>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::UINT:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<unsigned int>(param);
						else
							params_.emplace_back(sto<unsigned int>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::FLOAT:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<float>(param);
						else
							params_.emplace_back(sto<float>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::CHAR:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<char>(param);
						else
							params_.emplace_back(sto<char>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::BOOL:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<bool>(param);
						else
							params_.emplace_back(sto<bool>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::BLOCK:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].i = sto<block>(param);
						else
							params_.emplace_back(sto<bool>(param));

						lastParamInd_++;

						break;
					
				}

			}
			else
				logger::errorLog("AI agent action with code " + std::to_string(actionCode) + " is not registered");

		}

		void aiGame::pushParams_(unsigned int actionCode) {

			if (actionRegistered_(actionCode)) {

				for (std::size_t i = 0; i < rawParams_.size(); i++) {

					switch (aiRecordActions_[actionCode].paramType(i)) {

						// Parse parameter and push it into 'params_' to make it available for the corresponding
						// action that is going to be executed.
						case agentActionArg::type::INT:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<int>(rawParams_[i]);
							else
								params_.emplace_back(sto<int>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::UINT:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<unsigned int>(rawParams_[i]);
							else
								params_.emplace_back(sto<unsigned int>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::FLOAT:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<float>(rawParams_[i]);
							else
								params_.emplace_back(sto<float>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::CHAR:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<char>(rawParams_[i]);
							else
								params_.emplace_back(sto<char>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::BOOL:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<bool>(rawParams_[i]);
							else
								params_.emplace_back(sto<bool>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::BLOCK:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].i = sto<block>(rawParams_[i]);
							else
								params_.emplace_back(sto<bool>(rawParams_[i]));

							lastParamInd_++;

							break;

					}

				}

			}
			else
				logger::errorLog("AI agent action with code " + std::to_string(actionCode) + " is not registered");

		}

		void aiGame::cleanUp() {
		
			selectedGame_ = nullptr;

			for (auto it = aiGames_.cbegin(); it != aiGames_.cend(); it++)
				delete it->second;

			aiGames_.clear();
			gamesRegisterOrder_.clear();

			initialised_ = false;
		
		}

		block aiGame::getLastModifiedBlock(unsigned int agentID, bool popBlock) {
		
			if (isAgentRegistered(agentID)) {

				if (recordAgentModifiedBlocks_) {

					if (agentModifiedBlocks_[agentID].empty())
						logger::errorLog("The AI agent has not made any modifications to the level's terrain");
					else {
					
						block b = agentModifiedBlocks_[agentID].back();

						if (popBlock)
							agentModifiedBlocks_[agentID].pop_back();

						return b;
					
					}

				}
				else
					logger::errorLog("AI agents' level's terrain modifications are not recorded");
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");
		
		}

		block aiGame::getFirstModifiedBlock(unsigned int agentID, bool popBlock) {
		
			if (isAgentRegistered(agentID)) {

				if (recordAgentModifiedBlocks_) {

					if (agentModifiedBlocks_[agentID].empty())
						logger::errorLog("The AI agent has not made any modifications to the level's terrain");
					else {

						block b = agentModifiedBlocks_[agentID].front();

						if (popBlock)
							agentModifiedBlocks_[agentID].pop_front();

						return b;

					}

				}
				else
					logger::errorLog("AI agents' level's terrain modifications are not recorded");

			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");
		
		}

		block aiGame::getModifiedBlock(unsigned int agentID, unsigned int blockIndex) {
		
			if (isAgentRegistered(agentID)) {

				if (recordAgentModifiedBlocks_) {

					if (agentModifiedBlocks_[agentID].empty())
						logger::errorLog("The AI agent has not made any modifications to the level's terrain");
					else {

						block b = agentModifiedBlocks_[agentID][blockIndex];

						return b;

					}

				}
				else
					logger::errorLog("AI agents' level's terrain modifications are not recorded");

			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");
		
		}

		bool aiGame::isInWorld(int x, int y, int z) {

			return chunkManager::isInWorld(x, y, z);

		}

		void aiGame::selectAIworld(unsigned int agentID) {

			if (isAgentRegistered(agentID))
				chunkManager::selectAIworld(agentID);
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		unsigned int aiGame::blockViewDir(unsigned int agentID) {

			if (isAgentRegistered(agentID)) {
			
				if (recording_)
					recordAction("blockViewDir", {agentID});


				return AIagentLookDirection_[agentID];
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		block aiGame::setBlock(unsigned int agentID, int x, int y, int z, VoxelEng::block blockID, bool record) {

			if (isAgentRegistered(agentID)) {
			
				if (recording_)
					recordAction("setBlock", {agentID, x, y, z, blockID});


				if (recordAgentModifiedBlocks_ && record)
					agentModifiedBlocks_[agentID].push_back(chunkManager::getBlock(x, y, z));

				return chunkManager::setBlock(x, y, z, blockID);
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");
		}

		std::vector<block> aiGame::getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2) {

			if (recording_)
				recordAction("getBlocksBox", { x1, y1, z1, x2, y2, z2 });


			return chunkManager::getBlocksBox(x1, y1, z1, x2, y2, z2);

		}

		void aiGame::moveEntity(unsigned int entityID, int x, int y, int z) {

			if (entityManager::isEntityRegistered(entityID)){

				if (recording_)
					recordAction("moveEntity", {entityID, x, y, z});


				vec3& pos = entityManager::getEntity(entityID).pos();
				pos.x += x;
				pos.y += y;
				pos.z += z;

			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::rotateAgentViewDir(unsigned int agentID, unsigned int direction) {

			if (isAgentRegistered(agentID))
				logger::errorLog("AI agent with ID " + std::to_string(AIagentID_[agentID]) + " was not found");
			else {
			
				if (direction != PLUSX && direction != NEGX && direction != PLUSY && direction != NEGY)
					logger::errorLog("Rotation direction specified is not valid");
				else {
				
					if (recording_)
						recordAction("rotateAgentViewDir", { agentID, direction});


					vec3 rot = uDirectionToVec3(direction) * 90.0f;

					entity& agent = entityManager::getEntity(AIagentID_[agentID]);

					agent.rotate(rot);

					logger::debugLog("Rotating view from: " + std::to_string(AIagentLookDirection_[agentID]));

					AIagentLookDirection_[agentID] = rotateUDirection(AIagentLookDirection_[agentID], direction);

					logger::debugLog("to " + std::to_string(AIagentLookDirection_[agentID]) + " using " + std::to_string(direction));
				
				}

			}

		}

		void aiGame::rotateEntity(unsigned entityID, float rotX, float rotY, float rotZ) {
		
			if (entityManager::isEntityRegistered(entityID)) {

				if (recording_)
					recordAction("rotateEntity", {entityID, rotX, rotY, rotZ});


				entityManager::getEntity(entityID).rotate(rotX, rotY, rotZ);

			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");
		
		}

		void aiGame::inverseRotateEntity(unsigned entityID, float rotX, float rotY, float rotZ) {

			if (entityManager::isEntityRegistered(entityID)) {

				if (recording_)
					recordAction("inverseRotateEntity", { entityID, rotX, rotY, rotZ });


				entity& entity = entityManager::getEntity(entityID);
				entity.rotateZ(rotZ);
				entity.rotateY(rotY);
				entity.rotateX(rotX);

			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");
		
		}

		unsigned int aiGame::createEntity(unsigned int entityTypeID, int posX, int posY, int posZ, float rotX, float rotY, float rotZ) {

			if (recording_)
				recordAction("createEntity", { entityTypeID, posX, posY, posZ, rotX, rotY, rotZ });

			return entityManager::registerEntity(entityTypeID, posX, posY, posZ, rotX, rotY, rotZ);

		}

		unsigned int aiGame::createAgent(unsigned int entityTypeID, int x, int y, int z, unsigned int direction) {

			unsigned int ID = entityManager::registerEntity(entityTypeID, x, y, z, uDirectionToVec3(direction) * 90.0f);

			
			if (recording_)
				recordAction("createAgent", {entityTypeID, x, y, z, direction});


			activeEntityID_.insert(ID);
			if (freeAIAgentID_.empty()) {

				AIagentID_.push_back(ID);
				AIagentLookDirection_.push_back(direction);

			}
			else {

				unsigned int agentID = *freeAIAgentID_.begin();
				AIagentID_[agentID] = ID;
				AIagentLookDirection_[agentID] = direction;

				freeAIAgentID_.erase(agentID);

			}

			return ID;

		}

		vec3 aiGame::getEntityPos(unsigned int entityID) {

			if (entityManager::isEntityRegistered(entityID)) {
			
				if (recording_)
					recordAction("getEntityPos", {entityID});

				return entityManager::getEntity(entityID).pos();
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::changeEntityActiveState(unsigned int entityID, bool state) {

			if (entityManager::isEntityRegistered(entityID)) {
			
				if (recording_)
					recordAction("changeActiveState", {entityID, state});

				entityManager::changeEntityActiveStateAt(entityID, state);
			
			}	
			else
				logger::errorLog("AI agent with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::deleteEntity(unsigned int entityID) {

			if (entityManager::isEntityRegistered(entityID)) {
			
				if (recording_)
					recordAction("deleteEntity", {entityID});


				entityManager::deleteEntity(entityID);
			
			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::deleteAgent(unsigned int agentID) {

			if (isAgentRegistered(agentID)) {
			
				if (recording_)
					recordAction("deleteEntity", {agentID});


				unsigned int entityID = AIagentID_[agentID];

				entityManager::deleteEntity(entityID);
				freeAIAgentID_.insert(agentID);
				activeEntityID_.erase(entityID);
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		void aiGame::playActionRecordForward_(unsigned int actionCode) {
		
			playingRecordForward_ = true;
			
			if (actionRegistered_(actionCode))
				aiRecordActions_[actionCode].playRecordedAction();
			else
				logger::errorLog("There is no such action to play with the specified action code");
		
		}

		void aiGame::playActionRecordBackwards_(unsigned int actionCode) {
		
			playingRecordForward_ = false;

			if (actionRegistered_(actionCode))
				aiRecordActions_[actionCode].playRecordedAction();
			else
				logger::errorLog("There is no such action to play with the specified action code");
		
		}

		void aiGame::initBlockModRecording() {
		
			recordAgentModifiedBlocks_ = true;
			agentModifiedBlocks_.resize(AIagentID_.size());
		
		}

		void aiGame::clearBlockModRecording() {
		
			recordAgentModifiedBlocks_ = false;
			agentModifiedBlocks_.clear();
		
		}


		// 'traininGame' class.

		unsigned int trainingGame::generateRecordLoadedAgents_(const std::string& recordPath, const std::string& recordFilename,
															   const std::string& agentsPath) {

			if (recording_)
				logger::errorLog("A recording is already being made");
			else {

				if (recordPath.empty()) {
				
					logger::say("Recording's file path is empty");
					return 1;
				
				}	
				else {

					if (recordPath.back() != '/') {
					
						logger::say("The last character of the recording's file path is not \'/\'");
						return 1;
					
					}
					else {

						if (recordFilename.empty()) {
						
							logger::say("Recording's file name is empty");
							return 2;
						
						}
						else {

							if (isalnum(recordFilename)) {

								std::string recordingPath = recordPath + recordFilename + ".rec";

								if (std::filesystem::exists(recordingPath))
									return 3;
								else {

									recording_ = true;
									saveFile_.open(recordingPath);

									saveDataBuffer_ += name_ + "|";
									saveDataBuffer_ += "saves / recordings / " + recordFilename + "Recording.terrain | ";

									setUpRecord_();
									if (!recordLoadedAgents_(agentsPath)) {
									
										logger::say("No file located at " + agentsPath);
										return 4;
									
									}	

									saveFile_ << saveDataBuffer_;
									saveDataBuffer_.clear();
									saveFile_.close();
									recording_ = false;
								
									return 0;

								}
								
							}
							else {
							
								logger::say("Recording's file name contains no alphanumeric characters");
								return 2;
							
							}
							
						}

					}

				}

			}

			return 2;

		}

	}

}