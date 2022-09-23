#ifndef _VOXELENG_AIAPI_
#define _VOXELENG_AIAPI_
#include <vector>
#include <deque>
#include <string>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <initializer_list>
#include <cstddef>
#include <concepts>
#include <deque>
#include "definitions.h"
#include "entity.h"
#include "chunk.h"
#include "logger.h"
#include "worldGen.h"


namespace VoxelEng {

	namespace AIAPI {

		////////////
		//Classes.//
		////////////

		class agentActionArg {

		public:

			/*
			Attributes.
			*/
			enum class type { INT, UINT, FLOAT, CHAR, BOOL, BLOCK } tag;
			union {

				int i;
				unsigned int ui;
				float f;
				char c;
				bool b;
				block bl;

			};


			/*
			Methods.
			*/

			// Constructors.

			agentActionArg(int value);

			agentActionArg(unsigned int value);

			agentActionArg(float value);

			agentActionArg(char value);

			agentActionArg(bool value);

			agentActionArg(block value);


		private:

			const unsigned int unionSize_ = 6;

		};


		class AIagentAction {

		public:

			// Constructors.

			/*
			'recordFunc' should be a pointer to a void function that needs an 'aiGame' object as parameter
			and that it calls to aiGame::recordAction(actionstring) on said object
			*/
			AIagentAction(void(*action)(), std::initializer_list<agentActionArg::type> paramTypes);


			// Observers.

			agentActionArg::type paramType(unsigned int index) const;


			// Non-modifiers.

			void playRecordedAction();


		private:

			void(*action_)();
			const std::vector<agentActionArg::type> paramTypes_;

		};

		inline void AIagentAction::playRecordedAction() {

			action_();

		}

		/*
		WARNING: calling aiGame's constructor or the constructor of any class that derives
		from aiGame results in undefined behaviour. Use aiGame::registerGame to create
		a new AI game object from any derived class.
		*/
		class aiGame {

		public:

			// Initialization.

			static void init();


			// Constructors.

			aiGame();


			// Observers: general.

			static aiGame* selectedGame();

			/*
			Name used to identify AI save data along with the date the save was made.
			*/
			const std::string& name() const;

			/*
			Note. The methods inside the aiGame class already check this. This method
			is made public for use outside said class.
			*/
			bool isAgentRegistered(unsigned int agentID) const;

			bool entityIsAgent(unsigned int entityID) const;

			bool playingRecordForward() const;


			// Observers: Agent actions API. 

			template <typename T>
			T getParam(unsigned int index) const = delete;

			template <>
			int getParam<int>(unsigned int index) const;

			template <>
			unsigned int getParam<unsigned int>(unsigned int index) const;

			template <>
			float getParam<float>(unsigned int index) const;

			template <>
			char getParam<char>(unsigned int index) const;

			template <>
			bool getParam<bool>(unsigned int index) const;

			template <>
			block getParam<block>(unsigned int index) const;

			/*
			Returns if the AI agent has made modifications to its copy of the
			original level that is being used in the AI game.
			*/
			bool hasModifiedLevel(unsigned int agentID) const;

			bool recordAgentModifiedBlocks() const;


			// Modifiers: general.

			/*
			Register an instance of a 'aiGame' object (or an object from a class that derives from 'aiGame')
			to use it when generating new chunks in levels.
			'T' is the class that either is 'aiGame' or a class that derives from 'aiGame'.
			*/
			template <class T>
			requires std::derived_from<T, aiGame>
			static void registerGame(const std::string& gameName);

			static void selectGame(const std::string& gameName);

			/*
			Select the 'index'º registered AI game.
			'index' begins at 0 and the AI games are sorted
			in the order they were registered.
			*/
			static void selectGame(unsigned int index);

			/*
			WARNING. There must be an AI game previously loaded using aiGame::selectGame().
			*/
			static void startGame();

			/*
			WARNING. There must be an AI game previously loaded using aiGame::selectGame() and
			it must have been started previosuly using aiGame::startGame().
			*/
			static void finishGame();

			/*
			Display all registered AI games in the terminal window
		    for selection. Returns the number of registered AI games.
			*/
			static unsigned int listAIGames();


			// Modifiers: Agent actions' API.

			static void registerAction(const std::string& actionName, const AIagentAction& action);

			/*
			The included parameters' indices are in range [paramIndBegin, paramIndEnd].
			WARNING. Make sure that the recording flag is set to true before recording an action.
			*/
			void recordAction(const std::string& actionName, std::initializer_list<agentActionArg> args);

			block getLastModifiedBlock(unsigned int agentID, bool popBlock = false);

			block getFirstModifiedBlock(unsigned int agentID, bool popBlock = false);

			void initBlockModRecording();

			/*
			Also sets the record block modifications flag to false.
			*/
			void clearBlockModRecording();

			void generateAIWorld(const std::string& path = "");

			/*
			Get block and set block operations in the chunk manager system will now
			be performed on the AI world/level of AI agent with ID 'individualID'.
			AI mode must be turned on in the chunk manager system.
			WARNING. This method is not thread safe.
			*/
			void selectAIworld(unsigned int individualID);

			/*
			ONLY get block operations in the chunk manager system will now
			be performed on the original copy of the level that is being used for the AI game.
			AI mode must be turned on in the chunk manager system.
			Set block operations will use the latest AI world selected (the one corresponding to
			the AI agent with ID 0 by default).
			WARNING. This method is not thread safe.
			*/
			void selectOriginalWorld();

			/*
			This method cannot erase a modified block entry from the agent's record because
			it would render said record invalid as it would no longer contain a
			sequence of instantly followed block modifications.
			*/
			block getModifiedBlock(unsigned int entityID, unsigned int blockIndex);

			static bool isInWorld(const VoxelEng::vec3& pos);

			static bool isInWorld(int x, int y, int z);


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

			unsigned int blockViewDir(unsigned int agentID);

			/*
			Block ID 0 equals no block or empty/null block.
			Returns the old ID of the modified block.
			WARNING. Select the proper level you want to access with aiGame::selectAIWorld() or aiGame::selectOriginalWorld()
			before calling this method.
			*/
			block setBlock(unsigned int entityID, const vec3& pos, block blockID, bool record);

			/*
			Block ID 0 equals no block or empty/null block.
			Returns the old ID of the modified block.
			WARNING. Select the proper level you want to access with aiGame::selectAIWorld() or aiGame::selectOriginalWorld()
			before calling this method.
			*/
			block setBlock(unsigned int entityID, int x, int y, int z, block blockID, bool record);

			/*
			Get all blocks in the world that are in the box defined with the positions pos1 and pos2 taking into
			account that pos1.x <= pos2.x ^ pos1.y <= pos2.y ^ pos1.z <= pos2.z.
			*/
			std::vector<block> getBlocksBox(const vec3& pos1, const vec3& pos2);

			/*
			Get all blocks in the world that are in the box defined with the positions pos1 and pos2 taking into
			account that x1 <= x2 ^ y1 <= y2 ^ z1 <= z2.
			*/
			std::vector<block> getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2);

			/*
			Performs agent.pos() += movement;
			*/
			void moveEntity(unsigned int entityID, const vec3& movement);

			/*
			Performs agent.pos() += movement;
			*/
			void moveEntity(unsigned int entityID, int x, int y, int z);

			/*
			Apply rotation to the AI agent's model so it faces one of the 3 axes in one of both directions.
			For example, if the rotation vector of an agent is (90, 90, 0) and we apply rotateEntityFixed(agentID, UP),
			then said rotation vector will be (90, 180, 0) and if we apply rotateEntityFixed(agentID, DOWN)
			to said result said vector will now be again (90, 90, 0).
			*/
			void rotateAgentViewDir(unsigned int agentID, unsigned int direction);

			/*
			AI agents cannot be rotated with this method, use aiGame::rotateAgentViewDir() instead.
			The rotations are performed in the XYZ order.
			*/
			void rotateEntity(unsigned int entityID, const vec3& rot);

			/*
			AI agents cannot be rotated with this method, use aiGame::rotateAgentViewDir() instead.
			The rotations are performed in the XYZ order.
			*/
			void rotateEntity(unsigned entityID, float rotX, float rotY, float rotZ);

			/*
			Performs the following rotations.
			entity.rotateZ(rotZ);
			entity.rotateY(rotY);
			entity.rotateX(rotX);
			That is, the rotations are performed in the ZYX order.
			*/
			void inverseRotateEntity(unsigned entityID, const vec3& rot);

			/*
			Performs the following rotations.
			entity.rotateZ(rotZ);
			entity.rotateY(rotY);
			entity.rotateX(rotX);
			That is, the rotations are performed in the ZYX order.
			*/
			void inverseRotateEntity(unsigned entityID, float rotX, float rotY, float rotZ);

			unsigned int createEntity(unsigned int entityTypeID, const vec3& pos, const vec3& rot);

			unsigned int createEntity(unsigned int entityTypeID, int posX, int posY, int posZ, float rotX, float rotY, float rotZ);

			unsigned int createAgent(unsigned int entityTypeID, const vec3& pos, unsigned int faceViewDir = PLUSX);

			unsigned int createAgent(unsigned int entityTypeID, int x, int y, int z, unsigned int faceViewDir = PLUSX);
			
			vec3 getEntityPos(unsigned int entityID);

			void changeEntityActiveState(unsigned int entityID, bool state);

			/*
			WARNING. Only use when cleaning when the resources allocated with the AI game are no longer needed.
			For other cases, use aiGame::changeActiveState().
			*/
			void deleteEntity(unsigned int entityID);

			/*
			WARNING. Only use when cleaning when the resources allocated with the AI game are no longer needed.
			For other cases, use aiGame::changeActiveState().
			*/
			void deleteAgent(unsigned int agentID);

			/*
			Returns false if no EOF nor the beginning of the recording file were reached.
			Returns true otherwise.
			*/
			bool playRecordTick();


			// Clean up.

			/*
			Clean heap memory allocated by this system.
			More precisely, it cleans the heap memory allocated with
			registered AI games.
			*/
			static void cleanUp();

		protected:

			/*
			Attributes.
			*/

			static std::atomic<bool> recording_;
			static std::ofstream saveFile_;
			static std::string saveDataBuffer_;

			std::string name_;

			// Set of entities' ID who are also AI agents.	
			std::unordered_set<unsigned int> activeEntityID_;

			std::vector<unsigned int> AIagentID_, // Map between AI agent ID and entity ID.
									  AIagentLookDirection_; // Map between AI agent ID and the direction its looking at.
			std::unordered_set<unsigned int> freeAIAgentID_;

			VoxelEng::duration durationBetweenActions_; // Duration between playing a record's actions.
			bool playingRecordForward_,
				 recordAgentModifiedBlocks_;

			/*
			Stores a sequence of instantly followed block modifications done to each AI agent's level by the agent itself.
			*/
			std::vector<std::deque<block>> agentModifiedBlocks_;


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

			virtual void setUpTest_() = 0;

			virtual void test_() = 0;

			virtual void setUpRecord_() = 0;

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
			unsigned int generateRecord_(const std::string& path, const std::string& filename);

			/*
			Returns 1 if 'path' is invalid.
			Otherwise returns 0 and plays the recorded AI match using the engine's graphical mode.
			The extension ".rec" is automatically appended to 'filename'.
			The record must be executed in ticks inside the game's graphical mode.
			To go forward or backwards in the record, use aiGame::playRecordTick()
			*/
			unsigned int playRecord_(const std::string& path);

			void stopPlayingRecord_();

		private:

			/*
			Attributes.
			*/
			
			static bool initialised_;
			static std::atomic<bool> gameInProgress_,
									 playingRecord_;
			static aiGame* selectedGame_;
			static std::unordered_map<std::string, aiGame*> aiGames_;
			static std::vector<aiGame*> gamesRegisterOrder_;
			static std::vector<AIagentAction> aiRecordActions_;
			static std::unordered_map<std::string, unsigned int> AIactionsName_;
			static std::string playbackTerrainFile_;
			static std::ifstream loadedFile_;
			static std::string readFileData_,
							   readWord_;
			static char readCharacter_;
			static unsigned int nFileCharsRead_,
								readActionCode_,
							    readParamTypeInd_,
							    readState_, // 0 = Reading aigame line.
											// 1 = Reading level line.
											// 2 = Reading actions.
								lastParamInd_,
								recordingPlayerEntity_; // ID of the entity that has the tick function that executes a tick from the record playing process.

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
			: durationBetweenActions_(0), playingRecordForward_(true), recordAgentModifiedBlocks_(false)
		{}

		inline const std::string& aiGame::name() const {
		
			return name_;
		
		}

		inline bool aiGame::isAgentRegistered(unsigned int agentID) const {

			return agentID < AIagentID_.size() && freeAIAgentID_.find(agentID) == freeAIAgentID_.cend();

		}

		inline bool aiGame::playingRecordForward() const {
		
			return playingRecordForward_;
		
		}

		inline bool aiGame::recordAgentModifiedBlocks() const {
		
			return recordAgentModifiedBlocks_;
		
		}

		template <class T>
		requires std::derived_from<T, aiGame>
		static void aiGame::registerGame(const std::string& gameName) {
		
			if (aiGames_.find(gameName) == aiGames_.cend()) {
			
				T* newGame = new T();

				aiGames_.insert({ gameName, newGame });
				gamesRegisterOrder_.push_back(newGame);
			
			}
			else
				logger::errorLog("An AI game with the name " + gameName + " is already registered");
		
		}

		inline void aiGame::generateAIWorld(const std::string& path) {

			chunkManager::generateAIWorld(path);

		}

		inline void aiGame::selectOriginalWorld() {

			chunkManager::selectOriginalWorld();

		}

		inline bool aiGame::isInWorld(const VoxelEng::vec3& pos) {

			return isInWorld(pos.x, pos.y, pos.z);

		}

		inline block aiGame::setBlock(unsigned int agentID, const vec3& pos, VoxelEng::block blockID, bool record) {

			return setBlock(agentID, pos.x, pos.y, pos.z, blockID, record);

		}

		inline std::vector<block> aiGame::getBlocksBox(const vec3& pos1, const vec3& pos2) {

			return getBlocksBox(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z);

		}

		inline void aiGame::moveEntity(unsigned int entityID, const vec3& movement) {

			moveEntity(entityID, movement.x, movement.y, movement.z);

		}

		inline void aiGame::rotateEntity(unsigned int entityID, const vec3& rot) {
		
			rotateEntity(entityID, rot.x, rot.y, rot.z);
		
		}

		inline void aiGame::inverseRotateEntity(unsigned entityID, const vec3& rot) {
		
			inverseRotateEntity(entityID, rot.x, rot.y, rot.z);
		
		}

		inline unsigned int aiGame::createEntity(unsigned int entityTypeID, const vec3& pos, const vec3& rot) {

			return createEntity(entityTypeID, pos.x, pos.y, pos.z, rot.x, rot.y, rot.z);

		}

		inline unsigned int aiGame::createAgent(unsigned int entityTypeID, const vec3& pos, unsigned int faceViewDir) {

			return createAgent(entityTypeID, pos.x, pos.y, pos.z, faceViewDir);

		}

		inline bool aiGame::actionRegistered_(unsigned int actionCode) const {
		
			return actionCode < aiRecordActions_.size();
		
		}

		inline void aiGame::stopPlayingRecord_() {

			playingRecord_ = false;

		}

		// 'trainingGame' class.

		class trainingGame : public aiGame {

		public:

			// Modifiers.

		protected:

			// Modifiers.

			virtual void setUpTraining_() = 0;

			virtual void train_() = 0;

			virtual bool trainLoadedAgents_(const std::string& path) = 0;

			virtual bool testLoadedAgents_(const std::string& path) = 0;

			virtual bool recordLoadedAgents_(const std::string& path) = 0;

			virtual bool loadAgentsData_(const std::string& path) = 0;

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