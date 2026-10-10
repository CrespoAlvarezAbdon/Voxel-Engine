#ifndef _VOXELENG_SSBO_
#define _VOXELENG_SSBO_

#include <concepts>
#include <cstddef>
#include <string>
#include <vector>
#include <Utilities/Logger/logger.h>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>

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


		// Observers.

		/**
		* @brief Get the SSBO's name.
		*/
		const std::string& name() const;

		/**
		* @brief Get the SSBO's binding point.
		*/
		unsigned int bindingPoint() const;


		// Modifiers.

		/**
		* @brief Set the SSBO's contents and reupload them. Will reallocate buffer if the current size is lower
		* than the number of the new elements.
		* @param elements The new contents of the SSBO.
		*/
		void setContentsAndReupload(const std::vector<T>& elements);

		/**
		* @brief Set the SSBO's contents and reupload them. Will reallocate buffer if the current size is lower
		* than the number of the new elements.
		* @param elements The new contents of the SSBO.
		*/
		void setContentsAndReupload(const Padded3DArray<T>& elements);

		/**
		* @brief Set the SSBO's contents and reupload them. Will reallocate buffer if the current size is lower
		* than for the element to upload.
		* @param elements The new contents of the SSBO.
		*/
		void setContentsAndReupload(const T& element, std::size_t index);


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		~SSBO();

	private:

		/*
		Attributes.
		*/

		unsigned int graphicsAPIID_;
		unsigned int bindingPoint_;
		unsigned int nElements_;
		std::string name_;

	};

	template <class T>
	requires std::default_initializable<T>
	SSBO<T>::SSBO(const std::string& name, std::size_t maxSize, unsigned int bindingPoint)
	: name_(name), bindingPoint_(bindingPoint), nElements_(maxSize) {
	
		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * nElements_, nullptr, GL_DYNAMIC_DRAW);
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
	void SSBO<T>::setContentsAndReupload(const std::vector<T>& elements) {

		std::size_t elementsSize = elements.size();

		if (elementsSize > nElements_) {

			nElements_ = elementsSize;
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * nElements_, elements.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}
		else {

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * elementsSize, elements.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::setContentsAndReupload(const Padded3DArray<T>& elements) {

		std::size_t elementsSize = elements.size();

		if (elementsSize > nElements_) {

			nElements_ = elementsSize;
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * nElements_, elements.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}
		else {

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * elementsSize, elements.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}

	}

	template <typename T>
	requires std::default_initializable<T>
	void SSBO<T>::setContentsAndReupload(const T& element, std::size_t index) {

		if (index < nElements_) {

			glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(T) * index, sizeof(T), &element);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		}
		else
			throw std::runtime_error("There is no element at index " + std::to_string(index) + " on UBO " + name_);

	}
	
	template <typename T>
	requires std::default_initializable<T>
	SSBO<T>::~SSBO() {

		glDeleteBuffers(1, &graphicsAPIID_);

	}

}

#endif