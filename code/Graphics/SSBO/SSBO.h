#ifndef _VOXELENG_SSBO_
#define _VOXELENG_SSBO_

#include <concepts>
#include <cstddef>
#include <string>
#include <vector>
#include <Registry/RegistryInsOrdered/registryInsOrdered.h>
#include <Utilities/Logger/logger.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	/**
	* @brief A Shader Storage Buffer Object (SSBO) acts like an UBO, but it can provide larger storage capacity, 
	* it is writable and can have variable storage size.
	*/
	template <class T>
	requires std::default_initializable<T>
	class SSBO {

	public:

		// Constructors.

		/**
		* @brief Class constructor. Automatically assigns the given space in GPU.
		* @param name SSBO's name in the shaders it is going to be bound to.
		* @param maxSize The maximum number of elements of type T that can be stored in the SSBO.
		* @param bindingPoint The SSBO's binding point that is referenced in the shader where it
		* is going to be used.
		*/
		SSBO(const std::string& name, std::size_t maxSize, unsigned int bindingPoint);

		/**
		* @brief Class constructor. Automatically uploads the given data into GPU.
		* @param name SSBO's name in the shaders it is going to be bound to.
		* @param elements. The elements to make a copy of and store inside the SSBO.
		* @param bindingPoint The SSBO's binding point that is referenced in the shader where it
		* is going to be used.
		*/
		SSBO(const std::string& name, const registryInsOrdered<std::string, T>& elements, unsigned int bindingPoint);


		// Observers.

		/**
		* @brief Get the SSBO's name.
		*/
		const std::string& name() const;

		/**
		* @brief Get the SSBO's binding point.
		*/
		unsigned int bindingPoint() const;

		/**
		* @brief Get a specified element of the SSBO.
		* WARNING. Throws exception if the provided index points to an invalid or nonexistent element.
		*/
		const T& get(unsigned int index) const;


		// Modifiers.

		/**
		* @brief Get a specified element of the SSBO.
		* WARNING. Throws exception if the provided index points to an invalid or nonexistent element.
		*/
		T& get(unsigned int index);

		/**
		* @brief Reupload the UBO's entire current data into the GPU.
		* WARNING. Not thread-safe.
		*/
		void reupload();

		/**
		* @brief Reupload the UBO's current data for the specified element into the GPU.
		* WARNING. Not thread-safe.
		* WARNING. Throws exception if the provided index points to an invalid or nonexistent element.
		*/
		void reuploadElement(unsigned int index);

		/**
		* @brief Set the SSBO's contents and reupload them. Will reallocate buffer if the current size is lower
		* than the number of the new elements.
		* @param elements The new contents of the SSBO.
		*/
		void setContentsAndReupload(const registryInsOrdered<std::string, T>& elements);

		/**
		* @brief Set the SSBO's contents and reupload them. Will reallocate buffer if the current size is lower
		* than the number of the new elements.
		* @param elements The new contents of the SSBO.
		*/
		void setContentsAndReupload(const std::vector<T>& elements);


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		~SSBO();

	private:

		/*
		Methods.
		*/

		void setElements_(const registryInsOrdered<std::string, T>& elements);

		/*
		Attributes.
		*/

		unsigned int graphicsAPIID_;
		unsigned int bindingPoint_;
		std::string name_;
		std::vector<T> elements_;

	};

	template <class T>
	requires std::default_initializable<T>
	SSBO<T>::SSBO(const std::string& name, std::size_t maxSize, unsigned int bindingPoint)
	: name_(name), bindingPoint_(bindingPoint), elements_(maxSize) {
	
		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elements_.size(), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint_, graphicsAPIID_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	}

	template <typename T>
	requires std::default_initializable<T>
	SSBO<T>::SSBO(const std::string& name, const registryInsOrdered<std::string, T>& elements, unsigned int bindingPoint)
	: bindingPoint_(bindingPoint), name_(name), elements_(elements.size()) {

		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);

		setElements_(elements);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint_, graphicsAPIID_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	}

	template <typename T>
	requires std::default_initializable<T>
	const std::string& SSBO<T>::name() const {

		return name_;

	}

	template <typename T>
	requires std::default_initializable<T>
	unsigned int SSBO<T>::bindingPoint() const {

		return bindingPoint_;

	}

	template <typename T>
	requires std::default_initializable<T>
	const T& SSBO<T>::get(unsigned int index) const {

		if (index < elements_.size())
			return elements_[index];
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on SSBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	T& SSBO<T>::get(unsigned int index) {

		if (index < elements_.size())
			return elements_[index];
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on SSBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::reupload() {

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elements_.size(), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * elements_.size(), elements_.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::reuploadElement(unsigned int index) {

		if (index < elements_.size()) {
		
			T& element = elements_[index];

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * index, sizeof(T), &element);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		}
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on SSBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::setContentsAndReupload(const registryInsOrdered<std::string, T>& elements) {
	
		if (elements.size() > elements_.size()) {
		
			elements_ = std::vector<T>(elements.size());
			setElements_(elements);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		}
		else {
		
			setElements_(elements);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * elements.size(), elements_.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		}
	
	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::setContentsAndReupload(const std::vector<T>& elements) {

		if (elements.size() > elements_.size()) {

			elements_ = elements;
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}
		else {

			for (int i = 0; i < elements.size(); i++)
				elements_[i] = elements[i];
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * elements.size(), elements_.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}

	}
	
	template <typename T>
	requires std::default_initializable<T>
	SSBO<T>::~SSBO() {

		glDeleteBuffers(1, &graphicsAPIID_);

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::setElements_(const registryInsOrdered<std::string, T>& elements) {
	
		typename registryInsOrdered<std::string, T>::const_iterator it = elements.orderedCbegin();
		int i = 0;
		while (it != elements.orderedCend()) {

			elements_[i] = *(it->second);

			it++;
			i++;

		}
	
	}

}

#endif