#ifndef _VOXELENG_REGISTRY_
#define _VOXELENG_REGISTRY_

#include <any>
#include <cstddef>
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

		/**
		* @brief Represents a factory function to create the elements stored in the registry.
		*/
		using factoryFunc = std::function<std::unique_ptr<T>(std::any)>;

		using iterator = std::unordered_map<KeyT, std::unique_ptr<T>>::iterator;

		using const_iterator = std::unordered_map<KeyT, std::unique_ptr<T>>::const_iterator;


		// Constructors.

		registry() = delete;

		/**
		* @brief Class constructor.
		* @param factory Factory function to create the elements stored in the registry.
		*/
		registry(factoryFunc factory);


		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~registry();


		// Observers.

		/**
		* @brief Get the specified element. Throws exception if it is not registered.
		* @param key The key that corresponds to the value that is the specified element.
		*/

		const T& get(const KeyT& key) const;

		/**
		* @brief Get the number of elements in the registry.
		*/
		std::size_t size() const;

		/**
		* @brief Get an iterator to the beginning of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator begin() const;

		/**
		* @brief Get an iterator to the beginning of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator cbegin() const;

		/**
		* @brief Get an iterator to the end of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator end() const;

		/**
		* @brief Get an iterator to the end of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator cend() const;


		// Modifiers.

		/**
		* @brief Get the specified element. Throws exception if it is not registered.
		* @param key The key that corresponds to the value that is the specified element.
		*/
		T& get(const KeyT& key);

		/**
		* @brief Create and insert an element into the registry. Throws exception if the given key
		* to associate with the created element is already in use by another element previously registered.
		* @param key The key to associate to the value that is the specified element.
		* @param args The element's constructor parameters.
		*/
		template<typename... Args>
		void insert(const KeyT& key, Args&&... args);

		/**
		* @brief Erase the specified element. Throws exception if the given key does not correspond
		* to any of the registered elements.
		* @param key The key associated with the element to erase.
		*/
		void erase(const KeyT& key);

		/**
		* @brief Get the iterator to the beginning of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		iterator begin();

		/**
		* @brief Get an iterator to the end of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		iterator end();

	private:

		std::unordered_map<KeyT, std::unique_ptr<T>> elements_;
		std::string Tname_;
		factoryFunc factoryFunc_;

	};

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::registry(factoryFunc factory)
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
	std::size_t registry<KeyT, T>::size() const {
	
		return elements_.size();
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::const_iterator registry<KeyT, T>::begin() const {

		return elements_.begin();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::const_iterator registry<KeyT, T>::cbegin() const {
	
		return elements_.cbegin();
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
		registry<KeyT, T>::const_iterator registry<KeyT, T>::end() const {

		return elements_.end();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
		registry<KeyT, T>::const_iterator registry<KeyT, T>::cend() const {

		return elements_.cend();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
		T& registry<KeyT, T>::get(const KeyT& key) {

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

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::iterator registry<KeyT, T>::begin() {

		return elements_.begin();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registry<KeyT, T>::iterator registry<KeyT, T>::end() {

		return elements_.end();

	}

}

#endif