#ifndef _VOXELENG_REGISTRY_
#define _VOXELENG_REGISTRY_

#include <concepts>
#include <unordered_map>
#include <string>
#include "RegistryElement.h"

namespace VoxelEng {

	/////////////
	//Concepts.//
	/////////////

	template<typename T>
	concept DerivedFromRegistryElement = std::derived_from<RegistryElement, T>;


	////////////
	//Classes.//
	////////////

	/**
	* @brief A registry is a mapping of keys of type (or derive from) KeyT to elements of type (or derive from) T.
	* There cannot be two elements with the same key.
	* Elements can be created, read, updated, and deleted and objects of this class will throw exceptions
	* if the operations are not executed properly. For example, if an element of a non-registered key is specified
	* to be deleted, a std::runtime_error will be thrown.
	*/
	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	class registry {

	public:

		// Constructors.

		registry(const std::string& Tname);


		// Destructors.

		~registry();


		// Observers.

		const T& get(const KeyT& key);

		T& set(const KeyT& key);


		// Modifiers.



		void insert(const KeyT& key);

		void erase(const KeyT& key);

	private:

		std::unordered_map<KeyT, T> elements_;
		std::string Tname_;

	};

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	registry<KeyT, T>::registry(const std::string& Tname)
	: Tname_(Tname)
	{}

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	registry<KeyT, T>::~registry() 
	{}

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	const T& registry<KeyT, T>::get(const KeyT& key) {
	
	
	
	}

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	T& registry<KeyT, T>::set(const KeyT& key) {
	
	
	
	}

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	void registry<KeyT, T>::insert(const KeyT& key) {
	
		if (elements_.contains(key))
			logger::errorLog("The " + Tname + " " + name + " is already registered.");
		else
			elements_[name] = T(ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB, shininess);
	
	}

	template <typename KeyT, typename T>
	requires DerivedFromRegistryElement<T>
	void registry<KeyT, T>::erase(const KeyT& key);

}

#endif