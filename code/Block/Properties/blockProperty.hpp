#ifndef _VOXELENG_BLOCK_PROPERTY_
#define _VOXELENG_BLOCK_PROPERTY_

namespace VoxelEng {

	class blockProperty {

	public:

		// Initialisers.

		/**
		* @brief Initialise the block property system.
		*/
		static void init();


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		blockProperty();


		// Observers.

		/**
		* @brief Get the empty block property object.
		* @returns The empty block property object.
		*/
		static const blockProperty& emptyProp();


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		virtual ~blockProperty();


		// Clean up.

		/**
		* @brief Resets the block system.
		*/
		static void reset();

	private:

		static bool initialised_;
		static blockProperty* emptyProp_;

	};

	inline blockProperty::blockProperty() 
	{}

	inline const blockProperty& blockProperty::emptyProp() {
	
		return *emptyProp_;
	
	}

	inline blockProperty::~blockProperty() 
	{}

}

#endif