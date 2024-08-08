#ifndef _VOXELENG_REGISTRY_
#define _VOXELENG_REGISTRY_

#include <any>
#include <concepts>
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include "registryElement.h"
#include "../definitions.h"
#include "../logger.h"

namespace VoxelEng {

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
	requires std::derived_from<T, registryElement>
	class registry {

	public:

		// Types.

		using FactoryFunc = std::function<std::unique_ptr<T>(std::any)>;


		// Constructors.

		registry() = delete;

		registry(FactoryFunc factory);


		// Destructors.

		~registry();


		// Observers.

		const T& get(const KeyT& key) const;

		T& get(const KeyT& key);


		// Modifiers.

		template<typename... Args>
		void insert(const KeyT& key, Args&&... args);

		void erase(const KeyT& key);

	private:

		std::unordered_map<KeyT, std::unique_ptr<T>> elements_;
		std::string Tname_;
		FactoryFunc factoryFunc_;

	};

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::registry(FactoryFunc factory)
	: Tname_(T::typeName()), factoryFunc_(factory)
	{}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::~registry() 
	{}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	const T& registry<KeyT, T>::get(const KeyT& key) const {
	
		if (elements_.contains(key))
			return elements_.at(key);
		else
			logger::errorLog("The " + Tname_ + " " + key + " is not registered.");
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	T& registry<KeyT, T>::get(const KeyT& key)  {
	
		if (elements_.contains(key))
			return elements_.at(key);
		else
			logger::errorLog("The " + Tname_ + " " + key + " is not registered.");
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	template <typename... Args>
	void registry<KeyT, T>::insert(const KeyT& key, Args&&... args) {
	
		if (elements_.contains(key))
			logger::errorLog("The " + Tname_ + " " + key + " is already registered.");
		else
			elements_[key] = factoryFunc_(std::make_any<std::tuple<Args...>>(std::forward<Args>(args)...));
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	void registry<KeyT, T>::erase(const KeyT& key) {
	
		if (elements_.contains(key))
			elements_.erase(key);
		else
			logger::errorLog("The " + Tname_ + " " + key + " is not registered.");
	
	}

}

#endif