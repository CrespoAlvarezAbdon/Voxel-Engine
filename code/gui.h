#ifndef _VOXELENG_GUI_
#define _VOXELENG_GUI_
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include "vertex.h"
#include "renderer.h"
#include "gameWindow.h"
#include "definitions.h"
#include "controls.h"


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

		// Constructors.
		GUIElement();


		// Attributes.
		static std::unordered_map<unsigned int, GUIElement*>* activeGUIElements_,
															* inactiveGUIElements_;
		static const window* window_;

		vec2<float> position_,
					size_;
		unsigned int textureID_;
		bool enabled_,
			 actKeyPressed_;
		std::vector<vertex2D> vertices_;
		std::vector<unsigned int> children_; // Store children ID.
		void (*KeyFuncPtr)();


		// Modifiers.

		/*
		Generate vertex data used to represent this GUI element.
		Calls addTextures().
		*/
		virtual void genVertexData() = 0;
		virtual void addTextures() = 0;

		/*
		Function that will be called when the key bound to this GUIElement (if any) is pressed.
		*/
		void executeKeyAction();

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
		static void initialize(window& window, shader& shader, renderer& renderer);


		// Modifiers.

		static void updateOrthoMatrix();

		static void bindActKey(const std::string& GUIElementName, key key);

		static void bindActKeyFunction(const std::string& GUIElementName, void(*func)());

		static void changeGUIState(const std::string& GUIElementName, bool isEnabled);

		static void changeGUIState(const std::string& GUIElementName);

		/*
		Get and process all GUI-related inputs.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		static void processGUIInputs();

		/*
		Screen coordinates range from 0 to 1 in both edges and start at (0, 0) in
		the lower left corner.
		WARNING. GUIElements can only be added when loading the game!
		*/
		static void addGUIBox(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
									  bool isEnabled = true, const std::string& parentName = "");


		// Other methods.

		/*
		Draws all active GUI elements.
		WARNING. Must be called in a thread with valid OpenGL context.
		WARNING. Graphic rendering mode will be set to 2D while this method executes.
		*/
		static void drawGUI();

	private:

		static unsigned int GUIElementCount_;
		static glm::mat4 projectionMatrix_;
		static std::unordered_map<unsigned int, GUIElement*> activeGUIElements_,
															 inactiveGUIElements_;
		static std::unordered_map<unsigned int, key> boundKey_;
		static std::unordered_map<std::string, unsigned int> GUIElementID;
		static window* window_;

		static vertexBuffer* vbo_;
		static vertexArray* vao_;
		static shader* shader_;
		static renderer* renderer_;
		
	};

}

#endif