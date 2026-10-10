#include "Settings.hpp"

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	settings::settings(const std::string& filePath)
	: maxChunkThreads_(0),
	chunkXZRenderDistance_(0),
	useDynamicLighting_(false),
	maxDistanceDynamicLightChunks_(0),
	windowWidth_(0),
	windowHeight_(0) {
	
		if (std::filesystem::exists(filePath)) {
		
			std::ifstream file(filePath);
			nlohmann::json settingsJSON = nlohmann::json::parse(file);
			maxChunkThreads_ = settingsJSON["maxChunkThreads"];
			chunkXZRenderDistance_ = settingsJSON["chunkXZRenderDistance"];
			useDynamicLighting_ = settingsJSON["useDynamicLighting"];
			maxDistanceDynamicLightChunks_ = settingsJSON["maxDistanceDynamicLightChunks"];
			windowWidth_ = settingsJSON["windowWidth"];
			windowHeight_ = settingsJSON["windowHeight"];
		
		}
		else
			logger::errorLog("Settings not found at " + filePath);
	
	}

}