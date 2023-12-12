#ifndef _VOXELENG_ATOMIC_RECYCLING_POOL_
#define _VOXELENG_ATOMIC_RECYCLING_POOL_

#include "recyclingPool.h"

namespace VoxelEng {

	template <typename T>
	class atomicRecyclingPool : public recyclingPool<T> {

	public:

		// Constructors.

		atomicRecyclingPool(unsigned int nElements = 0);


		// Observers.

		unsigned int totalSize();

		bool allFreeOnClear();


		// Modifiers.

		const T& getConst();

		T& get();

		void free(T& element);

		void setAllFreeOnClear(bool value);


		// Clean up.

		void clear();


		// Destructors.

		~atomicRecyclingPool();

	private:

		

	};

	template<typename T>
	atomicRecyclingPool<T>::atomicRecyclingPool(unsigned int nElements)
	: recyclingPool<T>::recyclingPool(nElements)
	{}

	template<typename T>
	unsigned int atomicRecyclingPool<T>::totalSize() {
	
		std::unique_lock<std::mutex> lock(recyclingPool<T>::mutex_);

		return recyclingPool<T>::totalSize();
	
	}

	template<typename T>
	bool atomicRecyclingPool<T>::allFreeOnClear() {

		std::unique_lock<std::mutex> lock(recyclingPool<T>::mutex_);

		return recyclingPool<T>::allFreeOnClear();

	}

	template<typename T>
	const T& atomicRecyclingPool<T>::getConst() {

		std::unique_lock<std::mutex> lock (recyclingPool<T>::mutex_);

		T* e = &recyclingPool<T>::get();

		return *e;

	}

	template<typename T>
	T& atomicRecyclingPool<T>::get() {

		std::unique_lock<std::mutex> lock(recyclingPool<T>::mutex_);

		T* e = &recyclingPool<T>::get();

		return *e;

	}

	template<typename T>
	void atomicRecyclingPool<T>::free(T& element) {

		recyclingPool<T>::mutex_.lock();

		recyclingPool<T>::free(element);

		recyclingPool<T>::mutex_.unlock();

	}

	template<typename T>
	void atomicRecyclingPool<T>::setAllFreeOnClear(bool value) {

		recyclingPool<T>::mutex_.lock();

		recyclingPool<T>::allFreeOnClear() = value;

		recyclingPool<T>::mutex_.unlock();

	}

	template<typename T>
	void atomicRecyclingPool<T>::clear() {

		recyclingPool<T>::mutex_.lock();

		recyclingPool<T>::clear();

		recyclingPool<T>::mutex_.unlock();

	}

	template<typename T>
	inline atomicRecyclingPool<T>::~atomicRecyclingPool() {

		clear();

	}

}

#endif