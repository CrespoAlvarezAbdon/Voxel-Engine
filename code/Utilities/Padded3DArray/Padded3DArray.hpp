#ifndef _VOXELENG_PADDED_3D_ARRAY_
#define _VOXELENG_PADDED_3D_ARRAY_

#include <algorithm>
#include <vector>

#include <vec.h>

namespace VoxelEng {

	/**
	* @brief Definition of an array with additonal padding on its indices so that indexing with negative values is possible.
	*/
	template <typename T>
	class Padded3DArray {

	public:

		// Constructors.

		/**
		* @brief Default constructor.
		*/
		Padded3DArray();

		/**
		* @brief Class constructor.
		* @param sizeX First dimension size of the array without padding.
		* @param sizeY Second dimension size of the array without padding.
		* @param sizeZ Third dimension size of the array without padding.
		* @param padding Padding added to the 3 dimensions of the array so that, for any dimension D, you can access indices from 0-padding to 
		* size-dimension-D+padding. So the true size of that dimension D is size-dimension-D+padding*2;
		* @param defaultValue. Default value to fill all the elemnts in the array with.
		*/
		Padded3DArray(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ, unsigned int padding, T defaultValue = T());

		/**
		* @brief Copy constructor
		* @param a Array to copy
		*/
		Padded3DArray(const Padded3DArray& a);

		/**
		* @brief Get the pointer to the array's raw data.
		* @returns The pointer to the array's raw data.
		*/
		const T* data() const;

		/**
		* @brief Get the total number of elements in the array.
		*/
		std::size_t size() const;


		// Observers.

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::const_reference at(int x, int y, int z) const;

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::const_reference at(const vec3& coords) const;

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::const_reference get(int x, int y, int z) const;

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::const_reference get(const vec3& coords) const;

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::const_reference operator[](const vec3& coords) const;


		// Modifiers.

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::reference at(int x, int y, int z);

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::reference at(const vec3& coords);

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::reference get(int x, int y, int z);

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::reference get(const vec3& coords);

		/**
		* @brief Get the element at the specified coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		typename std::vector<T>::reference operator[](const vec3& coords);

		/**
		* @brief Fill the array with the given value.
		* @param value The given value.
		*/
		void fill(T value);

		/**
		* @brief Fill the array with the default value.
		*/
		void clear();

		/**
		* @brief Get the pointer to the array's raw data.
		* @returns The pointer to the array's raw data.
		*/
		T* data();

	private:

		/*
		Attributes.
		*/
		// padding = 3;
		// actual size for dimension D is [16 + 3*2] = [16 + 6] = [22] = [0...21] = [-3...18] = [0-3...15+3].
		unsigned int sizeX_;
		unsigned int sizeY_;
		unsigned int sizeZ_;
		unsigned int padding_;
		unsigned int sizeYPadded_;
		unsigned int sizeZPadded_;
		std::vector<T> array_;
		T defaultValue_;

		/*
		Methods.
		*/

		unsigned int getLinearIndex_(int x, int y, int z) const;

	};

	template <typename T>
	inline Padded3DArray<T>::Padded3DArray()
	: sizeX_(0), sizeY_(0), sizeZ_(0), padding_(0),
	sizeYPadded_(0), sizeZPadded_(0),
	array_(0, T()), defaultValue_(T()) {}

	template <typename T>
	inline Padded3DArray<T>::Padded3DArray(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ, unsigned int padding, T defaultValue)
	: sizeX_(sizeX), sizeY_(sizeY), sizeZ_(sizeZ), padding_(padding),
	sizeYPadded_(sizeY + padding * 2), sizeZPadded_(sizeZ + padding * 2),
	array_((sizeX + padding*2) * sizeYPadded_* sizeZPadded_, defaultValue),
	defaultValue_(defaultValue) {}

	template <typename T>
	inline Padded3DArray<T>::Padded3DArray(const Padded3DArray<T>& a)
	: sizeX_(a.sizeX_), sizeY_(a.sizeY_), sizeZ_(a.sizeZ_), padding_(a.padding_),
	sizeYPadded_(sizeY_ + padding_ * 2), sizeZPadded_(sizeZ_ + padding_ * 2),
	array_(a.array_),
	defaultValue_(a.defaultValue_) {}

	template <typename T>
	inline const T* Padded3DArray<T>::data() const {

		return array_.data();

	}

	template <typename T>
	inline std::size_t Padded3DArray<T>::size() const {
	
		return array_.size();
	
	}

	template <typename T>
	inline typename std::vector<T>::const_reference Padded3DArray<T>::at(int x, int y, int z) const {
	
		return array_.at(getLinearIndex_(x, y, z));
	
	}

	template <typename T>
	inline typename std::vector<T>::const_reference Padded3DArray<T>::at(const vec3& coords) const {

		return array_.at(getLinearIndex_(coords.x, coords.y, coords.z));

	}

	template <typename T>
	inline typename std::vector<T>::const_reference Padded3DArray<T>::get(int x, int y, int z) const {

		return array_[getLinearIndex_(x, y, z)];

	}

	template <typename T>
	inline typename std::vector<T>::const_reference Padded3DArray<T>::get(const vec3& coords) const {

		return array_[getLinearIndex_(coords.x, coords.y, coords.z)];

	}

	template <typename T>
	inline typename std::vector<T>::const_reference Padded3DArray<T>::operator[](const vec3& coords) const {
	
		return array_[getLinearIndex_(coords.x, coords.y, coords.z)];
	
	}

	template <typename T>
	inline typename std::vector<T>::reference Padded3DArray<T>::at(int x, int y, int z) {

		return array_.at(getLinearIndex_(x, y, z));

	}

	template <typename T>
	inline typename std::vector<T>::reference Padded3DArray<T>::at(const vec3& coords) {

		return array_.at(getLinearIndex_(coords.x, coords.y, coords.z));

	}

	template <typename T>
	inline typename std::vector<T>::reference Padded3DArray<T>::get(int x, int y, int z) {

		return array_[getLinearIndex_(x, y, z)];

	}

	template <typename T>
	inline typename std::vector<T>::reference Padded3DArray<T>::get(const vec3& coords) {

		return array_[getLinearIndex_(coords.x, coords.y, coords.z)];

	}

	template <typename T>
	inline typename std::vector<T>::reference Padded3DArray<T>::operator[](const vec3& coords) {

		return array_[getLinearIndex_(coords.x, coords.y, coords.z)];

	}

	template <typename T>
	inline void Padded3DArray<T>::fill(T value) {
	
		std::fill(array_.begin(), array_.end(), value);
	
	}

	template <typename T>
	inline void Padded3DArray<T>::clear() {

		fill(defaultValue_);

	}

	template <typename T>
	inline T* Padded3DArray<T>::data() {
	
		return array_.data();
	
	}

	template <typename T>
	unsigned int Padded3DArray<T>::getLinearIndex_(int x, int y, int z) const {
	
		// First apply the padding.
		x += padding_;
		y += padding_;
		z += padding_;

		// Then compute the linear index of the element and return it.
		return x * (sizeYPadded_ * sizeZPadded_) + y * sizeZPadded_ + z;
	
	}

}

#endif