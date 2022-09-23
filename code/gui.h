#ifndef _VOXELENG_GUI_
#define _VOXELENG_GUI_
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include "vertex.h"
#include "renderer.h"
#include "gameWindow.h"
#include "definitions.h"
#include "controls.h"


namespace VoxelEng {

	////////////////////
	//Classes & enums.//
	////////////////////

	enum class GUIContainer {

		mainMenu,
		level,
		both

	};


	/*
	Base class that defines everything that a GUI element should at least have.
	*/
	class GUIElement {

	public:

		// Observers.

		const void* vertexData();

		size_t nVertices();

		bool isEnabled();

		const std::string& name();

		GUIContainer getGUIContainer();

		unsigned int& textureID();

	protected:

		// Constructors.

		GUIElement();


		// Observers.

		virtual bool getsInputFromTouch() = 0;


		// Modifiers.

		/*
		Generate vertex data used to represent this GUI element.
		Calls addTextures().
		*/
		virtual void genVertexData() = 0;

		virtual void addTextures() = 0;
		
		void executeKeyAction();

		void executeMouseButtonAction();


		// Attributes.

		static const window* window_;

		std::string name_;
		vec2 position_,
		     size_;
		unsigned int textureID_,
					 GUILayer;
		bool enabled_,
			 oldActivationEventReceived_;
		std::vector<vertex2D> vertices_;
		std::vector<unsigned int> children_; // Store children ID.
		void (*KeyFuncPtr_)();
		void (*MouseButtonFuncPtr_)();
		GUIContainer GUIContainer_;


	private:

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

	inline const std::string& GUIElement::name() {

		return name_;

	}

	inline GUIContainer GUIElement::getGUIContainer() {
	
		return GUIContainer_;
	
	}

	inline unsigned int& GUIElement::textureID() {
	
		return textureID_;
	
	}


	class GUIManager {

	public:

		// Initialisers.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		static void init(window& window, shader& shader, renderer& renderer);


		// Observers.

		static bool levelGUIOpened();

		static GUIElement* lastCheckedGUIElement();

		static bool someMMGUIChanged();

		static bool someLevelGUIChanged();
		

		// Modifiers.

		static void setLevelGUIOpened(bool value);

		static void switchLevelGUIOpened();

		static void setMMGUIChanged(bool value);

		static void setLevelGUIChanged(bool value);

		static void updateOrthoMatrix();

		/*
		Replaces the last bound key if there was already one.
		*/
		static void bindActKey(const std::string& GUIElementName, controlCode key);

		/*
		Replaces the last bound function if there was already one.
		*/
		static void bindActKeyFunction(const std::string& GUIElementName, void(*func)());

		/*
		Replaces the last bound key and function if there was already a bound key or function respectively.
		*/
		static void bindActKeyFunction(const std::string& GUIElementName, void(*func)(), controlCode key);

		/*
		Replaces the last bound key if there was already one.
		*/
		static void bindActMouseButton(const std::string& GUIElementName, controlCode mouseButton);

		/*
		Replaces the last bound function if there was already one.
		*/
		static void bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)());

		/*
		Replaces the last bound key and function if there was already a bound key or function respectively.
		*/
		static void bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)(), controlCode mouseButton);

		static void changeGUIState(const std::string& GUIElementName, bool isEnabled);

		static void changeGUIState(const std::string& GUIElementName);

		/*
		Get and process all GUI-related inputs that happen inside a game level.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		static void processLevelGUIInputs();

		/*
		Get and process all GUI-related inputs that happen inside the main menu.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		static void processMainMenuGUIInputs();

		/*
		Screen coordinates range from 0 to 1 in both edges and start at (0, 0) in
		the lower left corner.
		If isGUIOnMainMenu is false, then the GUI will only appear inside a game level.
		WARNING. GUIElements can only be added when loading the game!
		*/
		static void addGUIBox(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
						      bool isEnabled = true, GUIContainer container = GUIContainer::level, const std::string & parentName = "",
							  unsigned int layer = 0);

		/*
		Screen coordinates range from 0 to 1 in both edges and start at (0, 0) in
		the lower left corner.
		WARNING. GUIElements can only be added when loading the game!
		*/
		static void addGUIButton(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
								 bool isEnabled = true, GUIContainer container = GUIContainer::level, const std::string& parentName = "",
								 unsigned int layer = 0);

		/*
		Draws all active GUI elements.
		If renderMainMenu is equal to true, only GUIElements that correspond to the game level will be drawn if they are active.
		If it is equal to false, only GUIElements that correspond to the main menu will be drawn if they are active.
		WARNING. Must be called in a thread with valid OpenGL context.
		WARNING. Must be called with the graphic rendering mode set to 2D and depth test disabled.
		*/
		static void drawGUI(bool renderMainMenu = false);

		
		// Clean up.

		static void cleanUp();

	private:

		static bool initialised_,
					someGUIChanged_,
			        skipMouseInputs_,
					levelGUIOpened_;
		static controlCode lastMouseButtonInput_;
		static unsigned int GUIElementCount_;
		static glm::mat4 projectionMatrix_;

		static std::unordered_map<unsigned int, GUIElement*> GUIElements_;
		static std::unordered_map<unsigned int, controlCode> LevelBoundKey_,
															 MMBoundKey_,
															 boundMouseButton_;
		static std::unordered_set<unsigned int> LevelActiveGUIElements_[N_GUI_LAYERS],
												LevelInactiveGUIElements_[N_GUI_LAYERS],
												MMActiveGUIElements_[N_GUI_LAYERS],
												MMInactiveGUIElements_[N_GUI_LAYERS], // MM stands for Main Menu.
												LevelActiveGUIButtons_[N_GUI_LAYERS], // This allows us to only perform hitbox checks on active GUIButtons since buttons cannot be
												MMActiveGUIButtons_[N_GUI_LAYERS];	  // checked if they have been clicked if they are not active (that does not happen with other GUIElements
																                      // that perform an action by being pressed).
		static std::unordered_map<std::string, unsigned int> LevelGUIElementID,
															 MMGUIElementID;
		static window* window_;
		static GraphicsAPIWindow* windowAPIPointer_;

		static GUIElement* lastCheckedGUIElement_; // Has the last GUIElement checked by the processGUIInputs method. It's meant to reference, for example, a GUIButton inside
											       // it's own activation mouse button function.

		static vertexBuffer* vbo_;
		static vertexArray* vao_;
		static shader* shader_;
		static renderer* renderer_;
		
	};

	inline bool GUIManager::levelGUIOpened() {
	
		return levelGUIOpened_;
	
	}

	inline GUIElement* GUIManager::lastCheckedGUIElement() {
	
		return lastCheckedGUIElement_;
	
	}

	inline void GUIManager::setLevelGUIOpened(bool value) {
	
		levelGUIOpened_ = value;
	
	}

	inline void GUIManager::switchLevelGUIOpened() {
	
		levelGUIOpened_ = !levelGUIOpened_;
	
	}

	inline bool GUIManager::someMMGUIChanged() {
	
		return someGUIChanged_;
	
	}

	inline bool GUIManager::someLevelGUIChanged() {
	
		return someGUIChanged_;
	
	}

	inline void GUIManager::setMMGUIChanged(bool value) {
	
		someGUIChanged_ = value;
	
	}

	inline void GUIManager::setLevelGUIChanged(bool value) {
	
		someGUIChanged_ = value;
	
	}

}

#endif