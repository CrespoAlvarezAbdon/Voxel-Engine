#ifndef _VOXELENG_THREAD_SAFE_
#define _VOXELENG_THREAD_SAFE_

#include <mutex>
#include <utility>

namespace VoxelEng {

	template<typename T>
	class threadsafe 
	{
	public:

		// Constructors.

		threadsafe();

		template<typename U>
		requires std::constructible_from<T, U&&>
		threadsafe(U&& value);


		// Observers.

		/**
		* @brief Get the wrapped object. WARNING. No mutual exclusion guaranteed with only this method.
		* @return The wrapped object.
		*/
		const T& get() const;


		// Modifiers.

		/**
		* @brief Lock the mutex wrapping the object. Blocks the calling thread if its is already locked.
		*/
		void lock();

		/**
		* @brief Unlock the mutex wrapping the object. Undefined behaviour if it is already locked.
		*/
		void unlock();

		/**
		* @brief Tries to lock the mutex wrapping the object. Does not the calling thread if its is already locked.
		* @return Whether the mutex was locked (true) or not (false).
		*/
		bool try_lock();

		/**
		* @brief Get the wrapped object. WARNING. No mutual exclusion guaranteed with only this method.
		* @return The wrapped object.
		*/
		T& get();


	private:

		std::mutex mutex_;
		T object_;

	};

	template<typename T>
	inline threadsafe<T>::threadsafe() {}

	template<typename T>
	template<typename U>
	requires std::constructible_from<T, U&&>
	inline threadsafe<T>::threadsafe(U&& value)
	: object_(std::move(value))
	{}

	template<typename T>
	inline const T& threadsafe<T>::get() const {
	
		return object_;
	
	}

	template<typename T>
	inline void threadsafe<T>::lock() {

		mutex_.lock();

	}

	template<typename T>
	inline void threadsafe<T>::unlock() {

		mutex_.unlock();

	}

	template<typename T>
	inline bool threadsafe<T>::try_lock() {

		return mutex_.try_lock();

	}

	template<typename T>
	inline T& threadsafe<T>::get() {
	
		return object_;
	
	}

}

#endif