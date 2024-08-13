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
#include "../../definitions.h"
#include "../../logger.h"
#include "../../Registry/registryElement.h"
#include "../../Registry/registry.h"

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

		// Initialisation.

		/**
		* @brief Initialise the Uniform Buffer Object system.
		*/
		static void init(const std::string& typeName);

		
		// Constructors.

		/**
		* @brief Class constructor.
		* @param maxSize The maximum number of elements of type T that can be stored on the UBO.
		*/
		UBO(const std::string& name, std::size_t maxSize);

		/**
		* @brief Class constructor.
		* @param elements. The elements to make a copy of and store inside the UBO.
		*/
		UBO(const std::string& name, const registry<std::string, T>& elements);


		// Observers.

		const T& get() const;


		// Modifiers.

		T& get();

		void reupload();


		// Destructors.

		/**
		* @brief Default class destructor.
		*/
		~UBO();

	private:

		static bool initialised_;
		static unsigned int nUbos_; // Used to, for example, assign a non-occuppied binding point to the next UBO that gets created.

		unsigned int graphicsAPIID_;
		unsigned int bindingPoint_;
		std::string name_;
		std::vector<T> elements_;

	};

	template <typename T>
	requires std::default_initializable<T>
	void UBO<T>::init(const std::string& typeName) {

		if (initialised_) {

			logger::errorLog("Material system is already initialised");

		}
		else {

			nUbos_ = 0;

			initialised_ = true;

		}

	}

	template <typename T>
	requires std::default_initializable<T>
	UBO<T>::UBO(const std::string& name, std::size_t maxSize)
	: name_(name), elements_(maxSize) {

		bindingPoint_ = nUbos_++;

		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, bindingPoint_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}

	template <typename T>
	requires std::default_initializable<T>
	UBO<T>::UBO(const std::string& name, const registry<std::string, T>& elements)
	: name_(name), elements_(elements.size()) {

		typename registry<std::string, T>::const_iterator it = elements.cbegin();
		int i = 0;
		while(it != elements.cend()) {
		
			elements_[i] = it->second;

			it++;
			i++;
		
		}

		bindingPoint_ = nUbos_++;

		glGenBuffers(1, &graphicsAPIID_);
		glBindBuffer(GL_UNIFORM_BUFFER, graphicsAPIID_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * elements_.size(), elements_.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, bindingPoint_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	}

}

#endif