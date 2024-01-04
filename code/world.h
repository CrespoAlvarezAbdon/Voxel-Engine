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

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "chunk.h" // IGUAL ESTO DA PROBLEMAS.
#include "database.h"
#include "definitions.h"
#include "graphics.h"
#include "vec.h"

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
	typedef color skybox;


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
		* @brief Returns the maximun distance that any entity (including players) can be from
		* the center of the X axis on finite worlds.
		* If the returned distance is 0, it means that there is no maximun distance on said axis.
		*/
		static unsigned int maxDistanceX();

		/**
		* @brief Returns the maximun distance that any entity (including players) can be from
		* the center of the Y axis on finite worlds.
		*/
		static unsigned int maxDistanceY();

		/**
		* @brief Returns the maximun distance that any entity (including players) can be from
		* the center of the Z axis on finite worlds.
		*/
		static unsigned int maxDistanceZ();


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
		* Effectively will call world::processGlobalTickFunctions() and the entity tick functions counterpart of
		* this method.
		*/
		static void processWorldTicks();

		/**
		* @brief Method to provide the capability of processing the global ticks of the world
		* to a worker thread that is not created inside this method.
		*/
		static void processGlobalTickFunctions();

		/**
		* @brief 
		*/
		static void saveAll();

		static void createSaveDirectory(unsigned int slot);


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
		static unsigned int maxDistanceX_,
							maxDistanceY_,
							maxDistanceZ_;
		static std::unordered_map<std::string, tickFunc> globalTickFunctions_;
		static std::unordered_set<std::string> activeTickFunctions_;
		static std::mutex tickFunctionsMutex_;
		static unsigned int currentWorldSlot_;
		static std::string currentWorldPath_;
		static database* regions_;


		/*
		Methods.
		*/

		static void saveMainData_();

		static void saveAllChunks_();

		static void saveChunk_(chunk* c);
	
	};

	inline unsigned int world::maxDistanceX() {

		return maxDistanceX_;

	}

	inline unsigned int world::maxDistanceY() {

		return maxDistanceY_;

	}

	inline unsigned int world::maxDistanceZ() {

		return maxDistanceZ_;

	}

	inline void world::setSkybox(const skybox& background) {

		#if GRAPHICS_API == OPENGL

			glClearColor(background.red() / 255.0f, background.green() / 255.0f, background.blue() / 255.0f, background.alpha());

		#else

			

		#endif
	
	}

}

#endif