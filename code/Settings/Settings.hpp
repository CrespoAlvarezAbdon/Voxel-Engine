#ifndef _VOXELENG_SETTINGS_
#define _VOXELENG_SETTINGS_

#include <string>

namespace VoxelEng {

	class settings {

	public:

		// Constructor.

		/**
		* @brief Class constructor. Loads the settings from the specified file at 'filePath'.
		* @param filePath The path, relative to the executable directory, to the settings file. 
		* Must include the filename and the file extension.
		*/
		settings(const std::string& filePath);


		// Observers.

		/**
		* @brief Maximum number of chunk generation/meshing simultaneous jobs being executed.
		*/
		//const unsigned int MAX_N_CHUNK_SIMULT_TASKS = 8;
		unsigned int maxChunkhreads() const;

		/**
		* @brief Default maximum distance in chunk coordinates for chunks to be computed in the X and Z axes.
		*/
		//const unsigned int DEF_N_CHUNKS_TO_COMPUTE = 20;
		unsigned int chunkXZRenderDistance() const;

		/**
		* @brief Get the window's width.
		*/
		unsigned int windowWidth() const;

		/**
		* @brief Get the window's height.
		*/
		unsigned int windowHeight() const;

	private:

		unsigned int maxChunkThreads_;
		unsigned int chunkXZRenderDistance_;
		unsigned int windowWidth_;
		unsigned int windowHeight_;

	};

	inline unsigned int settings::maxChunkhreads() const {
	
		return maxChunkThreads_;
	
	}

	inline unsigned int settings::chunkXZRenderDistance() const {
	
		return chunkXZRenderDistance_;
	
	}

	inline unsigned int settings::windowWidth() const {
	
		return windowWidth_;
	
	}

	inline unsigned int settings::windowHeight() const {
	
		return windowHeight_;
	
	}

}

#endif