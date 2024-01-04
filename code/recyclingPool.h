#ifndef _VOXELENG_RECYCLING_POOL_
#define _VOXELENG_RECYCLING_POOL_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_set>
#include "logger.h"


namespace VoxelEng {

	template<typename T>
	class recyclingPool {

	public:

		// Constructors.

		recyclingPool(unsigned int nElements = 0);


		// Observers.

		unsigned int totalSize() const;

		unsigned int nFreeElements() const;

		unsigned int nOccupiedElements() const;

		bool allFreeOnClear() const;


		// Modifiers.

		const T& getConst();

		T& get();

		void free(T& element);

		bool& allFreeOnClear();

		void waitUntilAllFree();


		// Clean up.

	    void clear();


		// Destructors.

		~recyclingPool();

	protected:

		/*
		Attributes.
		*/

		std::queue<T*> freeElements_;
		std::unordered_set<T*> occupiedElements_;
		bool allFreeOnClear_; // Tells if all the elements must be free elements before clearing the pool.
		std::mutex mutex_;
		std::condition_variable allFreeCV_;

	};

	template<typename T>
	recyclingPool<T>::recyclingPool(unsigned int nElements) {
	
		for (unsigned int i = 0; i < nElements; i++)
			freeElements_.push(new T());
	
	}

	template<typename T>
	inline unsigned int recyclingPool<T>::totalSize() const {
	
		return freeElements_.size() + occupiedElements_.size();
	
	}

	template<typename T>
	inline unsigned int recyclingPool<T>::nFreeElements() const {

		return freeElements_.size();

	}

	template<typename T>
	inline unsigned int recyclingPool<T>::nOccupiedElements() const {

		return occupiedElements_.size();

	}

	template<typename T>
	inline bool recyclingPool<T>::allFreeOnClear() const {
	
		return allFreeOnClear_;
	
	}

	template<typename T>
	inline const T& recyclingPool<T>::getConst() {
	
		return get();
	
	}

	template<typename T>
	T& recyclingPool<T>::get() {
	
		T* element = nullptr;
		if (freeElements_.empty())
			element = new T();
		else {
		
			element = freeElements_.front();
			freeElements_.pop();
			
		}

		occupiedElements_.insert(element);

		return *element;
	
	}

	template<typename T>
	void recyclingPool<T>::free(T& element) {
	
		T* e = &element;

		if (occupiedElements_.contains(e)) {

			occupiedElements_.erase(e);
			freeElements_.push(e);

			if (occupiedElements_.empty())
				allFreeCV_.notify_all();

		}
		else
			logger::errorLog("Element does not belong to this recyclingPool");
	
	}

	template<typename T>
	inline bool& recyclingPool<T>::allFreeOnClear() {

		return allFreeOnClear_;

	}

	template<typename T>
	void recyclingPool<T>::waitUntilAllFree() {
	
		std::unique_lock<std::mutex> lock(mutex_);

		while (!occupiedElements_.empty())
			allFreeCV_.wait(lock);
	
	}

	template<typename T>
	void recyclingPool<T>::clear() {
	
		if (!allFreeOnClear_ || occupiedElements_.empty())
			for (int i = 0; i < freeElements_.size(); i++) {

				delete(freeElements_.front());
				freeElements_.pop();

			}
		else
			logger::errorLog("Some elements from this recyclingPool are still in use when clearing it");
	
	}

	template<typename T>
	inline recyclingPool<T>::~recyclingPool() {
	
		clear();
	
	}

}

#endif