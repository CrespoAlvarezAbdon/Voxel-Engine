/**
* @file block.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Block
* @brief A block is the minimal (for now) unit of terrain in the engine.
*/
#ifndef _VOXELENG_BLOCK_
#define _VOXELENG_BLOCK_

#include <initializer_list>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include "logger.h"


namespace VoxelEng {

	//////////
	//Enums.//
	//////////

	enum class blockOpacity {EMPTYBLOCK = -1, FULLTRANSPARENT = 0, TRANSLUCENTBLOCK = 1, OPAQUEBLOCK = 2};


	////////////
	//Classes.//
	////////////

	/**
	* @brief A block is the minimal (for now) unit of terrain in the engine. This class is the base
	* for the representation of any type of blocks registered and supported by this engine.
	*/
	class block { // NEXT. METER EL REGISTRO DE IDS NUMERICAS

	public:

		// Initialisers.

		/**
		* @brief Initialise the block system.
		*/
		static void init();


		// Constructors.

		/**
		* @brief Default constructor.
		* WARNING. Do NOT create new types of blocks with this constructor. Use the method
		* block::registerBlock to do that properly.
		*/
		block();


		// Observers.

		/**
		* @brief Get the specified block.
		*/
		static block& getBlockC(const std::string& name);

		/**
		* @brief Get the specified block.
		*/
		static block& getBlockC(unsigned int ID);

		/**
		* @brief Returns true if the specified block is registered or
		* false otherwise.
		*/
		static bool isBlockRegistered(const std::string& name);

		/**
		* @brief Returns the empty block.
		*/
		static block* emptyBlockP();

		/**
		* @brief Returns the empty block.
		*/
		static block& emptyBlock();

		/**
		* @brief Returns true if the block system is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the numerical ID assigned to this block.
		*/
		unsigned int intID() const;

		/**
		* @brief Returns the specified texture ID of this block. A block can have many textures assigned.
		* Each one of them is identified by
		* Said ID is used to locate the block's texture inside the texture atlas.
		*/
		unsigned int textureIDAt(const std::string& textureName) const; // TODO. FILL TEXTURE ID WITH PROPER DYNAMIC ATLAS BUILDING.

		/**
		* @brief Returns the specified texture ID of this block. A block can have many textures assigned.
		* Each one of them is identified by
		* Said ID is used to locate the block's texture inside the texture atlas.
		* Returns the default texture named "all" assigned to each block or throws an exception if returnDefault is true or false respectively
		* and the specified texture is not associated with this block.
		*/
		unsigned int textureID(const std::string& textureName, bool returnDefault = true) const;

		/**
		* @brief Returns true if the specified texture is associated with the block or false otherwise.
		*/
		bool containsTexture(const std::string& textureName);

		/**
		* @brief Returns true if this block is the empty block or
		* false otherwise.
		*/
		bool isEmptyBlock() const;

		/**
		* @brief Returns the block's namespaced ID.
		*/
		const std::string& name() const;

		/**
		* @brief Returns the block's opacity.
		*/
		blockOpacity opacity() const;

		/**
		* @brief Get the block's material ID.
		*/
		unsigned int getMaterialIndex() const;


		// Modifiers.

		/**
		* @brief Registers a block into the system.
		* Its numerical ID is assigned automatically.
		*/
		static void registerBlock(const std::string& name, blockOpacity opacity, 
			const std::initializer_list<std::pair<std::string, unsigned int>>& textures,
			const std::string& material = "Default");

		/**
		* @brief Unregisters a block.
		*/
		static void unregisterBlock(const std::string& name);


		// Operators.

		/**
		* @brief Returns whether the two given blocks are equal (true) or not (false).
		* @param b2 The left operand of the == operation.
		*/
		bool operator==(const block& b2) const;

		/**
		* @brief Returns whether the two given blocks are different (true) or not (false).
		* @param b2 The left operand of the == operation.
		*/
		bool operator!=(const block& b2) const;


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		~block();


		// Clean up.

		/**
		* @brief Resets the block system.
		*/
		static void reset();

	private:

		/*
		Methods.
		*/

		// Constructors.

		block(const std::string& name, unsigned int intID, blockOpacity opacity,
			  const std::initializer_list<std::pair<std::string, unsigned int>>& texture,
			  const std::string& material);


		/*
		Attributes.
		*/

		static bool initialised_;
		static std::unordered_map<std::string, block> blocks_;
		static std::unordered_map<unsigned int, block*> blocksIntIDs_;
		static std::unordered_set<unsigned int> freeBlocksIntIDs_;
		static block* emptyBlock_;
		static const std::string emptyBlockName_;

		const std::string name_;
		const unsigned int intID_;
		blockOpacity opacity_;
		std::unordered_map<std::string, unsigned int> textures_;
		unsigned int materialIndex_;

	};

	inline block::block()
	: name_(""),
	  intID_(0),
	  opacity_(blockOpacity::OPAQUEBLOCK),
	  materialIndex_(0)
	{}

	inline bool block::isBlockRegistered(const std::string& name) {

		return blocks_.contains(name);

	}
	
	inline block* block::emptyBlockP() {

		return emptyBlock_;

	}

	inline block& block::emptyBlock() {
	
		return *emptyBlock_;
	
	}

	inline bool block::initialised() {
	
		return initialised_;
	
	}

	inline unsigned int block::intID() const {

		return intID_;

	}

	inline unsigned int block::textureIDAt(const std::string& name) const {
	
		return textures_.at(name);
	
	}

	inline bool block::containsTexture(const std::string& textureName) {
	
		return textures_.contains(textureName);
	
	}

	inline bool block::isEmptyBlock() const {
	
		return this == emptyBlock_;
	
	}

	inline const std::string& block::name() const {
	
		return name_;
	
	}

	inline blockOpacity block::opacity() const {
	
		return opacity_;
	
	}

	inline unsigned int block::getMaterialIndex() const {
	
		return materialIndex_;
	
	}

	inline block::~block() {}

}

#endif