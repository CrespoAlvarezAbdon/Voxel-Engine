/**
* @file var.h
* @version 1.0
* @date 23/10/2024
* @author Abdon Crespo Alvarez
* @title Var.
* @brief Contains the declaration of the 'varRef' class.
*/
#ifndef _VOXELENG_VARREF_
#define _VOXELENG_VARREF_

#include <string>
#include <Utilities/VarBase/varBase.h>

namespace VoxelEng {

	/**
	* @brief Class used to wrap a pointer to an object or basic type variable while storing its type as a string.
	* Unlike the var class, once the pointer is passed to the var object, it will NOT take care of it until it goes out of scope. So freeing up resources is left
	* up to the user.
	*/
	class varRef : public varBase {

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
		varRef();

		/**
		* @brief Class constructor.
		* @param pointer Pointer to the object or basic type variable to wrap.
		* @param varType The type of the object or basic type variable to wrap.
		*/
		varRef(void* pointer, varType varType);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static std::string typeName_;

	};

	inline varRef::varRef() 
	: varBase()
	{}

	inline varRef::varRef(void* pointer, varRef::varType varType) 
	: varBase(pointer, varType)
	{}

	inline const std::string& varRef::typeName() {

		return typeName_;

	}

}

#endif