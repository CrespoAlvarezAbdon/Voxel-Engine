#ifndef _AIEXAMPLE_EX1_
#define _AIEXAMPLE_EX1_
#include <string>
#include <unordered_map>
#include <array>
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

	typedef std::array<std::array<int, SCZ>, SCX> chunkHeightMap;


	////////////
	//Classes.//
	////////////

	class miningWorldGen : public VoxelEng::worldGen {

	public:

		// Constructors.

		miningWorldGen();


		// Observers.

		const VoxelEng::vec3& spawnPos() const;

	protected:

		// Modifiers.

		void prepareGen_();

		void generate_(VoxelEng::chunk& chunk);

	private:

		/*
		Attributes.
		*/

		static std::uniform_int_distribution<unsigned int> dice_;

		bool spawnSet_;
		int maxBlockYCoord_;
		VoxelEng::vec3 AISpawnPos_; // Same spawn position for every AI agent.
		std::unordered_map<VoxelEng::vec2, chunkHeightMap> chunkColHeight_;

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

	class miningAIGame : public VoxelEng::AIAPI::trainingGame {

	public:

		// Constructors.

		miningAIGame();


		// Observers.

		const genetic& getGenetic() const;

		unsigned int visionDepth() const;

		unsigned int visionRadius() const;

		float blockScore(VoxelEng::block) const;

		bool needsTraining() const;

		unsigned int nInitialActions() const;

		/*
		The AI agent shouldn't spam moves that do not cost actions to avoid infinite loops.
		Returns the number of actions without cost that can be performed without penalization.
		Said penalization should result in decreasing the number of actions that the agent has left.
		*/
		unsigned int nNoCostActionsToPen() const;


		// Modifiers.

		genetic& getGenetic();

		void addScore(unsigned int individualID, float score);

		bool loadAgentsData(const std::string& path);

		void saveAgentsData(const std::string& path);

		/*
		By default the model used is the default one (ID = 0).
		*/
		void setAgentsModel(unsigned int modelID);

		/*
		Create (or reset if they are already created by a previous call to this method)
		as many agents as needed. Said number of needed AI agents is automatically
		provided by the set up methods.
		*/
		void spawnAgents();

	protected:

		/*
		Methods.
		*/


		// Modifiers.

		void generalSetUp_();

		void displayMenu_();

		void setUpTest_();

		void test_();

		void setUpRecord_();

		void record_();

		void spawnAgents_(unsigned int nAgents);

		void setUpTraining_();

		void train_();

		bool trainLoadedAgents_(const std::string& path);

		bool testLoadedAgents_(const std::string& path);

		bool recordLoadedAgents_(const std::string& path);

		bool loadAgentsData_(const std::string& path);

		void saveAgentsData_(const std::string& path);

	private:

		unsigned int popSize_,
					 visionDepth_,
					 visionRadius_,
					 epochForNewWorld_,
					 nInputs_,
					 nEpochs_,
			         nEpochsBetweenSaves_,
				     agentsModelID_;
		std::vector<unsigned int> layerSize_;
		std::unordered_map<VoxelEng::block, float> blockScore_;
		std::vector<float> scores_;
		genetic genetic_;

	};

	inline const genetic& miningAIGame::getGenetic() const {

		return genetic_;

	}

	inline genetic& miningAIGame::getGenetic() {

		return genetic_;

	}

	inline void miningAIGame::setAgentsModel(unsigned int modelID) {
	
		agentsModelID_ = modelID;
	
	}

	inline void miningAIGame::spawnAgents() {
	
		spawnAgents_(popSize_);
	
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
	
		return 10;
	
	}

	inline unsigned int miningAIGame::nNoCostActionsToPen() const {
	
		return 500;
	
	}

	inline miningAIGame::miningAIGame()
		: popSize_(0), visionDepth_(0), visionRadius_(0), epochForNewWorld_(0),
		agentsModelID_(2), nEpochsBetweenSaves_(0), nEpochs_(0), nInputs_(0)
	{}

	inline bool miningAIGame::loadAgentsData_(const std::string& path) {

		return genetic_.loadIndividualsData(path);

	}

	inline void miningAIGame::saveAgentsData_(const std::string& path) {
	
		genetic_.saveIndividualsData(path);
	
	}

}

#endif