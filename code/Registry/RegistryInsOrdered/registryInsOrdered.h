#ifndef _VOXELENG_REGISTYINSORDERED_
#define _VOXELENG_REGISTYINSORDERED_

#include <concepts>
#include <cstddef>
#include <map>
#include <type_traits>
#include "../registry.h"
#include "../../logger.h"


namespace VoxelEng {

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	class registryInsOrdered : public registry<KeyT, T> {

	public:

		// Nested classes.

		template <typename T, bool isConst>
		requires std::derived_from<T, registryElement>
		class registryIterator {

		public:

			// Types.

			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = typename std::conditional<IsConst, const T*, T*>::type;
			using reference = typename std::conditional<IsConst, const T&, T&>::type;


			// Constructors.

			/**
			* @brief Class constructor.
			* @param ptr The pointer from to the element the iterator refers to.
			*/
			iterator(pointer ptr);


			// Operators.

			/**
			* @brief Dereference operator.
			* @returns A reference to the element associated with the iterator.
			*/
			reference operator*() const;

			/**
			* @brief Pointer operator.
			* @returns A pointer to the element associated with the iterator.
			*/
			pointer operator->() const;

			/**
			* @brief Pre-increment operator. If the iterator was already end(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on but associated with the next element
			* in the container. If there is no next element, the iterator will point to end().
			*/
			iterator& operator++();

			/**
			* @brief Post-increment operator. If the iterator was already end(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on with the element it was associated with
			* before this call. If there is no next element, the iterator will point to end().
			*/
			iterator operator++(int);

			/**
			* @brief Pre-decrement operator. If the iterator was already begin(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on but associated with the previous element
			* in the container.
			*/
			iterator& operator--();

			/**
			* @brief Post-decrement operator. If the iterator was already begin(), the code will enter undefined behaviour.
			* @returns The same iterator this method was called on with the element it was associated with
			* before this call.
			*/
			iterator operator--(int);

			/**
			* @brief Addition operator.
			* @param offset The amount of positions in the container to advance.
			* @returns An iterator that points at the position specified by the iterator this method was called on
			* advanced the same number of times as the value in 'offset'. If this operation returns an iterator past end(),
			* the code will enter undefined behaviour.
			*/
			iterator operator+(difference_type offset) const;

			/**
			* @brief Subtraction operator.
			* @param offset The amount of positions in the container to move backwards.
			* @returns An iterator that points at the position specified by the iterator this method was called on
			* moved backwards the same number of times as the value in 'offset'. If this operation returns an iterator past begin(),
			* the code will enter undefined behaviour.
			*/
			iterator operator-(difference_type offset) const;

			/**
			* @brief Substraction operator.
			* @param it2 The left operand iterator.
			* @returns The difference of positions between the two iterators. Also called distance between the two iterators. Can be negative if the left operand
			* corresponds to an element that comes before the right operand's element when traversing the container from begin() to end() or if the iterators
			* are random-access iterators and the first element in the container can be reach from the last element in it.
			*/
			difference_type operator-(const iterator& it2) const;

			/**
			* @brief Compound assignment for addition. The code will enter undefined behaviour if a position previous to begin() or past end() is the
			* new position the iterator will be associated with when this call is computed.
			* @param offset Number of positions to advance the iterator.
			* @returns The iterator advanced the given number of positions.
			*/
			iterator& operator+=(difference_type offset);

			/**
			* @brief Compound assignment for subtraction. The code will enter undefined behaviour if a position previous to begin() or past end() is the
			* new position the iterator will be associated with when this call is computed.
			* @param offset Number of positions to move backwards the iterator.
			* @returns The iterator movec backwards the given number of positions.
			*/
			iterator& operator-=(difference_type offset);

			/**
			* @brief Subscript operator. The code will enter undefined behaviour if the provided index is not valid or is out of the container's bounds.
			* @param index The position of the element to access.
			* @returns The element specified through the given index.
			*/
			reference operator[](difference_type index) const;

			/**
			* @brief Equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) and it2 are equal (true) or not (false).
			*/
			bool operator==(const iterator& it2) const;

			/**
			* @brief Not equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) and it2 are NOT equal (true) or not (false).
			*/
			bool operator!=(const iterator& it2) const;

			/**
			* @brief Less than operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) comes before it2 (true) or not (false).
			*/
			bool operator<(const iterator& it2) const;

			/**
			* @brief Greater than operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) comes after it2 (true) or not (false).
			*/
			bool operator>(const iterator& it2) const;

			/**
			* @brief Less than or equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) comes before or is equal to it2 (true) or not (false).
			*/
			bool operator<=(const iterator& it2) const;

			/**
			* @brief Greater than or equal to operator.
			* @param it2 Left operand iterator.
			* @returns Whether the iterator this method was called on (the right operand) comes after or is equal to it2 (true) or not (false).
			*/
			bool operator>=(const iterator& it2) const;

		private:

			registry<KeyT, T>::iterator it_;
			pointer ptr_;

		};


		// Types.

		using iterator = registryIterator<T, false>;
		using const_iterator = registryIterator<T, true>;


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		registryInsOrdered();


		// Observers.

		/**
		* @brief Get the specified element. Throws exception if it is not registered or if the provided index is out of bounds or invalid.
		* @param index The index that corresponds to the specified element.
		* @param elementKey Optional output parameter to hold the retrieve element's key.
		* @returns The specified element.
		*/
		const T& get(unsigned int index, KeyT* elementKey = nullptr) const;

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
		void insert(const KeyT& key, Args&&... args) override;

		/**
		* @brief Get the specified element. Throws exception if it is not registered or if the provided index is out of bounds or invalid.
		* @param index The index that corresponds to the specified element.
		* @param elementKey Optional output parameter to hold the retrieve element's key.
		* @returns The specified element.
		*/
		T& get(unsigned int index, KeyT* elementKey = nullptr);

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

		std::map<int, KeyT> orderedKeys_;
		unsigned int nInserts_;

	};

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<KeyT, T>::registryInsOrdered()
	: nInserts_(0) {}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	const T& registryInsOrdered<KeyT, T>::get(unsigned int index, KeyT* elementKey) const {
	
		if (orderedKeys_.contains(index)) {
		
			const KeyT key = orderedKeys_[index];
			if (elementKey != nullptr) {

				*elementKey = key;

			}

			return elements_[key];
		
		}
		else {
		
			logger::errorLog("Element with index " + std::to_string(index) + " is not registered");
		
		}
		
	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	template <typename... Args>
	void registryInsOrdered<KeyT, T>::insert(const KeyT& key, Args&&... args) {

		registry<KeyT, T>::insert(key, args);
		orderedKeys_[nInserts_++] = key;

		// NEXT. TERMINAR DE IMPLEMENTAR LOS CRUD Y LOS MÉTODOS DE LOS ITERATORS.

		// TODO. CUANDO SE TERMINE TODO HACER PRUEBA DE REGISTRYINSORDERED::SHRINKTOFIT.

	}

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	T& registryInsOrdered<KeyT, T>::get(unsigned int index, KeyT* elementKey) {

		if (orderedKeys_.contains(index)) {

			const KeyT key = orderedKeys_[index];
			if (elementKey != nullptr) {

				*elementKey = key;

			}

			return elements_[key];

		}
		else {

			logger::errorLog("Element with index " + std::to_string(index) + " is not registered");

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
		for (std::map<int, KeyT>::iterator it = orderedKeys_.begin(); it != orderedKeys_.end(); it++) {
		
			iteratorIndex = it->first;

			if(index != iteratorIndex) {
			
				orderedKeys_[index] = orderedKeys_[iteratorIndex];
				orderedKeys_.erase(iteratorIndex);
			
			}

			index++;
		
		}
	
	}

}

#endif