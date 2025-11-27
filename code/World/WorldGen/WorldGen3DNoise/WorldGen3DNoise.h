/**
* @file 3DNoiseGen.h
* @version 1.0
* @date 06/05/2025
* @author Abdon Crespo Alvarez
* @title 3D Perlin Noise world generator.
* @brief Definition of a world generator that used 3D Perlin Noise.
*/
#ifndef _VOXELENG_WORLD_GEN_3D_NOISE_
#define _VOXELENG_WORLD_GEN_3D_NOISE_

#include <array>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <FastNoiseLite.h>

#include <definitions.h>
#include <event.h>
#include <listener.h>
#include <vec.h>
#include <Block/block.h>
#include <Chunk/chunk.h>
#include <Noise/Perlin3D/Perlin3D.hpp>
#include <World/WorldGen/worldGen.h>


namespace VoxelEng {

	/////////////
	//Typedefs.//
	/////////////

	/**
	* @brief A chunk's height map stores the highest non-null block in
	* in all the X,Z block columns of the chunk.
	*/
	typedef std::array<std::array<int, CHUNK_SIZE>, CHUNK_SIZE> chunkHeightMap;


	////////////
	//Classes.//
	////////////

	// 'chunkLoadListener' class.

	/**
	* @brief Listener of the event when a chunk is loaded.
	*/
	class chunkLoadListener : public listener {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		chunkLoadListener(std::unordered_map<vec2, chunkHeightMap>& chunkColHeight,
						  std::unordered_map<vec2, unsigned int>& chunkColHeightUses,
					      std::mutex& chunkColHeightMutex);

		// Modifiers.

		/**
		* @brief The method to execute when the attached event occurs.
		*/
		virtual void onEvent(event* e);

	private:

		std::unordered_map<vec2, chunkHeightMap>& chunkColHeight_;
		std::unordered_map<vec2, unsigned int>& chunkColHeightUses_;
		std::mutex& chunkColHeightMutex_;

	};

	inline chunkLoadListener::chunkLoadListener(std::unordered_map<vec2, chunkHeightMap>& chunkColHeight,
		std::unordered_map<vec2, unsigned int>& chunkColHeightUses,
		std::mutex& chunkColHeightMutex)
		: chunkColHeight_(chunkColHeight),
		  chunkColHeightUses_(chunkColHeightUses),
		  chunkColHeightMutex_(chunkColHeightMutex)
	{}


	// 'chunkUnloadListener' class.

	/**
	* @brief Listener of the event when a chunk is loaded.
	*/
	class chunkUnloadListener : public listener {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		chunkUnloadListener(std::unordered_map<vec2, chunkHeightMap>& chunkColHeight,
							std::unordered_map<vec2, unsigned int>& chunkColHeightUses,
							std::mutex& chunkColHeightMutex);

		// Modifiers.

		/**
		* @brief The method to execute when the attached event occurs.
		*/
		virtual void onEvent(event* e);

	private:

		std::unordered_map<vec2, chunkHeightMap>& chunkColHeight_;
		std::unordered_map<vec2, unsigned int>& chunkColHeightUses_;
		std::mutex& chunkColHeightMutex_;

	};

	inline chunkUnloadListener::chunkUnloadListener(std::unordered_map<vec2, chunkHeightMap>& chunkColHeight,
		std::unordered_map<vec2, unsigned int>& chunkColHeightUses,
		std::mutex& chunkColHeightMutex)
		: chunkColHeight_(chunkColHeight),
		  chunkColHeightUses_(chunkColHeightUses),
		  chunkColHeightMutex_(chunkColHeightMutex)
	{}


	// 'WorldGen3DNoise' class.

	/**
	* @brief Custom world generation for the example AI game.
	*/
	class WorldGen3DNoise : public worldGen {

	public:

		// Constructors.

		/**
		* @brief Default constructor.
		*/
		WorldGen3DNoise(const block& ore1, const block& ore2, const block& ore3, const block& ore4,
					    const block& layer0, const block& layer1, const block& layer2, const block& air);


		// Observers.

		/**
		* @brief Gets a spawn position for players and entities.
		*/
		const vec3& spawnPos() const;

	protected:

		// Modifiers.

		void prepareGen_();

		void generate_(chunk& chunk);

		void genPass2_(chunk& chunk);

		void clear_();

	private:

		/*
		Attributes.
		*/

		static std::uniform_int_distribution<unsigned int> int6Dice_;
		static std::uniform_int_distribution<unsigned int> intDice_;
		static std::uniform_real_distribution<float> floatDice_;

		const block& ore1_;
		const block& ore2_;
		const block& ore3_;
		const block& ore4_;
		const block& layer0_;
		const block& layer1_;
		const block& layer2_;
		const block& air_;
		const block& lightBlock_;
		const block& waterBlock_;
		const block& beachBlock_;

		bool spawnSet_;
		float minHeight_,
			  maxHeight_;
		vec3 AISpawnPos_; // Same spawn position for every AI agent.
		std::mutex chunkColHeightMutex_;
		std::unordered_map<vec2, chunkHeightMap> chunkColHeight_;
		std::unordered_map<vec2, unsigned int> chunkColHeightUses_;
		std::uniform_int_distribution<unsigned int>::param_type ore1SpreadRange_,
																ore2SpreadRange_,
																ore3SpreadRange_,
																ore4SpreadRange_;
		const float threshold_;
		const float waterLevel_;
		const float perc_;
		chunkLoadListener chunkLoadListener_;
		chunkUnloadListener chunkUnloadListener_;

		FastNoiseLite noise_gen_;

		/*
		Methods.
		*/

		// Generation steps functions.

		void noiseLayer(chunk& chunk);

		void surfaceLayer(chunk& chunk);

		// Utilities.

		bool noiseMakesBlockAt(float x, float y, float z);

	};

	inline WorldGen3DNoise::WorldGen3DNoise(const block& ore1, const block& ore2, const block& ore3, const block& ore4,
		const block& layer0, const block& layer1, const block& layer2, const block& air)
		: spawnSet_(false), minHeight_(0), maxHeight_(0), AISpawnPos_(vec3Zero), ore1_(ore1), ore2_(ore2), ore3_(ore3), ore4_(ore4),
		layer0_(layer0), layer1_(layer1), layer2_(layer2), air_(air), lightBlock_(block::getBlockC("starminer::marbleBlock2")),
		waterBlock_(block::getBlockC("starminer::water")),
		beachBlock_(block::getBlockC("starminer::sand")),
		ore1SpreadRange_(std::uniform_int_distribution<unsigned int>::param_type(1, 8)),
		ore2SpreadRange_(std::uniform_int_distribution<unsigned int>::param_type(1, 7)),
		ore3SpreadRange_(std::uniform_int_distribution<unsigned int>::param_type(1, 5)),
		ore4SpreadRange_(std::uniform_int_distribution<unsigned int>::param_type(1, 3)),
		threshold_(0.0f),
	    waterLevel_(64),
	    perc_(0.03f),
		chunkLoadListener_(chunkColHeight_, chunkColHeightUses_, chunkColHeightMutex_),
		chunkUnloadListener_(chunkColHeight_, chunkColHeightUses_, chunkColHeightMutex_)
	{}

	inline const vec3& WorldGen3DNoise::spawnPos() const {
	
		return AISpawnPos_;
	
	}

	inline bool WorldGen3DNoise::noiseMakesBlockAt(float x, float y, float z) {

		return noise_gen_.GetNoise(x, y, z) - (y - waterLevel_) * perc_ > threshold_;

	}

}

#endif