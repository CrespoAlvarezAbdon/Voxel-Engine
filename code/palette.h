#ifndef _VOXELENG_PALETTE_
#define _VOXELENG_PALETTE_

#include <cstddef>
#include <concepts>
#include <unordered_map>

#include "logger.h"


namespace VoxelEng {

	/////////////
	//Concepts.//
	/////////////

	template<typename T1, typename T2>
	concept T1smallerOrEqualToT2 = sizeof(T1) <= sizeof(T2);


	////////////
	//Classes.//
	////////////

	/**
	* @brief Container that maps a T1 set of integer IDs to a T2r set of IDs.
	*/
	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	class palette {

	public:


		// Observers.

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		const T1& getT1(const T2& l) const;

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		const T2& getT2(const T1& s) const;

		/**
		* @brief Returns true if the specified key is found
		*/
		bool containsT1(const T1& s) const;

		/**
		* @brief Returns true if the specified key is found
		*/
		bool containsT2(const T2& s) const;

		/**
		* @brief Returns the current number of pairs of T1-T2 values
		* in the palette.
		*/
		std::size_t size() const;

		/**
		* @brief Returns the constant iterator that points to the first element of the palette.
		* NOTE. The palette is not required to store its elements in a certain order.
		*/
		std::unordered_map<T1, T2>::const_iterator cbegin() const;

		/**
		* @brief Returns the constant iterator that points to the last element of the palette.
		* NOTE. The palette is not required to store its elements in a certain order.
		*/
		std::unordered_map<T1, T2>::const_iterator cend() const;


		// Modifiers.

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		T1& getT1(const T2& l);

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		T2& getT2(const T1& s);

		/**
		* @brief Establishes a relation between the two values inside the palette.
		*/
		void insert(const T1& s, const T2& l);

		/**
		* @brief Erases the specified key.
		* Does nothing if such key is not in the palette.
		*/
		void eraseT1(const T1& s);

		/**
		* @brief Erases the specified key.
		* Does nothing if such key is not in the palette.
		*/
		void eraseT2(const T2& l);

	private:

		std::unordered_map<T1, T2> T1ToT2_;
		std::unordered_map<T2, T1> T2ToT1_;
		
	};

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	inline const T1& palette<T1, T2>::getT1(const T2& l) const {
	
		if (T2ToT1_.contains(l))
			return T2ToT1_[l];
		else
			logger::errorLog("The specified T2 key is not present in the palette");
	
	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	const T2& palette<T1, T2>::getT2(const T1& s) const {

		if (T1ToT2_.contains(s))
			return T1ToT2_[s];
		else
			logger::errorLog("The specified T1 key is not present in the palette");

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	inline bool palette<T1, T2>::containsT1(const T1& s) const {
	
		return T1ToT2_.contains(s);
	
	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	inline bool palette<T1, T2>::containsT2(const T2& l) const {

		return T2ToT1_.contains(l);

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	inline std::size_t palette<T1, T2>::size() const {

		return T1ToT2_.size();

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	std::unordered_map<T1, T2>::const_iterator palette<T1, T2>::cbegin() const {
	
		return T1ToT2_.cbegin();
	
	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	std::unordered_map<T1, T2>::const_iterator palette<T1, T2>::cend() const {
	
		return T1ToT2_.cend();
	
	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	T1& palette<T1, T2>::getT1(const T2& l) {

		if (T2ToT1_.contains(l))
			return T2ToT1_[l];
		else
			logger::errorLog("The specified T2 key is not present in the palette");

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	T2& palette<T1, T2>::getT2(const T1& s) {

		if (T1ToT2_.contains(s))
			return T1ToT2_[s];
		else
			logger::errorLog("The specified T1 key is not present in the palette");

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	void palette<T1, T2>::insert(const T1& s, const T2& l) {

		T1ToT2_[s] = l;
		T2ToT1_[l] = s;

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	void palette<T1, T2>::eraseT1(const T1& s) {

		const T2& l = T1ToT2_[s];
		T2ToT1_.erase(l);
		T1ToT2_.erase(s);

	}

	template<typename T1, typename T2>
	requires T1smallerOrEqualToT2<T1, T2>
	void palette<T1, T2>::eraseT2(const T2& l) {

		const T1& s = T2ToT1_[l];
		T1ToT2_.erase(s);
		T2ToT1_.erase(l);

	}

}

#endif