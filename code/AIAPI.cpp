#include "AIAPI.h"
#include <filesystem>
#include <ios>
#include "utilities.h"
#include "gui.h"


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

		agentActionArg::agentActionArg(blockViewDir value)
		: tag(type::BLOCKVIEWDIR), bvd(value)
		{}


		// 'aiGame' class.

		// protected
		std::atomic<bool> aiGame::recording_ = false,
						  aiGame::recordAgentModifiedBlocks_ = false,
						  aiGame::gameInProgress_ = false;
		std::ofstream aiGame::saveFile_;
		std::string aiGame::saveDataBuffer_;
		std::string aiGame::saveFileName_;
		recordPlayMode aiGame::recordPlayMode_ = recordPlayMode::FORWARD;
		std::vector<std::deque<block>> aiGame::agentModifiedBlocks_;
		std::list<unsigned int> aiGame::entityIDcreationOrder_;
		std::list<unsigned int> aiGame::agentIDcreationOrder_;

		// private
		bool aiGame::initialised_ = false,
			 aiGame::canForwardReplay_ = true,
			 aiGame::canBackwardReplay_ = false,
			 aiGame::backwardAIactionFound_ = true;
		aiGame* aiGame::selectedGame_ = nullptr;
		std::unordered_map<std::string, aiGame*> aiGame::aiGames_;
		std::vector<aiGame*> aiGame::gamesRegisterOrder_;
		std::vector<AIagentAction> aiGame::aiRecordActions_;
		std::unordered_map<std::string, unsigned int> aiGame::AIactionsName_;
		std::ifstream aiGame::loadedFile_;
		std::string aiGame::readFileData_,
					aiGame::readWord_;
		char aiGame::readCharacter_;
		unsigned int aiGame::readActionCode_ = 0,
					 aiGame::readParamTypeInd_ = 0,
					 aiGame::readState_ = 0,
					 aiGame::lastParamInd_ = 0,
			         aiGame::lastRawParamInd_ = 0;
		double aiGame::oldActualTime_ = 0;
		std::vector<agentActionArg> aiGame::params_;
		std::deque<std::string> aiGame::rawParams_;


		void aiGame::init() {
		
			if (initialised_)
				logger::errorLog("AI game system already initialised");
			else {

				recording_ = false;
				canForwardReplay_ = true;
				canBackwardReplay_ = false;
				recordPlayMode_ = recordPlayMode::FORWARD;
				recordAgentModifiedBlocks_ = false;
				gameInProgress_ = false;

				registerAction("setBlock", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					if (game->playingRecordForward()) {

						if (!game->recordAgentModifiedBlocks())
							logger::errorLog("The recording of agents' terrain modification is disabled");

						VoxelEng::logger::debugLog("Agent removes block on " + std::to_string(game->getParam<int>(1)) + '|' + std::to_string(game->getParam<int>(2)) + '|' + std::to_string(game->getParam<int>(3)));
						game->setBlock(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getParam<block>(4), true);

					}
					else {

						agentID agentID = game->getParam<unsigned int>(0);
						game->setBlock(agentID, game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getLastModifiedBlock(agentID, true), false);
						VoxelEng::logger::debugLog("Agent places block back in " + std::to_string(game->getParam<int>(1)) + '|' + std::to_string(game->getParam<int>(2)) + '|' + std::to_string(game->getParam<int>(3)));

					}

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::BLOCK }));

				registerAction("moveEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Moving entity " + std::to_string(game->getParam<int>(1)) + '|' + std::to_string(game->getParam<int>(2)) + '|' + std::to_string(game->getParam<int>(3)));
						game->moveEntity(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3));
					
					}
					else {
					
						VoxelEng::logger::debugLog("Moving entity " + std::to_string(-game->getParam<int>(1)) + '|' + std::to_string(-game->getParam<int>(2)) + '|' + std::to_string(-game->getParam<int>(3)));
						game->moveEntity(game->getParam<unsigned int>(0), -game->getParam<int>(1), -game->getParam<int>(2), -game->getParam<int>(3));
					
					}

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT }));

				registerAction("rotateAgentViewDir", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Rotating view dir " + std::to_string(game->getParam<int>(1)));
						game->rotateAgentViewDir(game->getParam<unsigned int>(0), game->getParam<blockViewDir>(1));
					
					}
					else {
					
						VoxelEng::logger::debugLog("Inversely Rotating view dir " + std::to_string(game->getParam<int>(1)));
						game->rotateAgentViewDir(game->getParam<unsigned int>(0), inverseUDirection(game->getParam<blockViewDir>(1)));
					
					}

				}, {agentActionArg::type::UINT, agentActionArg::type::BLOCKVIEWDIR }));

				registerAction("rotateEntity", AIagentAction([]() {
				
					aiGame* game = aiGame::selectedGame();
					
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Rotating entity model");
						game->rotateEntity(game->getParam<unsigned int>(0), game->getParam<float>(1), game->getParam<float>(2), game->getParam<float>(3));
					
					}
					else {
					
						VoxelEng::logger::debugLog("Inversely rotating entity model");
						game->inverseRotateEntity(game->getParam<unsigned int>(0), -game->getParam<float>(1), -game->getParam<float>(2), -game->getParam<float>(3));
					
					}
						
				}, {agentActionArg::type::UINT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("inverseRotateEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Inversely rotating entity model");
						game->inverseRotateEntity(game->getParam<unsigned int>(0), game->getParam<float>(1), game->getParam<float>(2), game->getParam<float>(3));

					}
					else {
					
						VoxelEng::logger::debugLog("Rotating entity model");
						game->rotateEntity(game->getParam<unsigned int>(0), -game->getParam<float>(1), -game->getParam<float>(2), -game->getParam<float>(3));
					
					}	

				}, {agentActionArg::type::UINT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("createEntity", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					
					if (game->playingRecordForward()) {

						VoxelEng::logger::debugLog("Creating entity");
						unsigned int ID = game->createEntity(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3),
																game->getParam<float>(4), game->getParam<float>(5), game->getParam<float>(6));
						entityIDcreationOrder_.push_back(ID);

					}
					else {

						VoxelEng::logger::debugLog("Deleting entity");
						game->deleteEntity(entityIDcreationOrder_.back());
						entityIDcreationOrder_.pop_back();

					}

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT,
					agentActionArg::type::FLOAT, agentActionArg::type::FLOAT, agentActionArg::type::FLOAT}));

				registerAction("createAgent", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Creating AI agent");
						agentID agentID = game->createAgent(game->getParam<unsigned int>(0), game->getParam<int>(1), game->getParam<int>(2), game->getParam<int>(3), game->getParam<blockViewDir>(4));
						agentIDcreationOrder_.push_back(agentID);
					
					}
					else {
					
						VoxelEng::logger::debugLog("Deleting AI agent");
						game->deleteAgent(agentIDcreationOrder_.back());
						agentIDcreationOrder_.pop_back();
					
					}
						

				}, {agentActionArg::type::UINT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::INT, agentActionArg::type::BLOCKVIEWDIR}));

				registerAction("changeActiveState", AIagentAction([]() {

					aiGame* game = aiGame::selectedGame();
					
					if (game->playingRecordForward()) {
					
						VoxelEng::logger::debugLog("Changing entity's active state");
						game->changeEntityActiveState(game->getParam<unsigned int>(0), game->getParam<bool>(1));
					
					}
					else {
					
						VoxelEng::logger::debugLog("Changing entity's active state (reverse)");
						game->changeEntityActiveState(game->getParam<unsigned int>(0), !game->getParam<bool>(1));
					
					}
						

				}, {agentActionArg::type::UINT, agentActionArg::type::BOOL}));

			}

		}

		aiGame* aiGame::selectedGame() {

			if (selectedGame_)
				return selectedGame_;
			else
				logger::errorLog("No AI game is currently loaded");

		}

		bool aiGame::entityIsAgent(entityID entityID) const {

			if (entityManager::isEntityRegistered(entityID))
				return entityIDIsAgent.find(entityID) != entityIDIsAgent.cend();
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not registered");

		}

		unsigned int aiGame::getAgentEntityID(agentID agentID) const {
		
			if (isAgentRegistered(agentID))
				return AIagentEntityID_[agentID];
			else
				logger::errorLog("There is no registered agent with agent ID " + std::to_string(agentID));
		
		}

		unsigned int aiGame::getEntityAgentID(entityID entityID) const {
		
			if (entityManager::isEntityRegistered(entityID)) {
			
				auto it = entityIDIsAgent.find(entityID);
				if (it != entityIDIsAgent.cend())
					return *it;
				else
					logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not an AI agent");
			
			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not registered");
		
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

		template <>
		blockViewDir aiGame::getParam<blockViewDir>(unsigned int index) const {

			if (index < lastParamInd_)
				return params_[index].bvd;
			else
				VoxelEng::logger::errorLog("Invalid index " + std::to_string(index) + " for an AI agent action parameter");

		}

		bool aiGame::hasModifiedLevel(agentID agentID) const {

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
					game::setAImode(true);
				
					selectedGame_->generalSetUp_();
					selectedGame_->displayMenu_();

					game::setAImode(false);
					selectedGame_->cleanUpGame_();
					gameInProgress_ = false;

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

		void aiGame::changeRecordPlayMode(recordPlayMode mode) {
		
			if (mode != recordPlayMode_) {
			
				GUIelement& element = GUImanager::getGUIElement("pauseIcon");
				element.lockMutex();
				switch (mode) {

				case recordPlayMode::FORWARD:
					element.changeTextureID(1021);
					break;

				case recordPlayMode::PAUSE:
					element.changeTextureID(1022);
					break;

				case recordPlayMode::BACKWARDS:
					element.changeTextureID(1023);
					break;

				default:
					logger::errorLog("Unsupported current record play mode");
					break;

				}
				element.unlockMutex();
				recordPlayMode_ = mode;
			
			}
		
		}

		void aiGame::stopPlayingRecord() {

			if (playingRecord()) {
			
				selectedGame_->clearBlockModRecording();
				loadedFile_.close();

				game::setLoopSelection(engineMode::EXITRECORD);
				game::setLoopSelection(engineMode::AIMENULOOP);
			
			}

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
			
				recording_ = true;
				if (path.empty()) {
				
					logger::say("Recording's file path is empty");
					recording_ = false;
					return 1;
				
				}
				else {
				
					if (path.back() != '/') {
					
						logger::say("The last character of the recording's file path is not \'/\'");
						recording_ = false;
						return 1;
					
					}	
					else {
					
						if (filename.empty()) {
						
							logger::say("Recording's file name is empty");
							recording_ = false;
							return 2;
						
						}	
						else {

							if (isalnum(filename)) {

								std::string recordingPath = path + filename + ".rec";

								if (std::filesystem::exists(recordingPath)) {
								
									logger::say("There is already a file named " + recordingPath);
									recording_ = false;
									return 3;
								
								}
								else {

									recording_ = true;
									saveFile_.open(recordingPath);

									// Initialise chunk manager system earlier in order to set the number of chunks to compute.
									if (chunkManager::initialised())
										chunkManager::setNChunksToCompute(DEF_N_CHUNKS_TO_COMPUTE);
									else
										chunkManager::init(DEF_N_CHUNKS_TO_COMPUTE);

									saveDataBuffer_ += selectedGame_->name_ + '|';
									saveDataBuffer_ += "saves/recordingWorlds/" + selectedGame_->name_ + '/' + filename + '|';
									saveDataBuffer_ += std::to_string(chunkManager::nChunksToCompute()) + "|@";

									saveFileName_ = filename;

									selectedGame_->setUpRecord_(1);
									selectedGame_->record_();

									saveFile_ << saveDataBuffer_;
									saveDataBuffer_.clear();
									saveFile_.close();
									recording_ = false;

									return 0;

								}
								
							}
							else {
							
								logger::say("Recording's file name contains no alphanumeric characters");
								recording_ = false;
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

					game::setLoopSelection(engineMode::INITRECORD);
					recordPlayMode_ = recordPlayMode::FORWARD;

					loadedFile_.open(truePath);

					bool readFirstLines = false;
					while (!readFirstLines) {

						if (loadedFile_.get(readCharacter_)) {

							if (readCharacter_ == '|') { // End of action code/action parameter.

								switch (readState_) {

								case 0: // End of aiGame line.

									if (readWord_ != selectedGame_->name_)
										logger::errorLog("The loaded recording file belongs to the AI game " + readWord_ + 
											". It does not belong to " + selectedGame_->name_);

									readState_++;
									break;

								case 1: // End of terrain line.

									chunkManager::openedTerrainFileName(readWord_);

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

					selectedGame_->initBlockModRecording();

					// Initialise required engine systems.
					if (!VoxelEng::chunkManager::initialised()) {

						VoxelEng::chunkManager::init(VoxelEng::DEF_N_CHUNKS_TO_COMPUTE);
						VoxelEng::entityManager::init();

					}

					// Initialisation of the elements that are used in aiGame::playRecordTick().
					canForwardReplay_ = true;
					canBackwardReplay_ = false;
					GUImanager::changeGUIState("mainMenu", false);
					GUImanager::changeGUIState("mainMenu.saveButton", false);
					GUImanager::changeGUIState("mainMenu.newButton", false);
					GUImanager::changeGUIState("mainMenu.loadButton", false);
					GUImanager::changeGUIState("mainMenu.exitButton", false);
					GUImanager::bindActKeyFunction("mainMenu", nullptr, controlCode::noKey);

					game::gameLoop(chunkManager::openedTerrainFileName());

					stopPlayingRecord();
					oldActualTime_ = 0;

					selectedGame_->cleanUpMatch_();

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

		void aiGame::playRecordTick() {
		
			if (game::selectedEngineMode() == engineMode::PLAYINGRECORD) {

				if (oldActualTime_ == 0)
					oldActualTime_ = time::actualTime<timeScale::s>();

				if (recordPlayMode_ != recordPlayMode::PAUSE && time::actualTime<timeScale::s>() - oldActualTime_ >= 0.5f) {

					// Starting from the last played action (or the beginning of the record file)...
					bool continuePlaying = true;
					if (recordPlayMode_ == recordPlayMode::FORWARD) { // Play the next action forward.

						if (canForwardReplay_) {
						
							bool readingActionCode = true;
							do {

								readCharacter_ = loadedFile_.peek();

								if (readCharacter_ == EOF) { // End of AI action lines reached. Do not advance and return.

									changeRecordPlayMode(recordPlayMode::PAUSE);
									canForwardReplay_ = false;
									canBackwardReplay_ = true;
									continuePlaying = false;
									loadedFile_.seekg(-1, std::ios::cur);
									backwardAIactionFound_ = true;

								}
								else {

									if (readCharacter_ == '#') { // New action found.

										if (!readingActionCode) { // Finished reading the parameters of a previously found action. Execute said action and return. 

											playActionRecordForward_(readActionCode_);
											continuePlaying = false;

											canBackwardReplay_ = true; // Now the engine can play backwards at least one action.
											backwardAIactionFound_ = false;

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
									else if (readCharacter_ != '@')
										readWord_ += readCharacter_;

									if (continuePlaying)
										loadedFile_.seekg(1, std::ios::cur);

								}

							} while (continuePlaying && game::threadsExecute[1]);
						
						}
						else
							changeRecordPlayMode(recordPlayMode::PAUSE);

						oldActualTime_ = time::actualTime<timeScale::s>();

						return;

					}
					else {
					
						if (canBackwardReplay_) { // Play the previous action backwards.

							bool readingAction = false;
							do {

								readCharacter_ = loadedFile_.peek();

								if (readCharacter_ == '#') { // New action found.

									if (backwardAIactionFound_) {

										// "Clear" last parsed parameters. Reuse already allocated memory.
										readParamTypeInd_ = 0;
										lastParamInd_ = 0;

										readActionCode_ = sto<unsigned int>(readWord_);
										pushParams_(readActionCode_);
										playActionRecordBackwards_(readActionCode_);
										continuePlaying = false;
										readWord_ = "";

										rawParams_.clear(); // Clear last parsed parameters.

										canForwardReplay_ = true; // Now the engine can play at least one action in forward mode again.

										backwardAIactionFound_ = false;

									}
									else
										backwardAIactionFound_ = true;

								}
								else if (readCharacter_ == '|') { // End of action code/action parameter.

									if (readingAction) {

										// We are reading the parameters in reverse order so we need to use push_front().
										rawParams_.push_front(std::string(readWord_.rbegin(), readWord_.rend()));
										readWord_ = "";

									}
									else
										readingAction = true;

								}
								else if (readCharacter_ == '@') {

									changeRecordPlayMode(recordPlayMode::PAUSE);
									canForwardReplay_ = true;
									canBackwardReplay_ = false;
									continuePlaying = false;

								}
								else if (readCharacter_ != EOF)
									readWord_ += readCharacter_;

								if (continuePlaying) // As we need to continue playing, go back one position in the recording file.
									loadedFile_.seekg(-1, std::ios::cur);

							} while (continuePlaying && game::threadsExecute[1]);

						}
						else
							changeRecordPlayMode(recordPlayMode::PAUSE);

						oldActualTime_ = time::actualTime<timeScale::s>();

						return;

					}
				
				}

				return;

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

				if (AIactionsName_.find(actionName) != AIactionsName_.cend()) {

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

							case agentActionArg::type::BLOCKVIEWDIR:

								saveDataBuffer_ += std::to_string(static_cast<unsigned int>(arg.bvd)) + "|";

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
							params_[lastParamInd_].ui = sto<unsigned int>(param);
						else
							params_.emplace_back(sto<unsigned int>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::FLOAT:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].f = sto<float>(param);
						else
							params_.emplace_back(sto<float>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::CHAR:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].c = sto<char>(param);
						else
							params_.emplace_back(sto<char>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::BOOL:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].b = sto<bool>(param);
						else
							params_.emplace_back(sto<bool>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::BLOCK:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].bl = sto<block>(param);
						else
							params_.emplace_back(sto<block>(param));

						lastParamInd_++;

						break;

					case agentActionArg::type::BLOCKVIEWDIR:
						if (lastParamInd_ < params_.size())
							params_[lastParamInd_].bvd = sto<blockViewDir>(param);
						else
							params_.emplace_back(sto<blockViewDir>(param));

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
								params_[lastParamInd_].ui = sto<unsigned int>(rawParams_[i]);
							else
								params_.emplace_back(sto<unsigned int>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::FLOAT:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].f = sto<float>(rawParams_[i]);
							else
								params_.emplace_back(sto<float>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::CHAR:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].c = sto<char>(rawParams_[i]);
							else
								params_.emplace_back(sto<char>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::BOOL:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].b = sto<bool>(rawParams_[i]);
							else
								params_.emplace_back(sto<bool>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::BLOCK:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].bl = sto<block>(rawParams_[i]);
							else
								params_.emplace_back(sto<block>(rawParams_[i]));

							lastParamInd_++;

							break;

						case agentActionArg::type::BLOCKVIEWDIR:
							if (lastParamInd_ < params_.size())
								params_[lastParamInd_].bvd = sto<blockViewDir>(rawParams_[i]);
							else
								params_.emplace_back(sto<blockViewDir>(rawParams_[i]));

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
			agentModifiedBlocks_.clear();
			entityIDcreationOrder_.clear();
			agentIDcreationOrder_.clear();

			initialised_ = false;
		
		}

		block aiGame::getLastModifiedBlock(agentID agentID, bool popBlock) {
		
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

		block aiGame::getFirstModifiedBlock(agentID agentID, bool popBlock) {
		
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

		block aiGame::getModifiedBlock(agentID agentID, unsigned int blockIndex) {
		
			if (isAgentRegistered(agentID)) {

				if (recordAgentModifiedBlocks_) {

					auto& modifiedBlocks = agentModifiedBlocks_[agentID];

					if (modifiedBlocks.empty())
						logger::errorLog("The AI agent has not made any modifications to the level's terrain");
					else {

						if (blockIndex < modifiedBlocks.size())
							return modifiedBlocks[blockIndex];
						else
							logger::errorLog("Specified block index is invalid");

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

		void aiGame::selectAIworld(agentID agentID) {

			if (isAgentRegistered(agentID))
				chunkManager::selectAIworld(agentID);
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		blockViewDir aiGame::getBlockViewDir(agentID agentID) {

			if (isAgentRegistered(agentID))
				return AIagentLookDirection_[agentID];
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		block aiGame::setBlock(agentID agentID, int x, int y, int z, VoxelEng::block blockID, bool record) {

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

		void aiGame::moveEntity(entityID entityID, int x, int y, int z) {

			entityManager::moveEntity(entityID, x, y, z);

			if (recording_)
				recordAction("moveEntity", { entityID, x, y, z });

		}

		void aiGame::setEntityPos(unsigned entityID, int x, int y, int z) {

			if (entityManager::isEntityRegistered(entityID)) {

				if (recording_)
					recordAction("setEntityPos", { entityID, x, y, z });


				vec3& pos = entityManager::getEntity(entityID).pos();
				pos.x = x;
				pos.y = y;
				pos.z = z;

			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::rotateAgentViewDir(agentID agentID, blockViewDir direction) {

			if (isAgentRegistered(agentID)) {

				if (direction != blockViewDir::PLUSX && direction != blockViewDir::NEGX && direction != blockViewDir::PLUSY && direction != blockViewDir::NEGY)
					logger::errorLog("Rotation direction specified is not valid");
				else {

					if (recording_)
						recordAction("rotateAgentViewDir", { agentID, direction });

					entityManager::getEntity(AIagentEntityID_[agentID]).rotateView(uDirectionToVec3(direction) * 90.0f);

					AIagentLookDirection_[agentID] = rotateUDirection(AIagentLookDirection_[agentID], direction);

				}

			}
			else 
				logger::errorLog("AI agent with ID " + std::to_string(AIagentEntityID_[agentID]) + " was not found");

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

		unsigned int aiGame::createAgent(unsigned int entityTypeID, int x, int y, int z, blockViewDir direction) {

			unsigned int ID = entityManager::registerEntity(entityTypeID, x, y, z, uDirectionToVec3(direction));

			if (recording_)
				recordAction("createAgent", {entityTypeID, x, y, z, direction});

			entityIDIsAgent.insert(ID);
			if (freeAIagentID_.empty()) {

				AIagentEntityID_.push_back(ID);
				AIagentLookDirection_.push_back(direction);

				if (game::selectedEngineMode() == engineMode::PLAYINGRECORD && ID >= agentModifiedBlocks_.size())
					agentModifiedBlocks_.emplace_back();

				return AIagentEntityID_.size() - 1;

			}
			else {

				agentID agentID = *freeAIagentID_.begin();
				AIagentEntityID_[agentID] = ID;
				AIagentLookDirection_[agentID] = direction;

				freeAIagentID_.erase(agentID);

				return agentID;

			}

		}

		vec3 aiGame::getEntityPos(entityID entityID) {

			if (entityManager::isEntityRegistered(entityID))
				return entityManager::getEntity(entityID).pos();
			else
				logger::errorLog("AI agent with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::changeEntityActiveState(entityID entityID, bool state) {

			if (entityManager::isEntityRegistered(entityID)) {
			
				if (recording_)
					recordAction("changeActiveState", {entityID, state});

				entityManager::changeEntityActiveStateAt(entityID, state);
			
			}	
			else
				logger::errorLog("AI agent with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::deleteEntity(entityID entityID) {

			if (entityManager::isEntityRegistered(entityID)) {
			
				if (recording_)
					recordAction("deleteEntity", {entityID});


				entityManager::deleteEntity(entityID);
			
			}
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

		}

		void aiGame::deleteAgent(agentID agentID) {

			if (isAgentRegistered(agentID)) {
			
				if (recording_)
					recordAction("deleteEntity", {agentID});


				entityID entityID = AIagentEntityID_[agentID];

				entityManager::deleteEntity(entityID);
				freeAIagentID_.insert(agentID);
				entityIDIsAgent.erase(entityID);
			
			}
			else
				logger::errorLog("AI agent with ID " + std::to_string(agentID) + " was not found");

		}

		void aiGame::playActionRecordForward_(unsigned int actionCode) {
			
			if (actionRegistered_(actionCode))
				aiRecordActions_[actionCode].playRecordedAction();
			else
				logger::errorLog("There is no such action to play with the specified action code");
		
		}

		void aiGame::playActionRecordBackwards_(unsigned int actionCode) {

			if (actionRegistered_(actionCode))
				aiRecordActions_[actionCode].playRecordedAction();
			else
				logger::errorLog("There is no such action to play with the specified action code");
		
		}

		void aiGame::initBlockModRecording() {
		
			recordAgentModifiedBlocks_ = true;
			agentModifiedBlocks_.resize(AIagentEntityID_.size());
		
		}

		void aiGame::clearBlockModRecording() {
		
			recordAgentModifiedBlocks_ = false;
			agentModifiedBlocks_.clear();
			entityIDcreationOrder_.clear();
			agentIDcreationOrder_.clear();
		
		}

		void aiGame::generateAIWorld(const std::string& path) {

			if (!chunkManager::initialised())
				logger::errorLog("Chunk management system is not initialised");

			chunkManager::generateAIWorld(path);

		}


		// 'trainingGame' class.

		unsigned int trainingGame::generateRecordLoadedAgents_(const std::string& recordPath, const std::string& recordFilename,
															   const std::string& agentsPath) {

			if (recording_)
				logger::errorLog("A recording is already being made");
			else {

				recording_ = true;
				if (recordPath.empty()) {
				
					logger::say("Recording's file path is empty");
					recording_ = false;
					return 2;
				
				}	
				else {

					if (recordPath.back() != '/') {
					
						logger::say("The last character of the recording's file path is not \'/\'");
						recording_ = false;
						return 2;
					
					}
					else {

						if (recordFilename.empty()) {
						
							logger::say("Recording's file name is empty");
							recording_ = false;
							return 2;
						
						}
						else {

							if (isalnum(recordFilename)) {

								std::string recordingPath = recordPath + recordFilename + ".rec";

								if (std::filesystem::exists(recordingPath)) {

									recording_ = false;
									return 3;

								}	
								else {

									saveFile_.open(recordingPath);

									saveDataBuffer_ += name_ + '|';
									saveDataBuffer_ += "saves/recordingWorlds/" + name_ + '/' + recordFilename + '|';
									saveDataBuffer_ += std::to_string(chunkManager::nChunksToCompute()) + "|@";

									saveFileName_ = recordFilename;

									if (!recordLoadedAgents_(agentsPath)) {
									
										logger::say("No file located at " + agentsPath);

										saveFile_.close();
										saveDataBuffer_.clear();
										std::filesystem::remove(recordingPath);

										recording_ = false;
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
								recording_ = false;
								return 2;
							
							}
							
						}

					}

				}

			}

		}

	}

}