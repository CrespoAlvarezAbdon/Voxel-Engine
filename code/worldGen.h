#ifndef _VOXELENG_WORLDGEN_
#define _VOXELENG_WORLDGEN_
#include <unordered_map>
#include <string>
#include <concepts>
#include <atomic>
#include <random>
#include <limits>
#include "chunk.h"
#include "logger.h"
#include "definitions.h"


namespace VoxelEng {

	// Forward class declaration.

	class chunk;
	class chunkManager;


	/* 
	WARNING: calling worldGen's constructor or the constructor of any class that derives
	from worldGen results in undefined behaviour. Use worldGen::registerGen to create
	a new world generator object from any derived class.
	The 'worldGen' class auto registers itself into the world generator system, providing
	a default world generator.
	*/
	class worldGen {

	public:

		// Initialisers.

		static void init();


		// Observers.

		/*
		Usage notes. Use std::dynamic_cast<> to check if the loaded worldGen corresponds
		to the subclass that is being used in the loaded AI game.
		By default the returned pointer points to an existing built-in 'worldGen' object
		so no need to check if it is returning null.
		*/
		static const worldGen& cSelectedGen();

		static const vec3& playerSpawnPos();

		static bool isGenRegistered(const std::string& name);


		// Modifiers.

		/*
		Register an instance of a 'worldGen' object (or an object from a class that derives from 'worldGen')
		to use it when generating new chunks in levels.
		'T' is the class that either is 'worldGen' or a class that derives from 'worldGen'.
		*/
		template <class T>
		requires std::derived_from<T, worldGen>
		static void registerGenAt(const std::string& genName);

		/*
		Same as registerGenAt() but makes no boundary checks.
		*/
		template <class T>
		requires std::derived_from<T, worldGen>
		static void registerGen(const std::string& genName);

		static void selectGenAt(const std::string& genName);

		/*
		Same as registerGenAt() but makes no boundary checks.
		*/
		static void selectGen(const std::string& genName);

		static worldGen& selectedGen();

		static void unregisterGen(const std::string& genName);

		/*
		Sets a random seed for the selected world generator to use.
		Cannot be changed while in a level.
		*/
		static void setSeed();

		/*
		Sets the specified seed for the selected world generator to use.
		Cannot be changed while in a level.
		*/
		static void setSeed(unsigned int seed);

		/*
		Prepare the selected world generator to generate a new level.
		*/
		static void prepareGen();

		/*
		Fill a chunk's block data according to the selected world generator.
		*/
		static void generate(chunk& chunk);


		// Clean up.

		/*
		Clean up heap memory allocated for this system.
		Note, any extra heap memory allocated by any user-implemented generator
		will have to be handled by the user's code.
		WARNING. The built in generator is also deleted so only use
		this when the program is going to finish.
		*/
		static void cleanUp();

	protected:

		/*
		Attributes.
		*/
		static vec3 playerSpawnPos_;
		static unsigned int seed_;
		static std::random_device RD_;
		static std::mt19937 generator_;
		static std::uniform_int_distribution<unsigned int> uDistribution_;


		/*
		Methods.
		*/

		//Modifiers.
		
		/*
		Any preparations before generating a new level are made here.
		*/
		virtual void prepareGen_() = 0;

		virtual void generate_(chunk& chunk) = 0;

	private:

		static std::atomic<bool> isCreatingAllowed_; // To restrict constructor's use.
		static bool initialised_;
		static std::unordered_map<std::string, worldGen*> generators_;
		static worldGen* selectedGen_,
					   * defaultGen_;
		

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

		generators_.insert({ genName, new T() });

	}

	inline void worldGen::selectGen(const std::string& genName) {

		selectedGen_ = generators_[genName];

	}

	inline void worldGen::prepareGen() {
	
		selectedGen_->prepareGen_();
	
	}

	inline void worldGen::generate(chunk& chunk) {

		selectedGen_->generate_(chunk);
	
	}

	
	// 'defaultWorldGen' class.

	class defaultWorldGen : public worldGen {

	public:

		// Constructors.

		defaultWorldGen();


	protected:

		// Modifiers.

		void prepareGen_();

		void generate_(chunk& chunk);

	private:


	};

	inline defaultWorldGen::defaultWorldGen() {}

}

#endif