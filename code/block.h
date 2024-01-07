#ifndef _VOXELENG_BLOCK_
#define _VOXELENG_BLOCK_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "logger.h"


namespace VoxelEng {

	/**
	* @brief A block is the minimal unit of terrain in this engine. This class is the base
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


		// Operators.

		/**
		* @brief As each block must have an unique name. In consequence, the == operator
		* only checks if the names of the two operands are equal or not.
		*/
		bool operator==(const block& b) const;


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
		* @brief Returns the numerical ID assigned to this block.
		*/
		unsigned int intID() const;

		/**
		* @brief Returns the texture ID assigned to this block.
		* Said ID is used to locate the block's texture inside the texture atlas.
		*/
		unsigned int textureID() const; // TODO. FILL TEXTURE ID WITH PROPER DYNAMIC ATLAS BUILDING.

		/**
		* @brief Returns true if this block is the empty block or
		* false otherwise.
		*/
		bool isEmptyBlock() const;

		/**
		* @brief Returns the block's namespaced ID.
		*/
		const std::string& name() const;


		// Modifiers.

		/**
		* @brief Registers a block into the system.
		* Its numerical ID is assigned automatically.
		*/
		static void registerBlock(const std::string& name, unsigned int textureID);

		/**
		* @brief Unregisters a block.
		*/
		static void unregisterBlock(const std::string& name);


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

		block(const std::string& name, unsigned int indID, unsigned int textureID);


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
		unsigned int textureID_; // 0 means no texture assigned.

	};

	inline block::block()
	: name_(""),
	  intID_(0),
	  textureID_(0)
	{}

	inline block::block(const std::string& name, unsigned int intID, unsigned int textureID)
    : name_(name),
	  intID_(intID),
	  textureID_(textureID)
	{}

	inline bool block::operator==(const block& b) const {
	
		return this == &b;
	
	}

	inline bool block::isBlockRegistered(const std::string& name) {

		return blocks_.contains(name);

	}
	
	inline block* block::emptyBlockP() {

		return emptyBlock_;

	}

	inline block& block::emptyBlock() {
	
		return *emptyBlock_;
	
	}

	inline unsigned int block::intID() const {

		return intID_;

	}

	inline unsigned int block::textureID() const {
	
		return textureID_;
	
	}

	inline bool block::isEmptyBlock() const {
	
		return this == emptyBlock_;
	
	}

	inline const std::string& block::name() const {
	
		return name_;
	
	}

	inline block::~block() {}

}

#endif