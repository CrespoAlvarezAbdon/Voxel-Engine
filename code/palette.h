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
	* @brief Container that maps a small set of integer IDs to a larger set of IDs.
	*/
	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	class palette {

	public:


		// Observers.

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		const small& getSmall(const large& l) const;

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		const large& getLarge(const small& s) const;

		/**
		* @brief Returns true if the specified key is found
		*/
		bool containsSmall(const small& s) const;

		/**
		* @brief Returns true if the specified key is found
		*/
		bool containsLarge(const large& s) const;

		/**
		* @brief Returns the current number of pairs of small-large values
		* in the palette.
		*/
		std::size_t size() const;


		// Modifiers.

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		small& getSmall(const large& l);

		/**
		* @brief Returns the value associated with the specified key.
		* Throws an exception if said key is not in the palette.
		*/
		large& getLarge(const small& s);

		/**
		* @brief Establishes a relation between the two values inside the palette.
		*/
		void insert(const small& s, const large& l);

		/**
		* @brief Erases the specified key.
		* Does nothing if such key is not in the palette.
		*/
		void eraseSmall(const small& s);

		/**
		* @brief Erases the specified key.
		* Does nothing if such key is not in the palette.
		*/
		void eraseLarge(const large& l);

	private:

		std::unordered_map<small, large> smallToLarge_;
		std::unordered_map<large, small> largeToSmall_;
		
	};

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	inline const small& palette<small, large>::getSmall(const large& l) const {
	
		if (largeToSmall_.contains(l))
			return largeToSmall_[l];
		else
			logger::errorLog("The specified large key is not present in the palette");
	
	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	const large& palette<small, large>::getLarge(const small& s) const {

		if (smallToLarge_.contains(s))
			return smallToLarge_[s];
		else
			logger::errorLog("The specified small key is not present in the palette");

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	inline bool palette<small, large>::containsSmall(const small& s) const {
	
		return smallToLarge_.contains(s);
	
	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	inline bool palette<small, large>::containsLarge(const large& l) const {

		return largeToSmall_.contains(l);

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	inline std::size_t palette<small, large>::size() const {

		return smallToLarge_.size();

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	small& palette<small, large>::getSmall(const large& l) {

		if (largeToSmall_.contains(l))
			return largeToSmall_[l];
		else
			logger::errorLog("The specified large key is not present in the palette");

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	large& palette<small, large>::getLarge(const small& s) {

		if (smallToLarge_.contains(s))
			return smallToLarge_[s];
		else
			logger::errorLog("The specified small key is not present in the palette");

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	void palette<small, large>::insert(const small& s, const large& l) {

		smallToLarge_[s] = l;
		largeToSmall_[l] = s;

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	void palette<small, large>::eraseSmall(const small& s) {

		const large& l = smallToLarge_[s];
		largeToSmall_.erase(l);
		smallToLarge_.erase(s);

	}

	template<typename small, typename large>
	requires T1smallerOrEqualToT2<small, large>
	void palette<small, large>::eraseLarge(const large& l) {

		const small& s = largeToSmall_[l];
		smallToLarge_.erase(s);
		largeToSmall_.erase(l);

	}

}

#endif