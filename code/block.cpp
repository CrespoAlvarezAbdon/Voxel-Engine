#include "block.h"
#include "logger.h"


namespace VoxelEng {
	
	std::unordered_map<std::string, block> block::blocks_;
	block* block::emptyBlock_ = nullptr;
    const std::string block::emptyBlockName_ = "";


	void block::init() {

		blocks_.emplace(std::pair<std::string, block>(emptyBlockName_, block (emptyBlockName_, 0)));
		emptyBlock_ = &blocks_[""];
	
	}

    block& block::getBlockC(const std::string& name) {

		if (blocks_.contains(name))
			return blocks_[name];
		else
			logger::errorLog("Block " + name + " is not registered");

	}

	void block::registerBlock(const std::string& name, unsigned int textureID) {
	
		if (blocks_.contains(name))
			logger::errorLog("Block " + name + " already registered");
		else
			blocks_.emplace(std::pair<std::string, block>(name, block(name, textureID)));

	}

	void block::unregisterBlock(const std::string& name) {

		if (blocks_.contains(name)) {
		
			if (name == emptyBlockName_)
				logger::errorLog("It is unsupported to unregister the empty block");
			else
				blocks_.erase(name);
		
		}
		else
			logger::errorLog("Block " + name + " is not registered");

	}

}