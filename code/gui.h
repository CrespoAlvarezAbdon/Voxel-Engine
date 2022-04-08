#ifndef _VOXELENG_GUI_
#define _VOXELENG_GUI_
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>
#include "definitions.h"


namespace VoxelEng {

	///////////
	//Classes//
	///////////

	// GUIElement

	class GUIElement {

	public:

		// Observers.
		const bool& enabled() const;

		// Modifiers.
		bool& enabled();

	private:

		int textureID_;
		vec2<float> position_,
					scale_;
		bool enabled_;

	};

	inline const bool& GUIElement::enabled() const {

		return enabled_;

	}

	inline bool& GUIElement::enabled() {

		return enabled_;

	}

	// GUIManager.

	class GUIManager {

	public:

		// Initializers.

		static void initialize(int screenWidth, int screenHeight);

		// Observers.

		static const glm::mat4& projectionMatrix();

		static const glm::mat4& viewMatrix();

		static const std::unordered_map<unsigned int, GUIElement*>& activeGUIelements();

		// Cleaners.

		/*
		Called when the game is finished to free all GUIs from
		the heap.
		*/
		static void clear();

	private:

		static glm::mat4 projectionMatrix_,
							   viewMatrix_;
		static std::unordered_map<unsigned int, GUIElement*> activeGUIelements_,
			                                                 inactiveGUIelements_;

	};

	inline const glm::mat4& GUIManager::projectionMatrix() {
	
		return projectionMatrix_;
	
	}

	inline const glm::mat4& GUIManager::viewMatrix() {

		return viewMatrix_;

	}

}

#endif