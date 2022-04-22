#ifndef _VOXELENG_GUI_
#define _VOXELENG_GUI_
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>
#include <vector>
#include <mutex>
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include "vertex.h"
#include "renderer.h"
#include "gameWindow.h"
#include "definitions.h"


namespace VoxelEng {

	///////////
	//Classes//
	///////////

	// GUIElement.
	// Base class that defines everything that a GUI element should at least have.
	class GUIElement {

	public:

		// Observers.

		const void* vertexData();

		size_t nVertices();

		bool isEnabled();

	protected:

		// Attributes.
		static std::vector<GUIElement*>* activeGUIElements_,
									   * inactiveGUIElements_;
		static const window* window_;

		vec2<float> position_,
					size_;
		unsigned int textureID_;
		bool enabled_;
		std::vector<vertex2D> vertices_;


		// Other methods.

		/*
		Generate vertex data used to represent this GUI element.
		Calls addTextures().
		*/
		virtual void genVertexData() = 0;
		virtual void addTextures() = 0;

		/*
		WARNING. Can only be called when genVertexData has already been called!
		*/
		virtual void resizeElement() = 0;

		// Misc
		friend class GUIManager;

	};

	inline const void* GUIElement::vertexData() {
	
		return vertices_.data();
	
	}

	inline size_t GUIElement::nVertices() {
	
		return vertices_.size();
	
	}

	inline bool GUIElement::isEnabled() {
	
		return enabled_;
	
	}


	// GUIManager.
	class GUIManager {

	public:

		// Initializers.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		static void initialize(const window& window, shader& shader, renderer& renderer);


		// Modifiers.

		static void updateAspectRatio();

		/*
		Screen coordinates range from 0 to 1 in both edges and start at (0,0) in
		the lower left corner.
		*/
		static void addGUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, bool isEnabled = true);


		// Other methods.

		/*
		Draws all active GUI elements.
		WARNING. Must be called in a thread with valid OpenGL context.
		WARNING. Graphic rendering mode will be set to 2D while this method executes.
		*/
		static void drawGUI();

	private:

		static glm::mat4 projectionMatrix_;
		static std::vector<GUIElement*> activeGUIElements_,
										inactiveGUIElements_;
		static const window* window_;

		static std::recursive_mutex mutex_;
		static vertexBuffer* vbo_;
		static vertexArray* vao_;
		static shader* shader_;
		static renderer* renderer_;

	};

}

#endif