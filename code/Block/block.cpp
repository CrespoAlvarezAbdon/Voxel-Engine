#include "block.h"
#include <stdexcept>
#include <Registry/registries.h>
#include <Graphics/Materials/materials.h>

namespace VoxelEng {
	
	bool block::initialised_ = false;
	std::unordered_map<std::string, block> block::blocks_;
	std::unordered_map<unsigned int, block*> block::blocksIntIDs_;
	std::unordered_set<unsigned int> block::freeBlocksIntIDs_;
	registryInsOrdered<std::string, directionalLight>* block::directionalLightsRegistry_ = nullptr;
	registryInsOrdered<std::string, pointLight>* block::pointLightsRegistry_ = nullptr;
	registryInsOrdered<std::string, spotLight>* block::spotLightsRegistry_ = nullptr;
	block* block::emptyBlock_ = nullptr;
    const std::string block::emptyBlockName_ = "";


	void block::init() {

		if (initialised_)
			logger::errorLog("Block system is already initialised");
		else {
		
			auto it = blocks_.emplace(std::pair<std::string, block>(emptyBlockName_, block(emptyBlockName_, 0, blockOpacity::EMPTYBLOCK, {}, "Default", "")));
			emptyBlock_ = &it.first->second;
			blocksIntIDs_.insert(std::pair<unsigned int, block*>(0, emptyBlock_));

			if (registries::initialised())
			{
				directionalLightsRegistry_ = registries::getInsOrdered("DirectionalLights")->pointer<registryInsOrdered<std::string, directionalLight>>();
				pointLightsRegistry_ = registries::getInsOrdered("PointLights")->pointer<registryInsOrdered<std::string, pointLight>>();
				spotLightsRegistry_ = registries::getInsOrdered("SpotLights")->pointer<registryInsOrdered<std::string, spotLight>>();
			}
			else
				throw std::runtime_error("Chunk manager system needs registries system to be initialised first");

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
		const std::string& material, const std::string& light) {
	
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

			auto it = blocks_.emplace(std::pair<std::string, block>(name, block(name, intID, opacity, textures, material, light)));
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
		const std::string& materialID, const std::string& light)
		: name_(name),
		intID_(intID),
		opacity_(opacity),
		materialIndex_(registries::getCInsOrdered("Materials")->pointer<const registryInsOrdered<std::string, var>>()->getInsIndex(materialID)),
		emittedLightIndex_(0)
	{

		if (textures.size()) {

			for (auto it = textures.begin(); it != textures.end(); it++)
				textures_[it->first] = it->second;

			if (!containsTexture("all"))
				textures_["all"] = textures.begin()->second;

		}
		else
			textures_["all"] = 0;

		// Light is expected to be of format "LIGHTTYPE:LIGHTNAME"
		if (!light.empty()) {

			std::string::size_type delimiterPos = light.find(':');

			if (delimiterPos == std::string::npos)
				throw std::runtime_error("Ill-formated light used by block " + name + ": " + light);
			else {
			
				std::string lightType = light.substr(0, delimiterPos);
				std::string lightName = light.substr(delimiterPos + 1);
				if (lightType == "DirectionalLight")
					throw std::runtime_error("A directional light cannot be assigned to a block");
				else if (lightType == "PointLight") {
				
					// NEXT. EL VAR NO PUEDE BORRAR EL PUNTERO AL LIGHT. 
					emittedLight_.setPointerAndType(pointLightsRegistry_->get(lightName), var::varType::POINTLIGHT);
					emittedLightIndex_ = pointLightsRegistry_->getInsIndex(lightName);
				
				}
				else if (lightType == "SpotLight") {
				
					emittedLight_.setPointerAndType(spotLightsRegistry_->get(lightName), var::varType::SPOTLIGHT);
					emittedLightIndex_ = spotLightsRegistry_->getInsIndex(lightName);
				
				}
				else
					throw std::runtime_error("Unknown light type used by block " + name + ": " + lightType);

			}

		}

	}

}