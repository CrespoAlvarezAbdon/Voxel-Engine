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
#include <Registry/registryElement.h>

namespace VoxelEng {

	/**
	* @brief Class used to wrap a pointer to an object or basic type variable while storing its type as a string.
	* Once the pointer is passed to the var object, it will take care of it until it goes out of scope. When that happens, it will
	* free the memory allocated in that pointer.
	*/
	class var : public registryElement {

	public:

		// Enumerations.

		enum class varType { UNKNOWN = -1, UBO_OF_MATERIALS };


		// Initialisation.

		/**
		* @brief Initialise the var system.
		*/
		static void init(const std::string& typeName);

		
		// Constructors.

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

		/**
		* @brief Get the pointer to the wrapped object or basic data type variable.
		* @returns The pointer to the wrapped object or basic data type variable.
		*/
		const void* pointer() const;

		/**
		* @brief Get the type of the pointer to the wrapped object
		* or basic data type variable.
		* @returns The type of the pointer to the wrapped object
		* or basic data type variable.
		*/
		varType getVarType() const;


		// Modifiers.

		/**
		* @brief Get the pointer to the wrapped object or basic data type variable.
		* @returns The pointer to the wrapped object or basic data type variable.
		*/
		void* pointer();

	private:

		/*
		Methods.
		*/

		void destroy();


		/*
		Attributes.
		*/

		static bool initialised_;
		static std::string typeName_;

		void* pointer_;
		varType varType_;

	};

	inline var::var(void* pointer, var::varType varType) 
	: pointer_(pointer),
	  varType_(varType)
	{}

	inline const std::string& var::typeName() {
	
		return typeName_;
	
	}

	inline const void* var::pointer() const {
	
		return pointer_;
	
	}

	inline var::varType var::getVarType() const {

		return varType_;

	}

	inline void* var::pointer() {

		return pointer_;

	}

}

#endif