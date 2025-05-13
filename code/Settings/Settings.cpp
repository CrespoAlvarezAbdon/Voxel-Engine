#include "Settings.hpp"

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	settings::settings(const std::string& filePath)
	: maxChunkThreads_(0), chunkXZRenderDistance_(0) {
	
		if (std::filesystem::exists(filePath)) {
		
			std::ifstream file(filePath);
			nlohmann::json settingsJSON = nlohmann::json::parse(file);
			maxChunkThreads_ = settingsJSON["maxChunkThreads"];
			chunkXZRenderDistance_ = settingsJSON["chunkXZRenderDistance"];
		
		}
		else
			logger::errorLog("Settings not found at " + filePath);
	
	}

}