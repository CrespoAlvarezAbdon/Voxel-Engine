/**
* @file world.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title World.
* @brief Contains the declaration of the world class, used to configure various
* global aspects of the currently loaded level as well as some additional
* related types.
*/
#ifndef _VOXELENG_VOXELENGWORLD_
#define _VOXELENG_VOXELENGWORLD_

#include <array>
#include <filesystem>
#include <fstream>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <atomicRecyclingPool.h>
#include <database.h>
#include <definitions.h>
#include <vec.h>
#include <Chunk/chunk.h>
#include <Graphics/graphics.h>
#include <Graphics/Lighting/Lights/LightInstance/lightInstance.h>
#include <Threading/threadPool.hpp>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif


namespace VoxelEng {

	/////////////////////
	//Type definitions.//
	/////////////////////

	/**
	* @brief The plain colour that the sky that boxes the entire level.
	*/
	typedef vec4 skybox;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Worlds are the greatest level of terrain organization in the engine and are a synonym of level. This class
	* provides capalities that apply globally to an entire world in this engine.
	*/
	class world {
	
	public:

		// Initialisers.

		/**
		* @brief Initialise the class's static members and allocated any resources
		* that are required on initialisation.
		*/
		static void init();


		// Observers.

		/**
		* @brief Get whether the world system is initialised or not.
		* @returns Whether the world system is initialised (true) or not (false).
		*/
		static bool initialised();

		/**
		* @brief Get whether the specified chunkPos corresponds to a chunk that has been
		* previously saved to the currently selected world or not.
		* @returns Whether the specified chunkPos corresponds to a chunk that has been
		* previously saved to the currently selected world (true) or not (false).
		*/
		static bool isSaved(const ivec3& chunkPos);

		/**
		* @brief Get the directional light instance corresponding to the world's sun.
		* @returns The directional light instance corresponding to the world's sun.
		*/
		static const lightInstance& sunLightInstanceC();


		// Modifiers.

		/**
		* @brief Set the level's skybox.
		*/
		static void setSkybox(const skybox& skybox);

		/**
		* @brief Add a global tick function to the level.
		*/
		static void addGlobalTickFunction(const std::string& name, tickFunc func, bool active = true);

		/**
		* @brief Change the active state of a global tick function registered in the level.
		*/
		static void changeStateGlobalTickFunction(const std::string& name);

		/**
		* @brief Change the active state of a global tick function registered in the level.
		*/
		static void changeStateGlobalTickFunction(const std::string& name, bool active);

		/**
		* @brief Delete a global tick function registered in the level.
		*/
		static void deleteGlobalTickFunction(const std::string& name);

		/**
		* @brief Method to provide the capability of processing the global and entity ticks of the world
		* to a worker thread that is not created inside this method.
		*/
		static void processWorldTicks();

		/**
		* @brief Save all data related to the currently opened world.
		*/
		static void saveAll();

		/**
		* @brief Save general information about the currently opened world such
		* as current player position, the world's seed...
		*/
		static void saveMainData();

		/**
		* @brief Save the specific chunk into disk according to the currently used level slot.
		*/
		static void saveChunk(chunk* c);

		/**
		* @brief Load the chunk's serialized data according to the currently used level slot.
		*/
		static std::string loadChunk(const ivec3& chunkPos);

		/**
		* @brief Load general information about the currently opened world such
		* as current player position, the world's seed...
		*/
		static void loadMainData();

		/**
		* @brief Setup the save directory in the currently selected save slot.
		*/
		static void setupSaveDirectory();

		/**
		* @brief Remove ALL the contents saved in the currently selected level slot.
		* Does nothing if the directory for that slot does not exist.
		*/
		static void clearSlot();

		/**
		* @brief Unloads the currently loaded world. Throws exception if no world is loaded.
		*/
		static void unloadWorld();

		/**
		* @brief Get the directional light instance corresponding to the world's sun.
		* @returns The directional light instance corresponding to the world's sun.
		*/
		static lightInstance& sunLightInstance();


		// Clean up.

		/**
		* @brief Deinitialise the class's static members and free any resources allocated by them.
		*/
		static void reset();

	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static std::mutex tickFunctionsMutex_;
		static std::unordered_map<std::string, tickFunc> globalTickFunctions_;
		static std::unordered_set<std::string> activeTickFunctions_;

		static unsigned int currentWorldSlot_;
		static std::string currentWorldPath_;
		static database* regions_;
		static lightInstance sunLightInstance_;


		/*
		Methods.
		*/

		static void saveAllChunks_();

		static void processGlobalTickFunctions_();

	};

	inline bool world::initialised() {
	
		return initialised_;
	
	}

	inline const lightInstance& world::sunLightInstanceC() {
	
		return sunLightInstance_;
	
	}

	inline void world::setSkybox(const skybox& background) {

		#if GRAPHICS_API == OPENGL

			glClearColor(background.x / 255.0f, background.y / 255.0f, background.z / 255.0f, background.w);

		#else

			

		#endif
	
	}

	inline lightInstance& world::sunLightInstance() {

		return sunLightInstance_;

	}

}

#endif