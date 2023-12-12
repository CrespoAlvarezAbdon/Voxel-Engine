/**
* @file AIAPI.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title AI API.
* @brief Contains everything related to AI agents and AI games.
*/

#ifndef _VOXELENG_AIAPI_
#define _VOXELENG_AIAPI_
#include <atomic>
#include <cstddef>
#include <concepts>
#include <deque>
#include <initializer_list>
#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include "chunk.h"
#include "definitions.h"
#include "entity.h"
#include "game.h"
#include "logger.h"
#include "time.h"
#include "worldGen.h"


namespace VoxelEng {

	namespace AIAPI {

		/////////////////////////
		//Forward declarations.//
		/////////////////////////

		class trainingGame;


		/////////////////
		//Enum classes.//
		/////////////////

		/**
		* @brief The available modes of playing an AI game match record.
		*/
		enum class recordPlayMode {FORWARD, PAUSE, BACKWARDS};


		////////////
		//Classes.//
		////////////

		/**
		* @brief Represents the type of one of the possible arguments that
		* an AI action can have.
		*/
		class agentActionArg {

		public:

			/*
			Attributes.
			*/

			/**
			* @brief The supported types that an AI action arguments can belong to.
			*/
			enum class type { INT, UINT, FLOAT, CHAR, BOOL, NUMERIC_BLOCK_ID, BLOCKVIEWDIR } tag;

			union {

				int i;
				unsigned int ui;
				float f;
				char c;
				bool b;
				numericShortID nbi;
				blockViewDir bvd;

			};


			/*
			Methods.
			*/

			// Constructors.

			/**
			* @brief Class constructor.
			*/
			agentActionArg(int value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(unsigned int value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(float value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(char value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(bool value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(numericShortID value);

			/**
			* @brief Class constructor.
			*/
			agentActionArg(blockViewDir value);


		private:

			const unsigned int unionSize_ = 6;

		};


		/**
		* @brief An AI agent action is what an AI agent can perform when in an AI game match in its level.
		* It may affect the level, the AI agent itself, other entities or agents or other things
		* defined in the AI game.
		*/
		class AIagentAction {

		public:

			// Constructors.

			/**
			* @brief Provide the created AI action with its function and argument types.
			* 'action' should be a pointer to a void function that needs an 'aiGame' object as parameter
			* and that it calls to aiGame::recordAction(actionstring) on said object
			*/
			AIagentAction(void(*action)(), std::initializer_list<agentActionArg::type> paramTypes);


			// Observers.

			/**
			* @brief Get the type of one of the AI action's parameters.
			*/
			agentActionArg::type paramType(unsigned int index) const;


			// Misc.

			/**
			* @brief Execute the AI action with the parameters that have been read from 
			* the recording file previously.
			*/
			void playRecordedAction();


		private:

			void(*action_)();
			const std::vector<agentActionArg::type> paramTypes_;

		};

		inline void AIagentAction::playRecordedAction() {

			action_();

		}


		/**
		* @brief Manages all the general aspects of AI games and all the aspects of AI games 
		* that do not require training.
		*/
		class aiGame {

		public:

			// Initialization.

			/**
			* @brief Initialise the management system.
			* Allocate any resources that are needed on initialisation.
			*/
			static void init();


			// Constructors.

			/**
			* @brief WARNING: calling aiGame's constructor or the constructor of any class that derives
			* from aiGame results in undefined behaviour. Use aiGame::registerGame to create
			* a new AI game object from any derived class. This constructor was made public because of
			* the requirements of some used C++'s STL data structures.
			*/
			aiGame();


			// Observers: general.

			/**
			* @brief Returns the currently loaded and selected AI game.
			*/
			static aiGame* selectedGame();

			/**
			* @brief Returns the name of the last specified recording file (even if it is being written at the moment).
			* Does not include the path or the extension of said file.
			*/
			static const std::string& recordFilename();

			/**
			* @brief Returns true if the engine is currently recording an AI game match and false if otherwise.
			*/
			static bool recording();

			/**
			* @brief Returns true if the engine is currently playing an AI game match record and false if otherwise.
			*/
			static bool playingRecord();

			/**
			* @brief Returns true if the engine is currently playing an AI game match record
			in forward mode and false if otherwise.
			*/
			static bool playingRecordForward();

			/**
			* @brief Returns true if the AI game match's record playback is paused and false if otherwise.
			*/
			static bool recordPaused();

			/**
			* @brief Name used to identify the AI game uniquely.
			*/
			const std::string& name() const;

			/**
			* @brief Returns true if the specified AI agent exists or false otherwise.
			*/
			bool isAgentRegistered(agentID agentID) const;

			/**
			* @brief Returns true if the specified entity is also an AI agent or false otherwise.
			*/
			bool entityIsAgent(entityID entityID) const;

			/**
			* @brief Get the AI agent's entity ID.
			*/
			unsigned int getAgentEntityID(agentID agentID) const;

			/**
			* @brief Get the entity's AI agent ID (assuming said entity is an AI agent, otherwise
			* an exception will be thrown).
			*/
			unsigned int getEntityAgentID(entityID entityID) const;


			// Observers: Agent actions API. 

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <typename T>
			T getParam(unsigned int index) const = delete;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			int getParam<int>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			unsigned int getParam<unsigned int>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			float getParam<float>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			char getParam<char>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			bool getParam<bool>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			numericShortID getParam<numericShortID>(unsigned int index) const;

			/**
			* @brief Get the value of one of the parameters read from the current
			* AI action that is going to be played.
			*/
			template <>
			blockViewDir getParam<blockViewDir>(unsigned int index) const;

			/**
			* @brief Returns true if the AI agent has made modifications to its copy of the
			* original level that is being used in the AI game or false otherwise.
			*/
			bool hasModifiedLevel(agentID agentID) const;

			/**
			* @brief Returns true if the recording of the AI agents' modifications
			* of the level's terrain is enabled or false otherwise.
			*/
			bool recordAgentModifiedBlocks() const;

			/**
			* @brief Returns the corresponding internal ID used to represent the specified block in the game.
			*/
			numericShortID getInternalID(const std::string& block) const;

			/**
			* @brief Returns the corresponding internal ID used to represent the specified block in the game.
			*/
			numericShortID getInternalID(const VoxelEng::block& block) const;

			/**
			* @brief Returns the corresponding namespaced ID to the block with the specified internal ID.
			*/
			const std::string& getNamespacedID(short internalID) const;


			// Modifiers: general.

			/**
			* @brief Register an instance of a 'aiGame' object (or an object from a class that derives from 'aiGame')
			* to use it when generating new chunks in levels.
			* 'T' is the class that either is 'aiGame' or a class that derives from 'aiGame'.
			*/
			template <class T>
			requires std::derived_from<T, aiGame>
			static void registerGame(const std::string& gameName);

			/**
			* @brief Select one registered AI game to load and mark as the
			* currently selected AI game.
			*/
			static void selectGame(const std::string& gameName);

			/**
			* @brief Select the 'index'º registered AI game.
			* 'index' begins at 0 and the AI games are sorted
			* in the order they were registered.
			*/
			static void selectGame(unsigned int index);

			/**
			* @brief Starts the currently loaded and selected AI game.
			* WARNING. There must be an AI game previously loaded using aiGame::selectGame().
			*/
			static void startGame();

			/**
			* @brief Finishes the currently loaded, selected and started AI game.
			WARNING. There must be an AI game previously loaded using aiGame::selectGame() and
			it must have been started previosuly using aiGame::startGame().
			*/
			static void finishGame();

			/**
			* @brief Display all registered AI games in the standard output
		    * for selection. Returns the number of registered AI games.
			*/
			static unsigned int listAIGames();

			/*
			* @brief Change between the different record playing modes.
			*/
			static void changeRecordPlayMode(recordPlayMode mode);

			/*
			* @brief Stops the current record playback.
			*/
			static void stopPlayingRecord();

			/*
			* @brief Generate the level to be used when using AI agents without
			* the graphical part of the engine.
			*/
			void generateAIWorld(const std::string& path = "");

			/**
			* @brief Get block and set block operations in the chunk manager system will now
			* be performed on the AI world/level of AI agent with ID 'individualID'.
			* AI mode must be turned on in the chunk manager system.
			* WARNING. This method is not thread safe.
			*/
			void selectAIworld(unsigned int individualID);

			/**
			* @brief ONLY get block operations in the chunk manager system will now
			* be performed on the original copy of the level that is being used for the AI game.
			* AI mode must be turned on in the chunk manager system.
			* Set block operations will use the latest AI world selected (the one corresponding to
			* the AI agent with ID 0 by default).
			* WARNING. This method is not thread safe.
			*/
			void selectOriginalWorld();


			// Modifiers: Agent actions' API.

			/**
			* @brief Register an AI action with its unique name to identify it.
			*/
			static void registerAction(const std::string& actionName, const AIagentAction& action);

			/**
			* @brief Check if a block coordinate is withing the level's limits.
			*/
			static bool isInWorld(const VoxelEng::vec3& pos);

			/**
			* @brief Check if a block coordinate is withing the level's limits.
			*/
			static bool isInWorld(int x, int y, int z);

			/**
			* @brief Record an AI action performed by an AI agent.
			* WARNING. The recording flag must be set to true before recording an action.
			*/
			void recordAction(const std::string& actionName, std::initializer_list<agentActionArg> args);

			/**
			* @brief Get the last modified block ID by the specified AI agent. The caller may also
			* optionally pop that ID out of the ordered list of the modified block IDs by said agent.
			*/
			const block& getLastModifiedBlock(agentID agentID, bool popBlock = false);
			
			/**
			* @brief Get the first modified block ID by the specified AI agent. The caller may also
			* optionally pop that ID out of the ordered list of the modified block IDs by said agent.
			*/
			const block& getFirstModifiedBlock(agentID agentID, bool popBlock = false);

			/**
			* @brief Begin the recording of the AI agents' modifications done to the level's terrain.
			*/
			void initBlockModRecording();

			/**
			* @brief Stop the recording of the AI agents' modifications done to the level's terrain and
			* clear the modification lists of all the registered agents.
			*/
			void clearBlockModRecording();

			/*
			@brief Same as "aiGame::getLastModifiedBlock" or "aiGame::getFirstModifiedBlock", but
			with access to any entry from the list by using their numerical indices.
			This method cannot pop out a modified block entry from the agent's record because
			it would render said record invalid as it would no longer contain a
			sequence of instantly followed block modifications.
			*/
			const block& getModifiedBlock(entityID entityID, unsigned int blockIndex);

			/**
			* @brief Get the Block View Direction (BVD) of the specified AI agent.
			*/
			blockViewDir getBlockViewDir(agentID agentID);


			// Modifiers: AI agent actions.
			//
			// Each AI agent action must be encoded with an unique unsigned integer to
			// be able to save AI matches' records in files.
			// In order to implement new agent actions for a AI game derived from the API classes,
			// use aiGame::nAPIActionsInRecord() as the starting code for your new actions.
			// If your AI game is deriving from a AI game class not from the official API classes,
			// then you should use baseAIgameClass::actionCodeForDerived() as all classes
			// that derive from aiGame must implement them.
			// Every AI agent action method implemented must be a static method inside the class
			// representing your AI game and it's fist parameter must always we the object
			// that represents the AI game instance.
			// WARNING. If an implemented AI agent action behaves differently when playing it backwards at 
			// least the first parameter must be the same when playing said action forward and backwards.
			// Otherwise the program's execution falls on undefined behaviour.

			/**
			* @brief Performs a modification of a terrain block and associates that modification to
			* a specified entity.
			* Block ID 0 equals no block or empty/null block.
			* Returns the old ID of the modified block.
			* WARNING. Select the proper level you want to access with "aiGame::selectAIWorld()" or "aiGame::selectOriginalWorld()"
			* before calling this method.
			*/
			const block& setBlock(entityID entityID, const vec3& pos, const block& block, bool record);

			/**
			* @brief Performs a modification of a terrain block and associates that modification to
			* a specified entity.
			* Block ID 0 equals no block or empty/null block.
			* Returns the old ID of the modified block.
			* WARNING. Select the proper level you want to access with "aiGame::selectAIWorld()" or "aiGame::selectOriginalWorld()"
			* before calling this method.
			*/
			const block& setBlock(entityID entityID, int x, int y, int z, const block& block, bool record);

			/**
			* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2.
			*/
			std::vector<const block*> getBlocksBox(const vec3& pos1, const vec3& pos2);

			/**
			* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2.
			*/
			std::vector<const block*> getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2);

			/**
			* @brief Performs agent.pos() += movement;
			*/
			void moveEntity(entityID entityID, const vec3& movement);

			/**
			* @brief Performs agent.pos() += vec3(x, y, z);
			*/
			void moveEntity(entityID entityID, float x, float y, float z);

			/**
			* @brief Performs agent.pos() = pos;
			*/
			void setEntityPos(unsigned entityID, const vec3& pos);

			/**
			* @brief Performs agent.pos() = vec3(x, y, z);
			*/
			void setEntityPos(unsigned entityID, float x, float y, float z);

			/**
			* @brief Rotate the AI agent's viewing direction and rotate it's model
			* to represent graphically where it is now looking at.
			*/
			void rotateAgentViewDir(agentID agentID, blockViewDir direction);

			/**
			* @brief Rotates an entity.
			* AI agents' view direction cannot be rotated with this method, use "aiGame::rotateAgentViewDir()" instead.
			*/
			void rotateEntity(entityID entityID, const vec3& rot);

			/**
			* @brief Rotates an entity.
			* AI agents' view direction cannot be rotated with this method, use "aiGame::rotateAgentViewDir()" instead.
			*/
			void rotateEntity(unsigned entityID, float rotX, float rotY, float rotZ);

			/**
			* @brief Inversely rotates an entity.
			* AI agents' view direction cannot be rotated with this method, use "aiGame::rotateAgentViewDir()" instead.
			*/
			void inverseRotateEntity(unsigned entityID, const vec3& rot);

			/**
			* @brief Inversely rotates an entity.
			* AI agents' view direction cannot be rotated with this method, use "aiGame::rotateAgentViewDir()" instead.
			*/
			void inverseRotateEntity(unsigned entityID, float rotX, float rotY, float rotZ);

			/**
			* @brief Create an entity with its model determined by "entityTypeID".
			*/
			unsigned int createEntity(unsigned int entityTypeID, const vec3& pos, const vec3& rot);

			/**
			* @brief Create an entity with its model determined by "entityTypeID".
			*/
			unsigned int createEntity(unsigned int entityTypeID, float posX, float posY, float posZ, float rotX, float rotY, float rotZ);

			/**
			* @brief Create an AI agent with its model determined by "entityTypeID".
			* This only creates the representation of said AI agent in the system and in graphical form, without the AI implementation part.
			*/
			unsigned int createAgent(unsigned int entityTypeID, const vec3& pos, blockViewDir faceViewDir = blockViewDir::PLUSX);

			/**
			* @brief Create an AI agent with its model determined by "entityTypeID".
			* This only creates the representation of said AI agent in the system and in graphical form, without the AI implementation part.
			*/
			unsigned int createAgent(unsigned int entityTypeID, float x, float y, float z, blockViewDir faceViewDir = blockViewDir::PLUSX);
			
			/**
			* @brief Get the entity's global position in the level.
			*/
			vec3 getEntityPos(entityID entityID);

			/**
			* @brief Change the entity's active state.
			*/
			void changeEntityActiveState(entityID entityID, bool state);

			/**
			* @brief Delete an entity.
			*/
			void deleteEntity(entityID entityID);

			/**
			* @brief Delete an AI agent.
			*/
			void deleteAgent(agentID agentID);

			/**
			* @brief The implementation of the tick function that executes the next AI action
			* from the record file each time a determined amount of ticks have passed.
			*/
			void playRecordTick();


			// Clean up.

			/**
			* @brief Clean heap memory allocated by this system.
			* More precisely, it cleans the heap memory allocated with
			* registered AI games.
			*/
			static void cleanUp();

		protected:

			/*
			Attributes.
			*/

			static std::atomic<bool> recording_,
									 recordAgentModifiedBlocks_,
								     gameInProgress_;
			static std::ofstream saveFile_;
			static std::string saveDataBuffer_, 
							   saveFileName_;
			static recordPlayMode recordPlayMode_;

			// Stores a sequence of instantly followed block modifications done to each AI agent's level by the agent itself.
			static std::vector<std::deque<block>> agentModifiedBlocks_;

			// Stores the entity's ID in order of creation to, for example, properly delete them when playing a record in backwards mode.
			static std::list<entityID> entityIDcreationOrder_;
			// Stores the agent's ID in order of creation to, for example, properly delete them when playing a record in backwards mode.
			static std::list<agentID> agentIDcreationOrder_;

			std::string name_;

			// Set of entities' ID who are also AI agents.	
			std::unordered_set<entityID> entityIDIsAgent;

			std::vector<agentID> AIagentEntityID_; // Map between AI agent ID and entity ID.
			std::vector<blockViewDir> AIagentLookDirection_; // Map between AI agent ID and the direction its looking at.
			std::unordered_set<agentID> freeAIagentID_;

			// Block palette used for converting between namespaced block IDs and numerical IDs used in the AI game.
			std::unordered_map<std::string, numericShortID> blockPalette_;
			std::unordered_map<numericShortID, std::string> inverseBlockPalette_;
			std::unordered_set<numericShortID> freeInternalIDs_;


			/*
			Methods.
			*/


			// Modifiers: general.

			virtual void generalSetUp_() = 0;

			/*
			User should be able to select any of the options that the AI game offers
			(train the ai agents (if able), test the ai agents, generate a record from a match...)
			from the menu displayed in the terminal with this method.
			*/
			virtual void displayMenu_() = 0;

			virtual void setUpTest_(unsigned int nAgents, unsigned int nEpochs) = 0;

			virtual void test_() = 0;

			virtual void setUpRecord_(unsigned int nAgents) = 0;

			virtual void record_() = 0;

			/*
			Used when creating the AI agents for the first time or
			resetting them to their initial state.
			If 'nAgents' has a different value than the one
			in the last call made to this method, then it will
			delete or create AI agents depending on the difference.
			*/
			virtual void spawnAgents_(unsigned int nAgents) = 0;

			/*
			It returns 1 if 'recordPath' is invalid.
			It returns 2 if 'recordFilename' is invalid.
			It returns 3 if there is already a file at 'recordPath' + 'recordFilename' + ".rec".
			If any of the parameters is invalid an error message will be printed to the terminal.
			Otherwise it returns 0 and plays and generates the record of said AI match.
			If this method does not return 0 that means that the record has not
			been generated and the match has not been played.
			The extension ".rec" is automatically appended to 'filename'.
			*/
			static unsigned int generateRecord_(const std::string& path, const std::string& filename);

			/*
			Returns 1 if 'path' is invalid.
			Otherwise returns 0 and plays the recorded AI match using the engine's graphical mode.
			The extension ".rec" is automatically appended to 'filename'.
			The record must be executed in ticks inside the game's graphical mode.
			To go forward or backwards in the record, use aiGame::playRecordTick()
			*/
			static unsigned int playRecord_(const std::string& path);


			// Clean up.

			/*
			Clean up any resources for the last AI game loaded.
			*/
			virtual void cleanUpGame_() = 0;

			/*
			Clean up any resources for the last AI game match.
			*/
			virtual void cleanUpMatch_() = 0;

		private:

			/*
			Attributes.
			*/
			
			static bool initialised_,
				        canForwardReplay_,
				        canBackwardReplay_,
						backwardAIactionFound_;
			static aiGame* selectedGame_;
			static std::unordered_map<std::string, aiGame*> aiGames_;
			static std::vector<aiGame*> gamesRegisterOrder_;
			static std::vector<AIagentAction> aiRecordActions_;
			static std::unordered_map<std::string, unsigned int> AIactionsName_;
			
			// Specific to recording file parsing.
			static std::ifstream loadedFile_;
			static std::string readFileData_,
							   readWord_;
			static char readCharacter_;
			static unsigned int readActionCode_,
							    readParamTypeInd_,
							    readState_, // 0 = Reading aigame line.
											// 1 = Reading level line.
											// 2 = Reading actions.
								lastParamInd_, // ID of the entity that has the tick function that executes a tick from the record playing process.
								lastRawParamInd_;
			static double oldActualTime_;

			// Here are allocated the parameters available for the agent actions that are going to be executed as part of a record of an AI game.
			// The parameters are used the following way.
			// First, parameters should be allocated right before calling aiGame::playActionRecordForward(unsigned int actionCode)
			// or aiGame::playActionRecordForward(unsigned int actionCode) with the action code that corresponds
			// to the AI agent action that needs the parameters we are allocating.
			// Second, those parameters should be allocated in the first indices of these vectors. For example, if we need to execute the AI
			// agent action with code 2 and that action needs 2 floats and 1 unsigned integer as parameters, we will
			// allocate said 2 float parameters in the first and second indices of the float input parameters vector, replacing any old values if they exist.
			// The unsigned integer parameter will be allocated in the third index, overwriting any previous existing value.
			// Third, only basic data types are supported (VoxelEng::block is a typedef of a basic data type).
			// Last, only alphanumeric characters are allowed as char type parameteres.
			static std::vector<agentActionArg> params_;
			static std::deque<std::string> rawParams_;

			// Specific to recording file parsing ends.


			/*
			Methods.
			*/

			// Observers.

			bool actionRegistered_(unsigned int actionCode) const;


			// Modifiers.

			/*
			Push the parameter in raw string form provided by 'param' and parses it to the corresponding type depending
			on the 'actionCode' provided and the parameter type index related to said action code.
			*/
			void pushParam_(const std::string& param, unsigned int actionCode, unsigned int paramTypeInd);

			/*
			Same as pushParam_() but it is used to push and parse all the parameters related to the specified action
			in 'actionCode'. The parameters (in std::string raw format as they are directly read from a recording file)
			that are going to be pushed and parsed need to be stored in order (the first parameter in
			the queue is the first one to be pushed and parsed) in the deque rawParams_.
			*/
			void pushParams_(unsigned int actionCode);

			void playActionRecordForward_(unsigned int actionCode);

			void playActionRecordBackwards_(unsigned int actionCode);

		};

		inline aiGame::aiGame()
		{}

		inline const std::string& aiGame::recordFilename() {

			return saveFileName_;

		}

		inline bool aiGame::recording() {
		
			return recording_;
		
		}

		inline bool aiGame::playingRecord() {

			return game::selectedEngineMode() == engineMode::PLAYINGRECORD;

		}

		inline bool aiGame::playingRecordForward() {

			return recordPlayMode_ == recordPlayMode::FORWARD;

		}

		inline bool aiGame::recordPaused() {
		
			return recordPlayMode_ == recordPlayMode::PAUSE;
		
		}

		inline const std::string& aiGame::name() const {
		
			return name_;
		
		}

		inline bool aiGame::isAgentRegistered(agentID agentID) const {

			return agentID < AIagentEntityID_.size() && freeAIagentID_.find(agentID) == freeAIagentID_.cend();

		}

		inline bool aiGame::recordAgentModifiedBlocks() const {
		
			return recordAgentModifiedBlocks_;
		
		}

		inline numericShortID aiGame::getInternalID(const VoxelEng::block& block) const {

			return getInternalID(block.name());

		}

		template <class T>
		requires std::derived_from<T, aiGame>
		static void aiGame::registerGame(const std::string& gameName) {
		
			if (aiGames_.find(gameName) == aiGames_.cend()) {
			
				T* newGame = new T();

				aiGames_.insert({ gameName, newGame });
				gamesRegisterOrder_.push_back(newGame);

				// Create the directories for the records and AI data for these games
				// if not already created.
				std::string recordsPath = "records/" + gameName,
					        AIDataPath = "AIData/" + gameName,
					        terrainPath = "saves/recordingWorlds/" + gameName;
				if (std::filesystem::exists(recordsPath)) {
					
					if (!std::filesystem::is_directory(recordsPath))
						logger::errorLog(recordsPath + " is occupied by a file. Please move it.");

				}
				else
					std::filesystem::create_directory(recordsPath);

				if (std::derived_from<T, trainingGame>) {
				
					if (std::filesystem::exists(AIDataPath)) {

						if (!std::filesystem::is_directory(AIDataPath))
							logger::errorLog(AIDataPath + " is occupied by a file. Please move it.");

					}
					else
						std::filesystem::create_directory(AIDataPath);
				
				}

				if (std::filesystem::exists(terrainPath)) {

					if (!std::filesystem::is_directory(terrainPath))
						logger::errorLog(terrainPath + " is occupied by a file. Please move it.");

				}
				else
					std::filesystem::create_directory("saves/recordingWorlds/" + gameName);

			}
			else
				logger::errorLog("An AI game with the name " + gameName + " is already registered");
		
		}

		inline void aiGame::selectOriginalWorld() {

			chunkManager::selectOriginalWorld();

		}

		inline bool aiGame::isInWorld(const VoxelEng::vec3& pos) {

			return isInWorld(pos.x, pos.y, pos.z);

		}

		inline const block& aiGame::setBlock(agentID agentID, const vec3& pos, const block& blockID, bool record) {

			return setBlock(agentID, pos.x, pos.y, pos.z, blockID, record);

		}

		inline std::vector<const block*> aiGame::getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2) {

			return chunkManager::getBlocksBox(x1, y1, z1, x2, y2, z2);

		}

		inline std::vector<const block*> aiGame::getBlocksBox(const vec3& pos1, const vec3& pos2) {

			return getBlocksBox(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z);

		}

		inline void aiGame::moveEntity(entityID entityID, const vec3& movement) {

			moveEntity(entityID, movement.x, movement.y, movement.z);

		}

		inline void aiGame::setEntityPos(unsigned entityID, const vec3& pos) {

			setEntityPos(entityID, pos.x, pos.y, pos.z);

		}

		inline void aiGame::rotateEntity(entityID entityID, const vec3& rot) {
		
			rotateEntity(entityID, rot.x, rot.y, rot.z);
		
		}

		inline void aiGame::inverseRotateEntity(unsigned entityID, const vec3& rot) {
		
			inverseRotateEntity(entityID, rot.x, rot.y, rot.z);
		
		}

		inline unsigned int aiGame::createEntity(unsigned int entityTypeID, const vec3& pos, const vec3& rot) {

			return createEntity(entityTypeID, pos.x, pos.y, pos.z, rot.x, rot.y, rot.z);

		}

		inline unsigned int aiGame::createAgent(unsigned int entityTypeID, const vec3& pos, blockViewDir faceViewDir) {

			return createAgent(entityTypeID, pos.x, pos.y, pos.z, faceViewDir);

		}

		inline bool aiGame::actionRegistered_(unsigned int actionCode) const {
		
			return actionCode < aiRecordActions_.size();
		
		}

		// 'trainingGame' class.

		class trainingGame : public aiGame {

		public:

			// Modifiers.

		protected:

			// Modifiers.

			virtual void setUpTraining_(unsigned int nAgents, unsigned nEpochs) = 0;

			virtual void train_() = 0;

			virtual bool trainLoadedAgents_(const std::string& path) = 0;

			virtual bool testLoadedAgents_(const std::string& path) = 0;

			virtual bool recordLoadedAgents_(const std::string& path) = 0;

			/*
			Returns the number of loaded agents or 0 if an error ocurred.
			*/
			virtual int loadAgentsData_(const std::string& path) = 0;

			virtual void saveAgentsData_(const std::string& path) = 0;

			/*
			It returns 1 if 'recordPath' is invalid.
			It returns 2 if 'recordFilename' is invalid.
			It returns 3 if there is already a file at 'recordPath' + 'recordFilename' + ".rec".
			It returns 4 if 'agentsPath' is invalid.
			If any of the parameters is invalid an error message will be printed to the terminal.
			Otherwise it returns 0 and plays and generates the record of said AI match.
			If this method does not return 0 that means that the record has not
			been generated and the match has not been played.
			The extension ".rec" is automatically appended to 'filename'.
			*/
			unsigned int generateRecordLoadedAgents_(const std::string& recordPath, const std::string& recordFilename,	
													 const std::string& agentsPath);

		};

	}

}

#endif