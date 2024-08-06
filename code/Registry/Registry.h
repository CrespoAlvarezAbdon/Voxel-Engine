#ifndef _VOXELENG_REGISTRY_
#define _VOXELENG_REGISTRY_

#include <any>
#include <concepts>
#include <functional>
#include <unordered_map>
#include <string>
#include "RegistryElement.h"
#include "../logger.h"


namespace VoxelEng {

	/////////////
	//Concepts.//
	/////////////

	template<typename T>
	concept derivedFromRegistryElement = std::derived_from<registryElement, T>;


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
	requires derivedFromRegistryElement<T>
	class registry {

	public:

		// Types.

		using FactoryFunc = std::function<std::unique_ptr<T>(std::any)>;


		// Constructors.

		registry(FactoryFunc factory);


		// Destructors.

		~registry();


		// Observers.

		const T& get(const KeyT& key);

		T& set(const KeyT& key);


		// Modifiers.

		template<typename... Args>
		void insert(const KeyT& key, Args&&... args);

		void erase(const KeyT& key);

	private:

		std::unordered_map<KeyT, T> elements_;
		std::string Tname_;
		FactoryFunc factoryFunc_;

	};

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	registry<KeyT, T>::registry(FactoryFunc factory)
	: Tname_(T.typeName()), factoryFunc_(factory)
	{}

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	registry<KeyT, T>::~registry() 
	{}

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	const T& registry<KeyT, T>::get(const KeyT& key) {
	
	
	
	}

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	T& registry<KeyT, T>::set(const KeyT& key) {
	
	
	
	}

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	template <typename... Args>
	void registry<KeyT, T>::insert(const KeyT& key, Args&&... args) {
	
		if (elements_.contains(key))
			logger::errorLog("The " + Tname_ + " " + key + " is already registered.");
		else
			elements_[key] = factoryFunc_(std::make_any<std::tuple<Args...>>(std::forward<Args>(args)...));
	
	}

	template <typename KeyT, typename T>
	requires derivedFromRegistryElement<T>
	void registry<KeyT, T>::erase(const KeyT& key) {
	
	
	
	}

}

#endif