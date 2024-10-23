/**
* @file var.h
* @version 1.0
* @date 23/10/2024
* @author Abdon Crespo Alvarez
* @title Var.
* @brief Contains the declaration of the 'var' class.
*/
#ifndef _VOXELENG_VAR_
#define _VOXELENG_VAR_

#include <string>
#include <Utilities/VarBase/varBase.h>

namespace VoxelEng {

	/**
	* @brief Class used to wrap a pointer to an object or basic type variable while storing its type as a string.
	* Once the pointer is passed to the var object, it will take care of it until it goes out of scope. When that happens, it will
	* free the memory allocated in that pointer.
	*/
	class var : public varBase {

	public:

		// Initialisation.

		/**
		* @brief Initialise the var system.
		*/
		static void init(const std::string& typeName);

		
		// Constructors.

		/**
		* @brief Class default constructor. Constructs a NULL var. Check with var::isNull().
		*/
		var();

		/**
		* @brief Class constructor.
		* @param pointer Pointer to the object or basic type variable to wrap.
		* @param varType The type of the object or basic type variable to wrap.
		*/
		var(void* pointer, varType varType);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~var();

	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static std::string typeName_;

	};

	inline var::var() 
	: varBase()
	{}

	inline var::var(void* pointer, var::varType varType) 
	: varBase(pointer, varType)
	{}

	inline const std::string& var::typeName() {
	
		return typeName_;
	
	}

}

#endif