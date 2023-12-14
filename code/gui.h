/**
* @file gui.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title GUI.
* @brief Contains the definition of the Graphical User Interface (GUI)
* in the engine, along with the base class to create types of GUI elements
* and some built-in basic GUI elements.
*/
#ifndef _VOXELENG_GUI_
#define _VOXELENG_GUI_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include "controls.h"
#include "definitions.h"
#include "vertexBuffer.h"
#include "vertexArray.h"
#include "shader.h"
#include "vertex.h"
#include "gameWindow.h"
#include "vec.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>

#endif

namespace VoxelEng {

	////////////////////
	//Classes & enums.//
	////////////////////

	/**
	* @brief GUIcontainers are a way to group GUIelements
	* into sets that are only usable in different circumstances
	* of the engine's execution.
	*/
	enum class GUIcontainer {

		mainMenu,
		level,
		both

	};


	/**
	* @brief Base class that defines everything that a GUI element should at least have.
	*/
	class GUIelement {

	public:

		// Observers.

		/**
		* @brief Get the GUIelement's vertex data.
		* WARNING. Not thread-safe.
		*/
		const void* vertexData() const;

		/**
		* @brief Get the number of vertices of the element's mesh.
		* WARNING. Not thread-safe.
		*/
		size_t nVertices()  const;

		/**
		* @brief Returns true if the GUIelement is enabled 
		* (can be displayed and can receive input from other sources such as user input) or false otherwise.
		* WARNING. Not thread-safe.
		*/
		bool isEnabled()  const;

		/**
		* @brief Get the GUIelement's unique name.
		* WARNING. Not thread-safe.
		*/
		const std::string& name()  const;

		/**
		* @brief Get the GUIelement's GUIcontainer that is associated with.
		* WARNING. Not thread-safe.
		*/
		const GUIcontainer& getGUIContainer()  const;

		/**
		* @brief Get the texture ID of the mesh used by the GUIelement.
		* WARNING. Not thread-safe.
		*/
		const unsigned int& textureID()  const;

		/**
		* @brief Contains the GUIelement IDs of the children of this GUIelement.
		*/
		const std::unordered_set<unsigned int> children() const;

		/**
		* @brief Returns the GUIelement's original position.
		*/
		const vec2& originalPos() const;

		/**
		* @brief Returns the GUIelement's original size.
		*/
		const vec2& originalSize() const;

		/**
		* @brief Returns the GUIelement's position taking into account things like the
		* graphics API's aspect ratio.
		*/
		const vec2& truePos() const;


		// Modifiers.

		/**
		* @brief Changes the texture being used by the GUIelement.
		* WARNING. Not thread-safe.
		*/
		void changeTextureID(unsigned int newTextureID);

		/**
		* @brief Locks the mutex associated with the GUIelement.
		* WARNING. Will throw exception if the mutex is already locked.
		*/
		void lockMutex();

		/**
		* @brief Unlocks the mutex associated with the GUIelement.
		* WARNING. Undefined behaviour if called before a corresponding call to 'lockMutex' on the same object.
		*/
		void unlockMutex();

		/**
		* @brief Apply the current aspect ratio to the GUIelement.
		*/
		void applyAspectRatio(float aspectRatio);

	protected:

		// Constructors.

		GUIelement();


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
		vec2 originalPos_,
			 originalSize_,
			 truePos_, // This take into account aspect ratio and the parent's transform space (if any).
			 translatedPos_;
		unsigned int textureID_,
					 GUILayer_;
		bool enabled_,
			 oldActivationEventReceived_;
		std::vector<vertex2D> vertices_;
		std::unordered_set<unsigned int> children_; // Store children ID.
		GUIelement* parent_;
		void (*KeyFuncPtr_)();
		void (*MouseButtonFuncPtr_)();
		GUIcontainer GUIContainer_;
		std::mutex GUIElementMutex_;

	private:

		friend class GUImanager;
		
	};

	inline const void* GUIelement::vertexData() const {

		return vertices_.data();
	
	}

	inline size_t GUIelement::nVertices() const {
	
		return vertices_.size();
	
	}

	inline bool GUIelement::isEnabled() const {
	
		return enabled_;
	
	}

	inline const std::string& GUIelement::name() const {

		return name_;

	}

	inline const GUIcontainer& GUIelement::getGUIContainer() const {
	
		return GUIContainer_;
	
	}

	inline const unsigned int& GUIelement::textureID() const {
	
		return textureID_;
	
	}

	inline const std::unordered_set<unsigned int> GUIelement::children() const {
	
		return children_;
	
	}

	inline const vec2& GUIelement::originalPos() const {
	
		return originalPos_;
	
	}

	inline const vec2& GUIelement::originalSize() const {

		return originalSize_;

	}

	inline const vec2& GUIelement::truePos() const {
	
		return truePos_;
	
	}


	/**
	* @brief Manages everything related to GUIelements.
	*/
	class GUImanager {

	public:

		// Initialisers.

		/**
		* @brief Initialise the GUI management system.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void init(window& window, shader& shader);


		// Observers.
		
		/**
		* @brief Returns true if the specified GUIelement with the graphical main
		* menu execution phase as its GUIcontainer is registered or false otherwise.
		*/
		static bool isMMGUIElementRegistered(const std::string& name);

		/**
		* @brief Returns true if the specified GUIelement with the level execution phase
		* as its GUIcontainer is registered or false otherwise.
		*/
		static bool isLevelGUIElementRegistered(const std::string& name);

		/**
		* @brief Returns true if the in-level menu is being displayed or false otherwise.
		*/
		static bool levelGUIOpened();

		/**
		* @brief Returns a pointer to the last GUIelement that was processed
		* by the GUI input processing thread.
		*/
		static GUIelement* lastCheckedGUIElement();

		/**
		* @brief Returns true if any GUIelement with the graphical main
		* menu execution phase as its GUIcontainer was modified in any way or false otherwise.
		*/
		static bool someMMGUIChanged();

		/**
		* @brief Returns true if any GUIelement with the level
		* execution phase as its GUIcontainer was modified in any way or false otherwise.
		*/
		static bool someLevelGUIChanged();

		/**
		* @brief Returns the specified GUIelement.
		*/
		static const GUIelement& cGetGUIElement(const std::string& name);
		

		// Modifiers.

		/**
		* @brief Sets the flag that tells if the in-level menu is being displayed or false otherwise.
		*/
		static void setLevelGUIOpened(bool value);

		/**
		* @brief Switches the value of the flag that tells if the in-level menu is being displayed.
		*/
		static void switchLevelGUIOpened();

		/**
		* @brief Sets the flag that tells if any GUIelement with the graphical main menu
		* execution phase as its GUIcontainer was modified in any way.
		*/
		static void setMMGUIChanged(bool value);

		/**
		* @brief Sets the flag that tells if any GUIelement with the level
		* execution phase as its GUIcontainer was modified in any way.
		*/
		static void setLevelGUIChanged(bool value);

		/**
		* @brief Updates the orthographic matrix that is used
		* to render the GUIelements with the newest value
		* of the parameters that it uses.
		*/
		static void updateOrthoMatrix();

		/**
		* @brief Binds an user input key to activate its assigned GUI function (if any) to a GUIelement.
		* Replaces the last bound key if there was already one.
		*/
		static void bindActKey(const std::string& GUIElementName, controlCode key);

		/**
		* @brief Binds a GUI function activated with a user input key to a GUIelement.
		* Replaces the last bound function if there was already one.
		*/
		static void bindActKeyFunction(const std::string& GUIElementName, void(*func)());

		/**
		* @brief Binds a GUI function and the corresponding user input key to activate it to a GUIelement.
		* Replaces the last bound key and function if there was already a bound key or function respectively.
		*/
		static void bindActKeyFunction(const std::string& GUIElementName, void(*func)(), controlCode key);

		/**
		* @brief Binds an user input mouse button to activate its assigned GUI function (if any) to a GUIelement.
		* Replaces the last bound mouse button if there was already one.
		*/
		static void bindActMouseButton(const std::string& GUIElementName, controlCode mouseButton);

		/**
		* @brief Binds a GUI function activated with mouse button input to a GUIelement.
		* Replaces the last bound function if there was already one.
		*/
		static void bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)());

		/**
		* @brief Binds a GUI function and the corresponding user input mouse button to activate it to a GUIelement.
		* Replaces the last bound mouse button and function if there was already a bound key or function respectively.
		*/
		static void bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)(), controlCode mouseButton);

		/**
		* @brief Changes the active state of the specified GUIelement.
		*/
		static void changeGUIState(const std::string& GUIElementName, bool isEnabled);

		/**
		* @brief Changes the active state of the specified GUIelement.
		*/
		static void changeGUIState(const std::string& GUIElementName);

		/**
		* @brief Returns the specified GUIelement.
		*/
		static GUIelement& getGUIElement(const std::string& name);

		/**
		* @brief Get and process all GUI-related inputs that happen inside a game level.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void processLevelGUIInputs();

		/**
		* @brief Get and process all GUI-related inputs that happen inside the main menu.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void processMainMenuGUIInputs();

		/**
		* @brief Screen coordinates range from 0 to 1 in both edges and start at (0, 0) in
		* the lower left corner.
		* If isGUIOnMainMenu is false, then the GUI will only appear inside a game level.
		* WARNING. GUIElements can only be added when loading the game!
		* @param The GUIelement's name.
		* @param The position of the CENTER of the GUIelement in the X axis.
		* @param The position of the CENTER of the GUIelement in the Y axis.
		* @param How much is the GUIelement going to expand from its CENTER in the X axis in both directions.
		* @param How much is the GUIelement going to expand from its CENTER in the Y axis in both directions.
		*/
		static void addGUIBox(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
						      bool isEnabled = true, GUIcontainer container = GUIcontainer::level, const std::string & parentName = "",
							  unsigned int layer = 0);

		/**
		* @brief Screen coordinates range from 0 to 1 in both edges and start at (0, 0) in
		* the lower left corner.
		* WARNING. GUIElements can only be added when loading the game!
		*/
		static void addGUIButton(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
								 bool isEnabled = true, GUIcontainer container = GUIcontainer::level, const std::string& parentName = "",
								 unsigned int layer = 0);

		/**
		* @brief Draws all active GUI elements.
		* If "renderMainMenu" is equal to true, only GUIElements that correspond to the game level will be drawn if they are active.
		* If it is equal to false, only GUIElements that correspond to the main menu will be drawn if they are active.
		* WARNING. Must be called in a thread with valid graphics API context.
		* WARNING. Must be called with the graphic rendering mode set to 2D and depth test disabled.
		*/
		static void drawGUI(bool renderMainMenu = false);

		
		// Clean up.

		/**
		* @brief Deinitialise the system and clean any resources (heap memory, files...) allocated to it.
		*/
		static void reset();

	private:

		/*
		Attributes.
		*/

		static bool initialised_,
					someGUIChanged_,
			        skipMouseInputs_,
					levelGUIOpened_;
		static controlCode lastMouseButtonInput_;
		static unsigned int GUIElementCount_;
		static glm::mat4 projectionMatrix_;

		static std::unordered_map<unsigned int, GUIelement*> GUIElements_;
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
		static std::unordered_map<std::string, unsigned int> LevelGUIElementID_,
															 MMGUIElementID_;
		static window* window_;
		static GraphicsAPIWindow* windowAPIPointer_;

		static GUIelement* lastCheckedGUIElement_; // Has the last GUIElement checked by the processGUIInputs method. It's meant to reference, for example, a GUIButton inside
											       // it's own activation mouse button function.
		static vertexBuffer* vbo_;
		static vertexArray* vao_;
		static shader* shader_;


		/*
		Methods.
		*/

		static void changeGUIState(unsigned int GUIelementID, bool isEnabled);

		static void changeGUIState(unsigned int GUIelementID);
		
	};

	inline bool GUImanager::isMMGUIElementRegistered(const std::string& name) {
	
		return MMGUIElementID_.find(name) != MMGUIElementID_.cend();
	
	}

	inline bool GUImanager::isLevelGUIElementRegistered(const std::string& name) {

		return LevelGUIElementID_.find(name) != LevelGUIElementID_.cend();

	}

	inline bool GUImanager::levelGUIOpened() {
	
		return levelGUIOpened_;
	
	}

	inline GUIelement* GUImanager::lastCheckedGUIElement() {
	
		return lastCheckedGUIElement_;
	
	}

	inline void GUImanager::setLevelGUIOpened(bool value) {
	
		levelGUIOpened_ = value;
	
	}

	inline void GUImanager::switchLevelGUIOpened() {
	
		levelGUIOpened_ = !levelGUIOpened_;
	
	}

	inline bool GUImanager::someMMGUIChanged() {
	
		return someGUIChanged_;
	
	}

	inline bool GUImanager::someLevelGUIChanged() {
	
		return someGUIChanged_;
	
	}

	inline const GUIelement& GUImanager::cGetGUIElement(const std::string& name) {

		return getGUIElement(name);

	}

	inline void GUImanager::setMMGUIChanged(bool value) {
	
		someGUIChanged_ = value;
	
	}

	inline void GUImanager::setLevelGUIChanged(bool value) {
	
		someGUIChanged_ = value;
	
	}

}

#endif