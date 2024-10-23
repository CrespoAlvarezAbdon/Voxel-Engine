/**
* @file var.h
* @version 1.0
* @date 23/10/2024
* @author Abdon Crespo Alvarez
* @title Var.
* @brief Contains the declaration of the 'varBase' class.
*/
#ifndef _VOXELENG_VARBase_
#define _VOXELENG_VARBase_

#include <string>
#include <Registry/registryElement.h>

namespace VoxelEng {

	/**
	* @brief Base class for var and varRef classes that holds common elements between those them.
	*/
	class varBase : public registryElement {

	public:

		// Enumerations.

		enum class varType { UNKNOWN = -1, NOTYPE = 0,
			UBO_OF_MATERIALS, UBO_OF_DIRECTIONALLIGHTS, UBO_OF_POINTLIGHTS, UBO_OF_SPOTLIGHTS,
			SSBO_OF_LIGHTINSTANCES,
			REGISTRYINSORDERED_OF_STRINGS_MATERIALS,
			REGISTRYINSORDERED_OF_STRINGS_DIRECTIONALLIGHTS, REGISTRYINSORDERED_OF_STRINGS_POINTLIGHTS, REGISTRYINSORDERED_OF_STRINGS_SPOTLIGHTS,
			REGISTRY_OF_STRINGS_VARS,
			DIRECTIONALLIGHT, POINTLIGHT, SPOTLIGHT
		};


		// Initialisation.

		/**
		* @brief Initialise the var system.
		*/
		static void init(const std::string& typeName);

		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

		/**
		* @brief Get whether the var is considered a null var object or not.
		* @returns Whether the var is considered a null var object (true) or not (false).
		*/
		bool isNull() const;

		/**
		* @brief Get the pointer to the wrapped object or basic data type variable.
		* @returns The pointer to the wrapped object or basic data type variable.
		*/
		template <typename T>
		const T* pointer() const;

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
		template <typename T>
		T* pointer();

		/**
		* @brief Set the var's pointer.
		* @param newPointer The var's new pointer.
		*/
		void pointer(void* newPointer);

		/**
		* @brief Set the var's type for the wrapped object.
		* @param type The var's type for the wrapped object.
		*/
		void setVarType(varType type);

		/**
		* @brief Set the var's pointer and type for the wrapped object.
		* @param newPointer The var's new pointer.
		* @param type The var's type for the wrapped object.
		*/
		void setPointerAndType(void* newPointer, varType type);

	protected:

		/*
		Methods.
		*/

		varBase();
		varBase(void* pointer, varType varType);

		/*
		Attributes.
		*/

		void* pointer_;
		varType varType_;

	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static std::string typeName_;

	};

	inline const std::string& varBase::typeName() {
	
		return typeName_;
	
	}

	inline bool varBase::isNull() const {
	
		return pointer_ == nullptr && varType_ == varType::NOTYPE;
	
	}

	template <typename T>
	inline const T* varBase::pointer() const {
	
		return static_cast<const T*>(pointer_);
	
	}

	inline varBase::varType varBase::getVarType() const {

		return varType_;

	}

	template <typename T>
	inline T* varBase::pointer() {

		return static_cast<T*>(pointer_);

	}

	inline void varBase::pointer(void* newPointer) {
	
		pointer_ = newPointer;
	
	}

	inline void varBase::setVarType(varType type) {
	
		varType_ = type;
	
	}

	inline void varBase::setPointerAndType(void* newPointer, varType type) {

		pointer(newPointer);
		setVarType(type);

	}

	inline varBase::varBase() 
	: pointer_(nullptr),
	  varType_(varType::NOTYPE)
	{}

	inline varBase::varBase(void* pointer, varBase::varType varType) 
	: pointer_(pointer),
	  varType_(varType)
	{}

}

#endif