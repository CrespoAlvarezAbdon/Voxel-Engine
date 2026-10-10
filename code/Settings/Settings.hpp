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
		* @brief Get the maximum number of chunk generation/meshing simultaneous jobs being executed.
		* @returns The maximum number of chunk generation/meshing simultaneous jobs being executed.
		*/
		unsigned int maxChunkThreads() const;

		/**
		* @brief Get the maximum distance in chunk coordinates for chunks to be computed in the X and Z axes.
		* @returns The maximum distance in chunk coordinates for chunks to be computed in the X and Z axes.
		*/
		unsigned int chunkXZRenderDistance() const;

		/**
		* @brief Get whether the chunk dynamic lighting system is used or not.
		* @returns Whether the chunk dynamic lighting system is used (true) or not (false).
		*/
		bool useDynamicLighting() const;

		/**
		* @brief Get maximum distance for chunks with enabled dynamic lighting.
		* @returns The maximum distance for chunks with enabled dynamic lighting.
		*/
		unsigned int maxDistanceDynamicLightChunks() const;

		/**
		* @brief Get the engine window's width.
		* @returns The engine window's width.
		*/
		unsigned int windowWidth() const;

		/**
		* @brief Get the engine window's height.
		* @returns The engine window's height.
		*/
		unsigned int windowHeight() const;

	private:

		unsigned int maxChunkThreads_;
		unsigned int chunkXZRenderDistance_;
		bool useDynamicLighting_;
		unsigned int maxDistanceDynamicLightChunks_;
		unsigned int windowWidth_;
		unsigned int windowHeight_;

	};

	inline unsigned int settings::maxChunkThreads() const {
	
		return maxChunkThreads_;
	
	}

	inline unsigned int settings::chunkXZRenderDistance() const {

		return chunkXZRenderDistance_;

	}

	inline bool settings::useDynamicLighting() const {
	
		return useDynamicLighting_;
	
	}

	inline unsigned int settings::maxDistanceDynamicLightChunks() const {
	
		return maxDistanceDynamicLightChunks_;
	
	}

	inline unsigned int settings::windowWidth() const {
	
		return windowWidth_;
	
	}

	inline unsigned int settings::windowHeight() const {
	
		return windowHeight_;
	
	}

}

#endif