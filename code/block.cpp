#include "block.h"

#include "Registry/registries.h"

namespace VoxelEng {
	
	bool block::initialised_ = false;
	std::unordered_map<std::string, block> block::blocks_;
	std::unordered_map<unsigned int, block*> block::blocksIntIDs_;
	std::unordered_set<unsigned int> block::freeBlocksIntIDs_;
	block* block::emptyBlock_ = nullptr;
    const std::string block::emptyBlockName_ = "";


	void block::init() {

		if (initialised_)
			logger::errorLog("Block system is already initialised");
		else {
		
			auto it = blocks_.emplace(std::pair<std::string, block>(emptyBlockName_, block(emptyBlockName_, 0, blockOpacity::EMPTYBLOCK, {}, "Default")));
			emptyBlock_ = &it.first->second;
			blocksIntIDs_.insert(std::pair<unsigned int, block*>(0, emptyBlock_));

			initialised_ = true;

		}
	}

    block& block::getBlockC(const std::string& name) {

		if (blocks_.contains(name))
			return blocks_[name];
		else
			logger::errorLog("Block " + name + " is not registered");

	}

	block& block::getBlockC(unsigned int ID) {

		if (blocksIntIDs_.contains(ID))
			return *blocksIntIDs_[ID];
		else
			logger::errorLog("Block with ID " + std::to_string(ID) + " is not registered");

	}

	unsigned int block::textureID(const std::string& textureName, bool returnDefault) const {

		if (textures_.contains(textureName))
			return textures_.at(textureName);
		else {
			if (returnDefault)
				return textures_.at("all");
			else
				logger::errorLog("The specified texture " + textureName + " is not associated with block " + name_);
		}

	}

	void block::registerBlock(const std::string& name, blockOpacity opacity,
		const std::initializer_list<std::pair<std::string, unsigned int>>& textures,
		const std::string& material) {
	
		if (blocks_.contains(name))
			logger::errorLog("Block " + name + " already registered");
		else {
		
			unsigned int intID = 0;
			if (freeBlocksIntIDs_.empty())
				intID = blocks_.size() + 1; // intID 0 is reserved for empty block.
			else {

				intID = *freeBlocksIntIDs_.begin();
				freeBlocksIntIDs_.erase(intID);

			}

			auto it = blocks_.emplace(std::pair<std::string, block>(name, block(name, intID, opacity, textures, material)));
			blocksIntIDs_.emplace(std::pair<unsigned int, block*>(intID, &it.first->second));
			
		}
		
	}

	void block::unregisterBlock(const std::string& name) {

		if (blocks_.contains(name)) {
		
			if (name == emptyBlockName_)
				logger::errorLog("It is unsupported to unregister the empty block");
			else {
			
				unsigned int intID = blocks_[name].intID();

				blocks_.erase(name);
				blocksIntIDs_.erase(intID);
				freeBlocksIntIDs_.insert(intID);
			
			}
				
		}
		else
			logger::errorLog("Block " + name + " is not registered");

	}

	bool block::operator==(const block& b2) const {
	
		return name_ == b2.name_;
	
	}

	bool block::operator!=(const block& b2) const {

		return name_ != b2.name_;

	}

	void block::reset() {

		if (initialised_) {

			blocks_.clear();
			blocksIntIDs_.clear();
			freeBlocksIntIDs_.clear();
			emptyBlock_ = nullptr;

			initialised_ = false;

		}
		else
			logger::errorLog("Block system is not initialised");

	}

	block::block(const std::string& name, unsigned int intID, blockOpacity opacity,
		const std::initializer_list<std::pair<std::string, unsigned int>>& textures,
		const std::string& materialID)
		: name_(name),
		intID_(intID),
		opacity_(opacity),
		materialIndex_(registries::materials().getInsIndex(materialID))
	{

		if (textures.size()) {

			for (auto it = textures.begin(); it != textures.end(); it++)
				textures_[it->first] = it->second;

			if (!containsTexture("all"))
				textures_["all"] = textures.begin()->second;

		}
		else
			textures_["all"] = 0;

	}

}