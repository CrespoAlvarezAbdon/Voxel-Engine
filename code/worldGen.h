/**
* @file worldGen.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title World generator.
* @brief Contains the declaration of the worldGen class that provides
* a basic API for defining custom world generators to create new levels.
*/
#ifndef _VOXELENG_WORLDGEN_
#define _VOXELENG_WORLDGEN_
#include <unordered_map>
#include <string>
#include <concepts>
#include <atomic>
#include <random>
#include <limits>
#include "chunk.h"
#include "definitions.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	//////////////////////////////
	//Forward class declaration.//
	//////////////////////////////

	class chunk;
	class chunkManager;


	/**
	* @brief Provides a basic API for defining custom world generator to create new levels.
	* In order to create one, create a class derived from this one or from another that
	* ultimately has this class as a super class.
	* WARNING. Calling worldGen's constructor or the constructor of any class that derives
	* from worldGen results in undefined behaviour. Use worldGen::registerGen to create
	* a new world generator object from any derived class.
	* The 'worldGen' class auto registers itself into the world generator system, providing
	* a default world generator for performance-testing purposes.
	*/
	class worldGen {

	public:

		// Initialisers.

		/**
		* @brief Initialise the class's static members and allocated any resources
		* that are required on initialisation.
		* The default world generator is registered.
		*/
		static void init();


		// Observers.

		/**
		* @brief Returns the currently loaded and selected world generator.
		* Usage notes. Use std::dynamic_cast<> to check if the loaded worldGen corresponds
		* to the subclass that is being used in the loaded AI game.
		* By default the returned pointer points to an existing built-in 'worldGen' object
		* so no need to check if it is returning null.
		*/
		static const worldGen& cSelectedGen();

		/**
		* @brief Returns the current user spawn position into the world.
		*/
		static const vec3& playerSpawnPos();

		/**
		* @brief Returns true if the specified world generator is registered
		* or false otherwise.
		*/
		static bool isGenRegistered(const std::string& name);

		/**
		* @brief Returns true if the specified world generator is selected
		* or false otherwise or if it is not registered.
		*/
		static bool isGenSelected(const std::string& name);

		/**
		* @brief Returns the current level's seed. Used for adding a controlled randomness
		* factor to world generation.
		*/
		static unsigned int getSeed();

		/**
		* @brief Returns true if the world gen system is initialised or false otherwise.
		*/
		static bool initialised();


		// Modifiers.

		/**
		* @brief Register an instance of a 'worldGen' object (or an object from a class that derives from 'worldGen')
		* to use it when generating new chunks in levels.
		* 'T' is the class that either is 'worldGen' or a class that derives from 'worldGen'.
		*/
		template <class T>
		requires std::derived_from<T, worldGen>
		static void registerGenAt(const std::string& genName);

		/**
		* @brief Same as worldGen::registerGenAt() but makes no boundary checks.
		*/
		template <class T>
		requires std::derived_from<T, worldGen>
		static void registerGen(const std::string& genName);

		/**
		* @brief Sets the specified world generator as current loaded and selected world generator.
		*/
		static void selectGenAt(const std::string& genName);

		/**
		* @brief Same as worldGen::selectGenAt() but makes no boundary checks.
		*/
		static void selectGen(const std::string& genName);

		/**
		* @brief Sets the specified world generator as current loaded and selected world generator.
		*/
		static worldGen& selectedGen();

		/**
		* @brief Deletes and unregisters the specified world generator.
		*/
		static void unregisterGen(const std::string& genName);

		/**
		* @brief Sets a random seed for the selected world generator to use.
		* Cannot be changed while in a level.
		*/
		static void setSeed();

		/**
		* @brief Sets the specified seed for the selected world generator to use.
		*Cannot be changed while in a level.
		*/
		static void setSeed(unsigned int seed);

		/**
		* @brief Prepare the selected world generator to generate a new level.
		*/
		static void prepareGen();

		/**
		* @brief Fill a chunk's block data according to the selected world generator.
		*/
		static void generate(chunk& chunk);


		// Clean up.

		/**
		* @brief Deallocate the resources of the current world generator.
		*/
		static void clear();

		/**
		* @brief Clean up heap memory allocated for this system and deinitialise it.
		* Note, any extra heap memory allocated by any user-implemented generator
		* will have to be handled by the user's code.
		* WARNING. The built-in generator is also deleted.
		*/
		static void reset();

	protected:

		/*
		Attributes.
		*/
		static vec3 playerSpawnPos_;
		static unsigned int seed_;
		static std::random_device RD_;
		static std::mt19937 generator_;
		static std::uniform_int_distribution<unsigned int> uDistribution_;
		static std::uniform_int_distribution<unsigned int>::param_type flatWorldBlockDistribution_;


		/*
		Methods.
		*/

		// Modifiers.
		
		/*
		Any preparations before generating a new level are made here.
		*/
		virtual void prepareGen_() = 0;

		/*
		Chunk generation is done here.
		*/
		virtual void generate_(chunk& chunk) = 0;

		/*
		Method for deallocating and deinitialising anything related to the world generator.
		*/
		virtual void clear_() = 0;

	private:

		static std::atomic<bool> isCreatingAllowed_; // To restrict constructor's use.
		static bool initialised_;
		static std::unordered_map<std::string, worldGen*> generators_;
		static worldGen* selectedGen_,
					   * defaultGen_;
		static const std::string* selectedGenName_;
		
	};

	inline const worldGen& worldGen::cSelectedGen() {

		return *selectedGen_;
	
	}

	inline const vec3& worldGen::playerSpawnPos() {
	
		return playerSpawnPos_;
	
	}

	inline bool worldGen::isGenRegistered(const std::string& name) {
	
		return generators_.find(name) != generators_.cend();
	
	}

	inline bool worldGen::isGenSelected(const std::string& name) {
	
		return *selectedGenName_ == name;
	
	}

	inline unsigned int worldGen::getSeed() {
	
		return seed_;
	
	}

	inline bool worldGen::initialised() {
	
		return initialised_;
	
	}

	inline worldGen& worldGen::selectedGen() {

		return *selectedGen_;

	}

	template <class T>
	requires std::derived_from<T, worldGen>
		void worldGen::registerGenAt(const std::string& genName) {

		if (generators_.find(genName) == generators_.cend())
			registerGen<T>(genName);
		else
			logger::errorLog("Another world generator named " + genName + " is already registered");

	}

	template <class T>
	requires std::derived_from<T, worldGen>
	inline void worldGen::registerGen(const std::string& genName) {

		generators_.insert({ genName, new T(
			block::getBlockC("starminer::coalOre"), block::getBlockC("starminer::ironOre"), 
			block::getBlockC("starminer::goldOre") , block::getBlockC("starminer::diamondOre"),
			block::getBlockC("starminer::grass"), block::getBlockC("starminer::dirt"),
			block::getBlockC("starminer::stone"), block::emptyBlock()) });

	}

	inline void worldGen::selectGen(const std::string& genName) {

		selectedGen_ = generators_[genName];

	}

	inline void worldGen::prepareGen() {
	
		selectedGen_->prepareGen_();
	
	}

	inline void worldGen::generate(chunk& chunk) {
		
		chunk.blockDataMutex().lock();
		selectedGen_->generate_(chunk);
		chunk.blockDataMutex().unlock();
	
	}

	
	// 'defaultWorldGen' class.

	class defaultWorldGen : public worldGen {

	public:

		// Constructors.

		defaultWorldGen(const block& b1, const block& b2, const block& b3);


	protected:

		// Modifiers.

		void prepareGen_();

		void generate_(chunk& chunk);

		void clear_();

	private:

		const block& b1_;
		const block& b2_;
		const block& b3_;

	};

	inline defaultWorldGen::defaultWorldGen(const block& b1, const block& b2, const block& b3)
	: b1_(b1), b2_(b2), b3_(b3)
	{}

	inline void defaultWorldGen::clear_()
	{}

}

#endif