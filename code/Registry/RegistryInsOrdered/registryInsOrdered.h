#ifndef _VOXELENG_REGISTYINSORDERED_
#define _VOXELENG_REGISTYINSORDERED_

#include <concepts>
#include <cstddef>
#include <map>
#include <type_traits>
#include <utility>
#include <Registry/registry.h>
#include <Utilities/Logger/logger.h>


namespace VoxelEng {

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	class registryInsOrdered : public registry<KeyT, T> {

	public:

		// Nested classes.

		template <bool isConst>
		class registryIterator {

		public:

			// Types.

			using iterator_category = std::random_access_iterator_tag;
			using value_type = std::pair<const KeyT, std::unique_ptr<T>>;
			using difference_type = std::ptrdiff_t;
			using pointer = typename std::conditional<isConst, const value_type*, value_type*>::type;
			using reference = typename std::conditional<isConst, const value_type&, value_type&>::type;
			using nativeIterator = std::map<unsigned int, KeyT>::iterator;


			// Constructors.

			/**
			* @brief Class constructor.
			* @param ptr The pointer from to the element the iterator refers to.
			*/
			registryIterator(nativeIterator it, registryInsOrdered<KeyT, T>* registryPointer)
			: thisRegistry_(registryPointer), it_(it)
			{}


			// Operators.

			/**
			* @brief Dereference operator.
			* @returns A reference to the element associated with the iterator.
			*/
			reference operator*() const {
			
				return *(thisRegistry_->elements_.find(it_->second));
			
			}

			/**
			* @brief Pointer operator.
			* @returns A pointer to the element associated with the iterator.
			*/
			pointer operator->() const {
			
				return &(*(thisRegistry_->elements_.find(it_->second)));
			
			}

			/**
			* @brief Pre-increment operator. If the iterator was already end(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on but associated with the next element
			* in the container. If there is no next element, the iterator will point to end().
			*/
			registryIterator<isConst>& operator++() {
			
				it_ = thisRegistry_->orderedKeys_.find(it_->first + 1);
				return *this;
			
			}

			/**
			* @brief Post-increment operator. If the iterator was already end(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on with the element it was associated with
			* before this call. If there is no next element, the iterator will point to end().
			*/
			registryIterator<isConst> operator++(int) {
			
				registryIterator<isConst> temp = *this;
				++(*this);
				return temp;
			
			}

			/**
			* @brief Pre-decrement operator. If the iterator was already begin(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on but associated with the previous element
			* in the container.
			*/
			registryIterator<isConst>& operator--() {

				it_ = orderedKeys_.find(it_->first - 1);
				return *this;

			}

			/**
			* @brief Post-decrement operator. If the iterator was already begin(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on with the element it was associated with
			* before this call.
			*/
			registryIterator<isConst> operator--(int) {

				registryIterator<isConst> temp = *this;
				--(*this);
				return temp;

			}

			/**
			* @brief Equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) and it2 are equal (true) or not (false).
			*/
			bool operator==(const registryIterator<isConst>& it2) const {
			
				return it_ == it2.it_;

			}

			/**
			* @brief Not equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) and it2 are NOT equal (true) or not (false).
			*/
			bool operator!=(const registryIterator<isConst>& it2) const {

				return it_ != it2.it_;

			}

			/**
			* @brief Conversion operator from non-const iterator to const iterator.
			* @returns A const iterator.
			*/
			template <bool c = isConst, typename std::enable_if<!c, int>::type = 0>
			operator registryIterator<true>() const {
			
				return registryIterator<true>(it_, thisRegistry_);
			
			}

		private:

			registryInsOrdered<KeyT, T>* thisRegistry_;
			nativeIterator it_;

		};


		// Types.

		using iterator = registryIterator<false>;
		using const_iterator = registryIterator<true>;


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		registryInsOrdered(registry<KeyT, T>::factoryFunc factory, registry<KeyT, T>::onInsertFunc);


		// Observers.

		/**
		* @brief Get the specified element with its insertion order index. 
		* Throws exception if it is not registered or if the provided index is out of bounds or invalid.
		* @param index The index that corresponds to the specified element.
		* @param elementKey Optional output parameter to hold the retrieve element's key.
		* @returns The specified element.
		*/
		const T& getWithInsIndex(unsigned int index, KeyT* elementKey = nullptr) const;

		/**
		* @brief Get the insertion order index for the specified element key.
		* @param elementKey The element's key.
		* @returns The element's index in the insertion order.
		*/
		unsigned int getInsIndex(const KeyT& elementKey) const;

		/**
		* @brief Get an iterator to the beginning of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator orderedBegin() const;

		/**
		* @brief Get an iterator to the beginning of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator orderedCbegin() const;

		/**
		* @brief Get an iterator to the end of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator orderedEnd() const;

		/**
		* @brief Get an iterator to the end of the registry.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		const_iterator orderedCend() const;


		// Modifiers.

		/**
		* @brief Create and insert an element into the registry. Throws exception if the given key
		* to associate with the created element is already in use by another element previously registered.
		* @param key The key to associate to the value that is the specified element.
		* @param args The element's constructor parameters.
		*/
		template<typename... Args>
		void insert(const KeyT& key, Args&&... args);

		/**
		* @brief Get the specified element. Throws exception if it is not registered or if the provided index is out of bounds or invalid.
		* @param index The index that corresponds to the specified element.
		* @param elementKey Optional output parameter to hold the retrieve element's key.
		* @returns The specified element.
		*/
		T& getWithInsIndex(unsigned int index, KeyT* elementKey = nullptr);

		/**
		* @brief Erase the specified element. Throws exception if the given key does not correspond
		* to any of the registered elements.
		* @param key The key associated with the element to erase.
		*/
		void erase(const KeyT& key);

		/**
		* @brief Erase the specified element. Throws exception if the given key does not correspond
		* to any of the registered elements.
		* @param key The key associated with the element to erase.
		*/
		void erase(unsigned int index);

		/**
		* @brief Reduces the memory allocated that is unused by this registry.
		* WARNING. This method has linear time complexity on the number of elementes registered so call 
		* it only when required.
		*/
		void shrinkToFit();

		/**
		* @brief Get the iterator to the beginning of the registry. This iterator follows the elements' insertion order.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		iterator orderedBegin();

		/**
		* @brief Get an iterator to the end of the registry. This iterator follows the elements' insertion order.
		* WARNING. It is NOT guaranteed that the order of the elements in the registry will be consistent.
		* WARNING. Iterators are invalidated if the number of elements in the registry is altered.
		*/
		iterator orderedEnd();

	private:

		std::map<unsigned int, KeyT> orderedKeys_;
		std::map<KeyT, unsigned int> orderedKeysReverse_;
		unsigned int nInserts_;

	};

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::registryInsOrdered(registry<KeyT, T>::factoryFunc factory, registry<KeyT, T>::onInsertFunc onInsert)
	: registry<KeyT, T>(factory, onInsert), nInserts_(0) {}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	const T& registryInsOrdered<KeyT, T>::getWithInsIndex(unsigned int index, KeyT* elementKey) const {
	
		return const_cast<registryInsOrdered<KeyT, T>*>(this)->get(index, elementKey);
		
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	unsigned int registryInsOrdered<KeyT, T>::getInsIndex(const KeyT& elementKey) const {
	
		if (orderedKeysReverse_.contains(elementKey))
			return orderedKeysReverse_.at(elementKey);
		else
			logger::errorLog("There is no insertion order index for element key " + elementKey);
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::const_iterator registryInsOrdered<KeyT, T>::orderedBegin() const {
	
		return const_cast<registryInsOrdered<KeyT, T>*>(this)->orderedBegin();
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::const_iterator registryInsOrdered<KeyT, T>::orderedCbegin() const {

		return const_cast<registryInsOrdered<KeyT, T>*>(this)->orderedBegin();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::const_iterator registryInsOrdered<KeyT, T>::orderedEnd() const {

		return const_cast<registryInsOrdered<KeyT, T>*>(this)->orderedEnd();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::const_iterator registryInsOrdered<KeyT, T>::orderedCend() const {

		return const_cast<registryInsOrdered<KeyT, T>*>(this)->orderedEnd();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	template <typename... Args>
	void registryInsOrdered<KeyT, T>::insert(const KeyT& key, Args&&... args) {

		registry<KeyT, T>::insert_(key, std::forward<Args>(args)...);
		orderedKeys_[nInserts_++] = key;
		orderedKeysReverse_[key] = nInserts_ - 1;

		if(registry<KeyT, T>::onInsertFunc_)
			registry<KeyT, T>::onInsertFunc_();

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	T& registryInsOrdered<KeyT, T>::getWithInsIndex(unsigned int index, KeyT* elementKey) {

		if (orderedKeys_.contains(index)) {

			const KeyT& key = orderedKeys_[index];
			if (elementKey != nullptr) {

				*elementKey = key;

			}

			return registry<KeyT, T>::elements_[key];

		}
		else {

			logger::errorLog("Element with insertion order index " + std::to_string(index) + " is not registered");

		}

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	void registryInsOrdered<KeyT, T>::erase(const KeyT& key) {

		registry<KeyT, T>::erase(key);
		unsigned int keyIndex = orderedKeysReverse_.at[key];
		orderedKeysReverse_.erase(key);
		orderedKeys_.erase(keyIndex);

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	void registryInsOrdered<KeyT, T>::erase(unsigned int index) {
	
		if (orderedKeys_.contains(index)) {
		
			const KeyT& key = orderedKeys_[index];
			registry<KeyT, T>::erase(key);
			orderedKeysReverse_.erase(key);
			orderedKeys_.erase(index);
		
		}
		else {
		
			logger::errorLog("There is no element registered with index " + std::to_string(index));
		
		}
		
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	void registryInsOrdered<KeyT, T>::shrinkToFit() {
	
		nInserts_ = orderedKeys_.size();
		unsigned int index = 0;
		unsigned int iteratorIndex = 0;

		// Basically delete the "gaps" in orderedKeys between element N and N+1.
		// For example: if orderedKeys_ contains the following keys: {0, 1, 3, 5}.
		// This method should rearrange the elements so that they have the following keys while preserving the elements in the their order of insertion:
		// {0, 1, 2, 3}. Where 2 and 3 are keys that refer respectively to the elements that were previously referred by keys 3 and 5.
		
		for (auto it = orderedKeys_.begin(); it != orderedKeys_.end(); it++) {
		
			iteratorIndex = it->first;

			if(index != iteratorIndex) {
			
				orderedKeys_[index] = orderedKeys_[iteratorIndex];
				orderedKeys_.erase(iteratorIndex);
				orderedKeysReverse_[it->second] = index;
			
			}

			index++;
		
		}
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::iterator registryInsOrdered<KeyT, T>::orderedBegin() {
	
		return iterator(orderedKeys_.begin(), this);
	
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::iterator registryInsOrdered<KeyT, T>::orderedEnd() {

		return iterator(orderedKeys_.end(), this);

	}

}

#endif