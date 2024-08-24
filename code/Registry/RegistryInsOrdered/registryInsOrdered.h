#ifndef _VOXELENG_REGISTYINSORDERED_
#define _VOXELENG_REGISTYINSORDERED_

#include <concepts>
#include <cstddef>
#include <map>
#include "../registry.h"

namespace VoxelEng {

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement>
	class registryInsOrdered : public registry<KeyT, T> {

	public:

		// Nested classes.

		template <typename T>
		requires std::derived_from<T, registryElement>
		class iterator {

		public:

			// Types.

			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;


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

		private:

			registry<KeyT, T>::iterator it_;

		};

		class const_iterator {

		public:



		private:

			registry<KeyT, T>::const_iterator it_;

		};


		// Observers.

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

	};

}

#endif