/**
* @file AIGameEx1.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title AI Game example.
* @brief Contains the example AI game.
*/
#ifndef _AIEXAMPLE_EX1_
#define _AIEXAMPLE_EX1_
#include <string>
#include <unordered_map>
#include <array>
#include <arrayfire.h>
#include "af/random.h"
#include "../AIAPI.h"
#include "../definitions.h"
#include "../worldGen.h"
#include "../chunk.h"
#include "genetic.h"
#include "NN.h"


namespace AIExample {


	////////////////////////
	//Function prototypes.//
	////////////////////////

	float miningAIGameFitness(unsigned int individualID);


	/////////////
	//Typedefs.//
	/////////////

	/**
	* @brief A chunk's height map stores the highest non-null block in
	* in all the X,Z block columns of the chunk.
	*/
	typedef std::array<std::array<int, VoxelEng::SCZ>, VoxelEng::SCX> chunkHeightMap;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Custom world generation for the example AI game.
	*/
	class miningWorldGen : public VoxelEng::worldGen {

	public:

		// Constructors.

		/**
		* @brief Default constructor.
		*/
		miningWorldGen();


		// Observers.

		/**
		* @brief Gets a spawn position for players and entities.
		*/
		const VoxelEng::vec3& spawnPos() const;

	protected:

		// Modifiers.

		void prepareGen_();

		void generate_(VoxelEng::chunk& chunk);

		void cascadeOreGen_(const VoxelEng::vec3 chunkPos, unsigned int& nBlocksCounter, unsigned int nBlocks,
			unsigned int inChunkX, unsigned int inChunkY, unsigned int inChunkZ, VoxelEng::block oreID);


	private:

		/*
		Attributes.
		*/

		static const std::uniform_int_distribution<unsigned int> int6Dice_;
		static std::uniform_int_distribution<unsigned int> intDice_;
		static std::uniform_real_distribution<float> floatDice_;

		bool spawnSet_;
		int maxBlockYCoord_;
		VoxelEng::vec3 AISpawnPos_; // Same spawn position for every AI agent.
		std::unordered_map<VoxelEng::vec2, chunkHeightMap> chunkColHeight_;
		std::uniform_int_distribution<unsigned int>::param_type coalSpreadRange_,
																ironSpreadRange_,
																goldSpreadRange_,
																diamondSpreadRange_;
		VoxelEng::vec2 oreInChunkPos_;

		enum class ore { COAL, IRON, GOLD, DIAMOND };

		/*
		Methods.
		*/

		// Modifiers.

		/*
		Generates a height map from a chunk column (that is, a set
		of chunks that share the same X and Z chunk coordinate) using
		perlin noise if it is not already generated. Otherwise it
		returns said value for the specified chunk column.
		*/
		const chunkHeightMap& chunkHeightMap_(int blockX, int blockZ);

		/*
		Generates a height map from a chunk column (that is, a set
		of chunks that share the same X and Z chunk coordinate) using
		perlin noise if it is not already generated. Otherwise it
		returns said value for the specified chunk column.
		*/
		const chunkHeightMap& chunkHeightMap_(const VoxelEng::vec2& blockXZPos);

		void generateChunkHeightMap_(const VoxelEng::vec2& chunkXZPos);

		/*
		'inChunkX', 'inChunkY' and 'inChunkZ' serve as the starting point to generate the ore.
		*/
		void generateOre_(VoxelEng::vec3 inChunkPos, VoxelEng::chunk& chunk, ore ore);

	};

	inline miningWorldGen::miningWorldGen()
		: spawnSet_(false), maxBlockYCoord_(0), AISpawnPos_(VoxelEng::vec3Zero)
	{}

	inline const VoxelEng::vec3& miningWorldGen::spawnPos() const {
	
		return AISpawnPos_;
	
	}

	inline const chunkHeightMap& miningWorldGen::chunkHeightMap_(const VoxelEng::vec2& blockXZPos) {

		return chunkHeightMap_(blockXZPos[0], blockXZPos[1]);

	}


	// 'miningAIGame' class.

	/**
	* @brief AI game where the AI agents need to collect underground resources with a limited amount
	* of actions that they can perform per match.
	*/
	class miningAIGame : public VoxelEng::AIAPI::trainingGame {

	public:

		// Constructors.

		/**
		* @brief Default constructor.
		*/
		miningAIGame();


		// Observers.

		/**
		* @brief Get the "genetic" object that manages all the game's genetic algorithms part.
		*/
		const genetic& getGenetic() const;

		/**
		* @brief Get how far the AI agents can "see" blocks from their position.
		*/
		unsigned int visionDepth() const;

		/**
		* @brief Get how wide the AI agents vision is.
		*/
		unsigned int visionRadius() const;

		/**
		* @brief Get the score associated with a specified block ID.
		*/
		float blockScore(VoxelEng::block) const;

		/**
		* @brief Method to indicate if this AI game requires training of the agents or not.
		*/
		bool needsTraining() const;

		/**
		* @brief Number of actions that any AI agent spawns with.
		*/
		unsigned int nInitialActions() const;

		/**
		* @brief The AI agent shouldn't spam moves that do not cost actions to avoid infinite loops.
		* Returns the number of actions without cost that can be performed without penalization.
		* Said penalization should result in decreasing the number of actions that the agent has left.
		*/
		unsigned int nNoCostActionsToPen() const;


		// Modifiers.

		/**
		* @brief The random generator used by the AI game with things that do not involve terrain generation.
		*/
		af::randomEngine& AIrandEng();

		/**
		* @brief Set seed used to generate random weight data when creating new neural networks and
		* similar uses.
		*/
		void setAISeed();

		/**
		* @brief Set seed used to generate random weight data when creating new neural networks and
		* similar uses.
		*/
		void setAISeed(unsigned int seed);

		/**
		* @brief Get the "genetic" object that manages all the game's genetic algorithms part.
		*/
		genetic& getGenetic();

		/**
		* @brief Increase or substract an AI agent's score with bounds checking.
		*/
		void addScoreAt(unsigned int individualID, float score);

		/**
		* @brief Increase or substract an AI agent's score without bounds checking.
		*/
		void addScore(unsigned int individualID, float score);

		/**
		* @brief Get an AI agent's score with bounds checking.
		*/
		float getScoreAt(unsigned int individualID);

		/**
		* @brief Get an AI agent's score without bounds checking.
		*/
		float getScore(unsigned int individualID);

		/**
		* @brief Load an AI data file.
		*/
		int loadAgentsData(const std::string& path);

		/**
		* @brief Save the current AI data of all the agents into an AI data file.
		*/
		void saveAgentsData(const std::string& path);

		/**
		* @brief Sets the agents' model to a registered one.
		* By default the model used is the default one (model ID = 0).
		*/
		void setAgentsModel(unsigned int modelID);

		/**
		* @brief Create (or reset if they are already created by a previous call to this method)
		* as many agents as needed. Said number of needed AI agents is automatically
		* provided by the set up methods.
		*/
		void spawnAgents();

	protected:

		/*
		Methods.
		*/


		// Modifiers.

		void generalSetUp_();

		void displayMenu_();

		void setUpTest_(unsigned int nAgents, unsigned int nEpochs);

		void test_();

		void setUpRecord_(unsigned int nAgents);

		void record_();

		void spawnAgents_(unsigned int nAgents);

		void cleanUpMatch_();
		
		void cleanUpGame_();

		void setUpTraining_(unsigned int nAgents, unsigned nEpochs);

		void train_();

		bool trainLoadedAgents_(const std::string& path);

		bool testLoadedAgents_(const std::string& path);

		bool recordLoadedAgents_(const std::string& path);

		int loadAgentsData_(const std::string& path);

		void saveAgentsData_(const std::string& path);

	private:

		std::random_device rd_;
		std::mt19937 randGen_;
		std::uniform_int_distribution<unsigned int> UIdist_;
		af::randomEngine AFrandEng_; // Random engine used separately from the world generation for the AI training.
		unsigned int popSize_,
					 visionDepth_,
					 visionRadius_,
					 epochForNewWorld_,
					 nInputs_,
					 nEpochs_,
			         nEpochsBetweenSaves_,
				     agentsModelID_;
		std::unordered_map<VoxelEng::block, float> blockScore_;
		std::vector<float> scores_;
		genetic genetic_;
		std::string lastWorldPath_;

	};

	inline miningAIGame::miningAIGame()
		: randGen_(rd_()), UIdist_(0, std::numeric_limits<unsigned int>::max()), popSize_(0), visionDepth_(0),
		visionRadius_(0), epochForNewWorld_(0), agentsModelID_(2), nEpochsBetweenSaves_(0), nEpochs_(0), nInputs_(0) {

		name_ = "MiningAIGame";

	}
	
	inline af::randomEngine& miningAIGame::AIrandEng() {
	
		return AFrandEng_;
	
	}

	inline const genetic& miningAIGame::getGenetic() const {

		return genetic_;

	}

	inline void miningAIGame::setAISeed() {

		AFrandEng_.setSeed(UIdist_(randGen_));
		VoxelEng::logger::debugLog("AF seed: " + std::to_string(AFrandEng_.getSeed()));

	}

	inline void miningAIGame::setAISeed(unsigned int seed) {

		AFrandEng_.setSeed(seed);
		VoxelEng::logger::debugLog("AF seed: " + std::to_string(AFrandEng_.getSeed()));

	}

	inline genetic& miningAIGame::getGenetic() {

		return genetic_;

	}

	inline void miningAIGame::setAgentsModel(unsigned int modelID) {
	
		agentsModelID_ = modelID;
	
	}

	inline void miningAIGame::spawnAgents() {
	
		spawnAgents_((recording_) ? popSize_ : popSize_ * 2); // Newborn agents have their separate agent entity as well.
	
	}

	inline unsigned int miningAIGame::visionDepth() const {
	
		return visionDepth_;
	
	}

	inline unsigned int miningAIGame::visionRadius() const {
	
		return visionRadius_;
	
	}

	inline bool miningAIGame::needsTraining() const {
	
		return true;
	
	}

	inline unsigned int miningAIGame::nInitialActions() const {
	
		return 500;
	
	}

	inline unsigned int miningAIGame::nNoCostActionsToPen() const {
		
		return 10;
	
	}

	inline void miningAIGame::addScore(unsigned int individualID, float score) {
	
		scores_[individualID] += score;

	}

	inline float miningAIGame::getScore(unsigned int individualID) {

		return scores_[individualID];

	}

	inline int miningAIGame::loadAgentsData_(const std::string& path) {

		return genetic_.loadIndividualsData(path);

	}

	inline void miningAIGame::saveAgentsData_(const std::string& path) {
	
		genetic_.saveIndividualsData(path);
	
	}

}

#endif