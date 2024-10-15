/**
* @file UBOs.h
* @version 1.0
* @date 13/08/2024
* @author Abdon Crespo Alvarez
* @title Uniform Buffer Objects (UBOs).
* @brief Contains the definition of the UBO class.
*/
#ifndef _VOXELENG_UBOS_
#define _VOXELENG_UBOS_

#include <concepts>
#include <cstddef>
#include <string>
#include <vector>
#include <definitions.h>
#include <Registry/RegistryInsOrdered/registryInsOrdered.h>
#include <Utilities/Logger/logger.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief UBOs or Uniform Buffer Object is a set of global uniform shader variables
	* that are shared across all loaded shaders. In consequence, there is no need to resend them if
	* a currently bound shader is unbound and another one is bound instead.
	*/
	template <class T>
	requires std::default_initializable<T>
	class UBO {

	public:

		// Constructors.

		/**
		* @brief Class constructor. Automatically assigns the given space in GPU.
		* @param name UBO's name in the shaders it is going to be bound to.
		* @param maxSize The maximum number of elements of type T that can be stored in the UBO.
		* @param bindingPoint The UBO's binding point that is referenced in the shader where it
		* is going to be used.
		*/
		UBO(const std::string& name, std::size_t maxSize, unsigned int bindingPoint);

		/**
		* @brief Class constructor. Automatically uploads the given data into GPU.
		* @param name UBO's name in the shaders it is going to be bound to.
		* @param elements. The elements to make a copy of and store inside the UBO.
		* @param bindingPoint The UBO's binding point that is referenced in the shader where it
		* is going to be used.
		*/
		UBO(const std::string& name, const registryInsOrdered<std::string, T>& elements, unsigned int bindingPoint);


		// Observers.

		/**
		* @brief Get the UBO's name.
		*/
		const std::string& name() const;

		/**
		* @brief Get the UBO's binding point.
		*/
		unsigned int bindingPoint() const;

		/**
		* @brief Get a specified element of the UBO.
		* WARNING. Throws exception if the provided index points to an invalid or nonexistent element.
		*/
		const T& get(unsigned int index) const;


		// Modifiers.

		/**
		* @brief Get a specified element of the UBO.
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


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		~UBO();

	private:

		unsigned int graphicsAPIID_;
		unsigned int bindingPoint_;
		std::string name_;
		std::vector<T> elements_;

	};

	template <typename T>
	requires std::default_initializable<T>
	UBO<T>::UBO(const std::string& name, std::size_t maxSize, unsigned int bindingPoint)
	: bindingPoint_(bindingPoint), name_(name), elements_(maxSize) {

		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * elements_.size(), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}

	template <typename T>
	requires std::default_initializable<T>
	UBO<T>::UBO(const std::string& name, const registryInsOrdered<std::string, T>& elements, unsigned int bindingPoint)
	: bindingPoint_(bindingPoint), name_(name), elements_(elements.size()) {

		typename registryInsOrdered<std::string, T>::const_iterator it = elements.orderedCbegin();
		int i = 0;
		while(it != elements.orderedCend()) {
		
			elements_[i] = *(it->second);

			it++;
			i++;
		
		}

		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	}

	template <typename T>
	requires std::default_initializable<T>
	const std::string& UBO<T>::name() const {
	
		return name_;
	
	}

	template <typename T>
	requires std::default_initializable<T>
	unsigned int UBO<T>::bindingPoint() const {

		return bindingPoint_;

	}

	template <typename T>
	requires std::default_initializable<T>
	const T& UBO<T>::get(unsigned int index) const {

		if (index < elements_.size())
			return elements_[index];
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on UBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	T& UBO<T>::get(unsigned int index) {

		if (index < elements_.size())
			return elements_[index];
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on UBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	void UBO<T>::reupload() {

		glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * elements_.size(), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T) * elements_.size(), elements_.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}

	template <typename T>
	requires std::default_initializable<T>
	void UBO<T>::reuploadElement(unsigned int index) {

		if (index < elements_.size()) {
		
			T& element = elements_[index];

			glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(T) * index, sizeof(T), &element);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		}
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on UBO " + name_);

	}

	template <typename T>
	requires std::default_initializable<T>
	UBO<T>::~UBO() {

		glDeleteBuffers(1, &graphicsAPIID_);

	}

}

#endif