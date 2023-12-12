#ifndef _VOXELENG_BLOCK_
#define _VOXELENG_BLOCK_

#include <string>
#include <unordered_map>
#include "logger.h"


namespace VoxelEng {

	/**
	* @brief A block is the minimal unit of terrain in this engine. This class is the base
	* for the representation of any type of blocks registered and supported by this engine.
	*/
	class block { // TODO. HAY QUE REUSAR LOS OBJETOS BLOCK.
				  // TODO. DOC DE ESTE .H

	public:

		// Initialisers.

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

		static block& getBlockC(const std::string& name);

		static bool isBlockRegistered(const std::string& name);

		static block* emptyBlockP();

		static block& emptyBlock();

		unsigned int textureID() const; // TODO. FILL TEXTURE ID WITH PROPER DYNAMIC ATLAS BUILDING.

		bool isEmptyBlock() const;

		const std::string& name() const;


		// Modifiers.

		static void registerBlock(const std::string& name, unsigned int textureID);

		static void unregisterBlock(const std::string& name);


		// Destructors.

		~block();


		// Deinitialisers.

		static void cleanUp();

	private:

		// Constructors.
		block(const std::string& name, unsigned int textureID);


		// Attributes.

		static std::unordered_map<std::string, block> blocks_;
		static block* emptyBlock_;
		static const std::string emptyBlockName_;

		const std::string name_;

		unsigned int textureID_; // 0 means no texture assigned.

	};

	inline block::block()
	: textureID_(0),
	  name_("")
	{}

	inline block::block(const std::string& name, unsigned int textureID)
    : name_(name), textureID_(textureID)
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

	inline void block::cleanUp() {

		blocks_.clear();
		emptyBlock_ = nullptr;

	}

}

#endif