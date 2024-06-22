#include "gui.h"
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include "entity.h"
#include "input.h"
#include "game.h"
#include "gameWindow.h"
#include "logger.h"
#include "renderer.h"
#include "utilities.h"
#include "vertexBufferLayout.h"
#include "Graphics/graphics.h"
#include "Graphics/Textures/texture.h"


namespace VoxelEng {

	/*
	GUIElement.
	*/

	const window* GUIelement::window_ = nullptr;


	GUIelement::GUIelement() 
		: enabled_(false), textureID_(0), KeyFuncPtr_(nullptr), MouseButtonFuncPtr_(nullptr),
		  oldActivationEventReceived_(false), name_("Unnamed GUIElement"), GUIContainer_(GUIcontainer::both), parent_(nullptr),
		  GUILayer_(1), originalPos_{ 0, 0 }, originalSize_{ 0, 0 }, truePos_{ 0, 0 }
	{}

	void GUIelement::changeTextureID(unsigned int newTextureID) {

		if (textureID_ != newTextureID) {
		
			textureID_ = newTextureID;

			genVertexData();
		
		}

	}

	void GUIelement::lockMutex() {
	
		GUIElementMutex_.lock();
	
	}

	void GUIelement::unlockMutex() {
	
		GUIElementMutex_.unlock();
	
	}

	void GUIelement::executeKeyAction() {
	
		if (KeyFuncPtr_)
			KeyFuncPtr_();

	}

	void GUIelement::executeMouseButtonAction() {
	
		if (MouseButtonFuncPtr_)
			MouseButtonFuncPtr_();
	
	}

	void GUIelement::applyAspectRatio(float aspectRatio) {

		if (aspectRatio > 1) {

			truePos_.x = originalPos_.x * aspectRatio;
			truePos_.y = originalPos_.y;

		}
		else {

			truePos_.x = originalPos_.x;
			truePos_.y = originalPos_.y / aspectRatio;

		}

	}

	/**
	* @brief Representation of a box as a GUIelement.
	*/
	class GUIbox : public GUIelement {

	public:


		// Constructors.

		/**
		* @brief Class constructor.
		*/
		GUIbox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled = true,
				GUIcontainer container = GUIcontainer::both, unsigned int layer = 0);

	private:

		// Friend classes.

		friend class GUImanager;

		// Observers.

		bool getsInputFromTouch();


		// Modifiers.

		void genVertexData();

		void addTextures();

	};

	GUIbox::GUIbox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled, 
		GUIcontainer container, unsigned int layer) {
		
		originalPos_.x = posX;
		originalPos_.y = posY;
		originalSize_.x = sizeX;
		originalSize_.y = sizeY;
		textureID_ = textureID;
		enabled_ = isEnabled;
		name_ = name;
		GUIContainer_ = container;
		GUILayer_ = layer;

	}

	void GUIbox::genVertexData() {

		vertices_.clear();
		float aspectRatio = window_->aspectRatio();

		if (aspectRatio > 1) {
		
			if (parent_) {
			
				const vec2& parentOrgPos = parent_->originalPos(),
							parentTruePos = parent_->truePos(),
					        parentOrgSize = parent_->originalSize();
				translatedPos_.x = translateRange(truePos_.x, 0.0f, aspectRatio, parentTruePos.x - parentOrgSize.x, parentTruePos.x + parentOrgSize.x);
				translatedPos_.y = translateRange(originalPos_.y, 0.0f, 1.0f, parentOrgPos.y - parentOrgSize.y, parentOrgPos.y + parentOrgSize.y);

			}
			else {

				translatedPos_.x = truePos_.x; // Used later in GUI processing functions.
				translatedPos_.y = originalPos_.y;

			}
		
		}
		else {

			if (parent_) {
				
				const vec2& parentOrgPos = parent_->originalPos(),
					 		parentTruePos = parent_->truePos(),
							parentOrgSize = parent_->originalSize();
				translatedPos_.x = translateRange(truePos_.x, 0.0f, 1.0f, parentOrgPos.x - parentOrgSize.x, parentOrgPos.x + parentOrgSize.x);
				translatedPos_.y = translateRange(truePos_.y, 0.0f, 1 / aspectRatio, parentTruePos.y - parentOrgSize.y, parentTruePos.y + parentOrgSize.y);

			}
			else {

				translatedPos_.x = originalPos_.x;
				translatedPos_.y = truePos_.y; // Also used later in GUI processing methods.

			}

		}

		vertices_.push_back({ (translatedPos_.x - originalSize_.x), translatedPos_.y + originalSize_.y });
		vertices_.push_back({ (translatedPos_.x - originalSize_.x), translatedPos_.y - originalSize_.y });
		vertices_.push_back({ (translatedPos_.x + originalSize_.x), translatedPos_.y - originalSize_.y });
		vertices_.push_back({ (translatedPos_.x + originalSize_.x), translatedPos_.y + originalSize_.y });
		vertices_.push_back({ (translatedPos_.x - originalSize_.x), translatedPos_.y + originalSize_.y });
		vertices_.push_back({ (translatedPos_.x + originalSize_.x), translatedPos_.y - originalSize_.y });

		addTextures();

	}

	void GUIbox::addTextures() {

		const std::pair<int, int>& textureWH = texture::getTextureWH(textureID_);
		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  texCoordX = (textureID_ % (int)(atlasWidth / MIN_TEX_RES)) / (atlasWidth / MIN_TEX_RES) - (MIN_TEX_RES / atlasWidth),
			  texCoordY = ceil(textureID_ / (atlasHeight / MIN_TEX_RES)) / (atlasHeight / MIN_TEX_RES);

		if (vertices_.size() == 6) {

			vertices_[0].textureCoords[0] = texCoordX;
			vertices_[0].textureCoords[1] = texCoordY;

			vertices_[1].textureCoords[0] = texCoordX + 0.005 / atlasWidth; // +0.005 / atlasWidth is used to correct some texture bleeding.
			vertices_[1].textureCoords[1] = texCoordY - (textureWH.second / atlasHeight);

			vertices_[2].textureCoords[0] = texCoordX + (textureWH.first / atlasWidth);
			vertices_[2].textureCoords[1] = texCoordY - (textureWH.second / atlasHeight);

			vertices_[3].textureCoords[0] = texCoordX + (textureWH.first / atlasWidth);
			vertices_[3].textureCoords[1] = texCoordY;

			vertices_[4].textureCoords[0] = texCoordX;
			vertices_[4].textureCoords[1] = texCoordY;

			vertices_[5].textureCoords[0] = texCoordX + (textureWH.first / atlasWidth);
			vertices_[5].textureCoords[1] = texCoordY - (textureWH.second / atlasHeight);

		}
		else
			logger::errorLog("Incorrect number of vertices when adding GUI's texture!");

	}

	bool GUIbox::getsInputFromTouch() {
	
		return false;
	
	}

	/**
	* @brief Representation of a button as a GUIelement.
	*/
	class GUIbutton : public GUIbox {

	public:

		// Constructors.

		/**
		* @brief Class constructor.7
		*/
		GUIbutton(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled = true, GUIcontainer container = GUIcontainer::both,
				  unsigned int layer = 0);

	private:

		// Friend classes.

		friend class GUImanager;

		// Observers.

		bool getsInputFromTouch();

	};

	GUIbutton::GUIbutton(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled, GUIcontainer container, unsigned int layer)
		: GUIbox(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer) {}


	bool GUIbutton::getsInputFromTouch() {
	
		return true;
	
	}


	/*
	'GUIManager' class.
	*/

	bool GUImanager::initialised_ = false, 
		 GUImanager::someGUIChanged_ = false,
		 GUImanager::skipMouseInputs_ = false,
		 GUImanager::levelGUIOpened_ = false;
	controlCode GUImanager::lastMouseButtonInput_ = controlCode::noKey;
	unsigned int GUImanager::GUIElementCount_ = 0;
	glm::mat4 GUImanager::projectionMatrix_;

	std::unordered_map<unsigned int, GUIelement*> GUImanager::GUIElements_;
    std::unordered_set<unsigned int> GUImanager::LevelActiveGUIElements_[N_GUI_LAYERS],
									 GUImanager::LevelInactiveGUIElements_[N_GUI_LAYERS],
									 GUImanager::LevelActiveGUIButtons_[N_GUI_LAYERS],
									 GUImanager::MMActiveGUIElements_[N_GUI_LAYERS],
									 GUImanager::MMInactiveGUIElements_[N_GUI_LAYERS], 
									 GUImanager::MMActiveGUIButtons_[N_GUI_LAYERS];
	std::unordered_map<unsigned int, controlCode> GUImanager::LevelBoundKey_,
												  GUImanager::MMBoundKey_,
												  GUImanager::boundMouseButton_;
	std::unordered_map<std::string, unsigned int> GUImanager::LevelGUIElementID_,
											      GUImanager::MMGUIElementID_;

	window* GUImanager::window_ = nullptr;
	GraphicsAPIWindow* GUImanager::windowAPIPointer_ = nullptr;
	GUIelement* GUImanager::lastCheckedGUIElement_ = nullptr;
	vertexBuffer* GUImanager::vbo_ = nullptr;
	vertexArray* GUImanager::vao_ = nullptr;
	shader* GUImanager::shader_ = nullptr;
	

	void GUImanager::init(window& window, shader& shader) {

		if (initialised_)
			logger::errorLog("GUI management system is already initialised");
		else {
		
			// Attributes' initialisation.

			window_ = &window;
			windowAPIPointer_ = window_->windowAPIpointer();
			updateOrthoMatrix();
			vbo_ = &graphics::vbo("GUI");
			vao_ = &graphics::vao("2D");
			shader_ = &shader;

			// Initialize GUIElement attributes.
			GUIelement::window_ = window_;

			initialised_ = true;
		
		}

	}

	GUIelement& GUImanager::getGUIElement(const std::string& name) {
	
		if (isLevelGUIElementRegistered(name))
			return *GUIElements_[LevelGUIElementID_[name]];
		else if (isMMGUIElementRegistered(name))
			return *GUIElements_[MMGUIElementID_[name]];
		else
			logger::errorLog("GUIElement " + name + " is not registered");
	
	}

	void GUImanager::drawGUI(bool renderMainMenu) {
	
		// Update projection matrix for GUI.
		shader_->setUniformMatrix4f("u_MVPGUI", projectionMatrix_);
		
		// Binding section.
		vao_->bind();
		vbo_->bind();

		size_t size = 0;
		GUIelement* element = nullptr;
		if (renderMainMenu)
			for (int i = 0; i < N_GUI_LAYERS; i++)
				for (auto it = MMActiveGUIElements_[i].cbegin(); it != MMActiveGUIElements_[i].cend(); it++) {

					element = GUIElements_[*it];

					element->lockMutex();

					size = element->nVertices();

					vbo_->prepareStatic(element->vertexData(), size * sizeof(vertex2D));
					renderer::draw2D(size);

					element->unlockMutex();

				}
		else
			for (int i = 0; i < N_GUI_LAYERS; i++)
				for(auto it = LevelActiveGUIElements_[i].cbegin(); it != LevelActiveGUIElements_[i].cend(); it++) {

					element = GUIElements_[*it];

					element->lockMutex();

					size = sizeof(vertex2D) * element->nVertices();

					vbo_->prepareStatic(element->vertexData(), size);
					renderer::draw2D(size);

					element->unlockMutex();

				}
	
	}

	void GUImanager::updateOrthoMatrix() {

		float aspectRatio = window_->aspectRatio();

		if (aspectRatio > 1)
			projectionMatrix_ = glm::ortho(0.0f, 1.0f * aspectRatio, 0.0f, 1.0f, -1.0f, 1.0f);
		else
			projectionMatrix_ = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f / aspectRatio, -1.0f, 1.0f);

		for (auto it = GUIElements_.begin(); it != GUIElements_.end(); it++) {
		
			it->second->applyAspectRatio(aspectRatio);
			it->second->genVertexData();
		
		}
			
	}

	void GUImanager::bindActKey(const std::string& GUIElementName, controlCode controlCode) {

		if (LevelGUIElementID_.find(GUIElementName) == LevelGUIElementID_.cend())
			if (MMGUIElementID_.find(GUIElementName) == MMGUIElementID_.cend())
				logger::errorLog("No GUIElement was found when binding activation key with name " + GUIElementName);
			else
				MMBoundKey_[MMGUIElementID_[GUIElementName]] = controlCode;
		else
			LevelBoundKey_[LevelGUIElementID_[GUIElementName]] = controlCode;
	
	}

	void GUImanager::bindActKeyFunction(const std::string& GUIElementName, void(*func)()) {
	
		unsigned int ID = 0;
		std::unordered_map<unsigned int, controlCode>::iterator itKey;
		if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			itKey = LevelBoundKey_.find(ID);

			if (itKey == LevelBoundKey_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid key or it's key was key::noKey. GUIElement name: " + GUIElementName);
			else
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];
			itKey = MMBoundKey_.find(ID);

			if (itKey == MMBoundKey_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid key or it's key was key::noKey. GUIElement name: " + GUIElementName);
			else
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else
			logger::errorLog("[ERROR]: No GUIElement was found when binding activation key function with name " + GUIElementName);
	
	}

	void GUImanager::bindActKeyFunction(const std::string& GUIElementName, void(*func)(), controlCode controlCode) {

		unsigned int ID = 0;
		if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			GUIelement* element = GUIElements_[ID];

			element->KeyFuncPtr_ = func;
			if (controlCode != controlCode::noKey) {
			
				LevelBoundKey_[ID] = controlCode;

				if (element->GUIContainer_ == GUIcontainer::both)
					MMBoundKey_[ID] = controlCode;
			
			}
			else {
			
				LevelBoundKey_.erase(ID);

				if (element->GUIContainer_ == GUIcontainer::both)
					MMBoundKey_.erase(ID);
			
			}
			

		}
		else if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];

			GUIElements_[ID]->KeyFuncPtr_ = func;
			if (controlCode != controlCode::noKey)
				MMBoundKey_[ID] = controlCode;
			else
				MMBoundKey_.erase(ID);

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUImanager::bindActMouseButton(const std::string& GUIElementName, controlCode mouseButton) {
	
		if (LevelGUIElementID_.find(GUIElementName) == LevelGUIElementID_.cend())
			if (MMGUIElementID_.find(GUIElementName) == MMGUIElementID_.cend())
				logger::errorLog("No GUIElement was found when binding activation key with name " + GUIElementName);
			else
				boundMouseButton_[MMGUIElementID_[GUIElementName]] = mouseButton;
		else {
		
			unsigned int ID = LevelGUIElementID_[GUIElementName];

			boundMouseButton_[ID] = mouseButton;

			// Avoid searching again in MMGUIElementID.
			GUIelement* element = GUIElements_[ID];
			if (element->enabled_ && element->GUIContainer_ == GUIcontainer::both)
				MMActiveGUIButtons_[element->GUILayer_].insert(ID);
		
		}
			
	
	}

	void GUImanager::bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)()) {

		unsigned int ID = 0;
		std::unordered_map<unsigned int, controlCode>::iterator itKey;
		GUIelement* element = nullptr;
		if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			itKey = boundMouseButton_.find(ID);

			if (itKey == boundMouseButton_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_) {

					LevelActiveGUIButtons_[element->GUILayer_].insert(ID);

					// Avoid searching again in MMGUIElementID.
					if (element->GUIContainer_ == GUIcontainer::both)
						MMActiveGUIButtons_[element->GUILayer_].insert(ID);

				}

			}

		}
		else if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];
			itKey = boundMouseButton_.find(ID);

			if (itKey == boundMouseButton_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_)
					MMActiveGUIButtons_[element->GUILayer_].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUImanager::bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)(), controlCode mouseButton) {

		unsigned int ID = 0;
		GUIelement* element = nullptr;
		if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			boundMouseButton_[ID] = mouseButton;

			if (mouseButton == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_) {
				
					// Avoid searching again in MMGUIElementID.
					LevelActiveGUIButtons_[element->GUILayer_].insert(ID);

					if (element->GUIContainer_ == GUIcontainer::both)
						MMActiveGUIButtons_[element->GUILayer_].insert(ID);
				
				}

			}

		}
		else if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];
			boundMouseButton_[ID] = mouseButton;

			if (mouseButton == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_)
					MMActiveGUIButtons_[element->GUILayer_].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUImanager::changeGUIState(const std::string& GUIElementName, bool isEnabled) {
	
		unsigned int ID = 0;
		GUIelement* element = nullptr;
		bool elementContainerIsBoth;
		if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_ && !isEnabled) {

				MMInactiveGUIElements_[element->GUILayer_].insert(ID);
				MMActiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = false;

				elementContainerIsBoth = element->getGUIContainer() == GUIcontainer::both;

				if (elementContainerIsBoth) { // This saves looking at the LevelGUIElementsID with find()
				
					LevelInactiveGUIElements_[element->GUILayer_].insert(ID);
					LevelActiveGUIElements_[element->GUILayer_].erase(ID);
				
				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer_].erase(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer_].erase(ID);

				}

			}
			else if (!element->enabled_ && isEnabled) {

				MMActiveGUIElements_[element->GUILayer_].insert(ID);
				MMInactiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = true;

				elementContainerIsBoth = element->getGUIContainer() == GUIcontainer::both;

				if (elementContainerIsBoth) {
				
					LevelActiveGUIElements_[element->GUILayer_].insert(ID) ;
					LevelInactiveGUIElements_[element->GUILayer_].erase(ID);
				
				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer_].insert(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer_].insert(ID);

				}

			}

			

		}
		else if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_ && !isEnabled) {

				LevelInactiveGUIElements_[element->GUILayer_].insert(ID);
				LevelActiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = false;

				if (element->getsInputFromTouch())
					LevelActiveGUIButtons_[element->GUILayer_].erase(ID);

			}
			else if (!element->enabled_ && isEnabled) {

				LevelActiveGUIElements_[element->GUILayer_].insert(ID);
				LevelInactiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = true;

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
					LevelActiveGUIButtons_[element->GUILayer_].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with name " + GUIElementName);

	}

	void GUImanager::changeGUIState(const std::string& GUIElementName) {

		unsigned int ID = 0;
		GUIelement* element = nullptr;
		bool elementContainerIsBoth;
		if (MMGUIElementID_.find(GUIElementName) != MMGUIElementID_.cend()) {

			ID = MMGUIElementID_[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_) {

				MMInactiveGUIElements_[element->GUILayer_].insert(ID);
				MMActiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = false;

				elementContainerIsBoth = element->getGUIContainer() == GUIcontainer::both;

				if (elementContainerIsBoth) { // This saves looking at the LevelGUIElementsID with find()

					LevelInactiveGUIElements_[element->GUILayer_].insert(ID);
					LevelActiveGUIElements_[element->GUILayer_].erase(ID);

				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer_].erase(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer_].erase(ID);

				}

			}
			else {

				MMActiveGUIElements_[element->GUILayer_].insert(ID);
				MMInactiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = true;

				elementContainerIsBoth = element->getGUIContainer() == GUIcontainer::both;

				if (elementContainerIsBoth) {

					LevelActiveGUIElements_[element->GUILayer_].insert(ID);
					LevelInactiveGUIElements_[element->GUILayer_].erase(ID);

				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {
				
					MMActiveGUIButtons_[element->GUILayer_].insert(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer_].insert(ID);
				
				}
					

			}

		}
		else if (LevelGUIElementID_.find(GUIElementName) != LevelGUIElementID_.cend()) {

			ID = LevelGUIElementID_[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_) {

				LevelInactiveGUIElements_[element->GUILayer_].insert(ID);
				LevelActiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = false;

				if (element->getsInputFromTouch())
					LevelActiveGUIButtons_[element->GUILayer_].erase(ID);

			}
			else if (LevelInactiveGUIElements_[element->GUILayer_].find(ID) != LevelInactiveGUIElements_[element->GUILayer_].end()) {

				LevelActiveGUIElements_[element->GUILayer_].insert(ID);
				LevelInactiveGUIElements_[element->GUILayer_].erase(ID);
				element->enabled_ = true;

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
					LevelActiveGUIButtons_[element->GUILayer_].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with name " + GUIElementName);

	}

	void GUImanager::processLevelGUIInputs() {

		bool activationEventReceived = false;
		double pointX,
			   pointY;
		GUIelement* element = nullptr;
		vec2 lowLeftCorner,
			 upperRightCorner;
		float aspectRatio = window_->aspectRatio();
		bool shouldProcess;

		for (auto it = LevelBoundKey_.cbegin(); it != LevelBoundKey_.cend(); it++) {

			element = GUIElements_[it->first];

			lastCheckedGUIElement_ = element;

			activationEventReceived = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(it->second)) == GLFW_PRESS;
			shouldProcess = input::shouldProcessInputs();

			// This ensures that we only process the action once when the mouse button is pressed.
			if (shouldProcess && !element->oldActivationEventReceived_ && activationEventReceived) {

				element->oldActivationEventReceived_ = activationEventReceived;
				element->executeKeyAction();

			}
			else
				element->oldActivationEventReceived_ = activationEventReceived;

		}

		if (lastMouseButtonInput_ != controlCode::noKey && skipMouseInputs_)
			skipMouseInputs_ = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(lastMouseButtonInput_)) == GLFW_PRESS;
		else // Search for only active GUIButtons, not for every GUIButton that has a mouse button bound.
			for (int i = 0; i < N_GUI_LAYERS; i++) {

				for (auto it = LevelActiveGUIButtons_[i].cbegin(); it != LevelActiveGUIButtons_[i].cend(); it++) {

					element = GUIElements_[*it];

					lastCheckedGUIElement_ = element;

					lastMouseButtonInput_ = boundMouseButton_[*it];
					activationEventReceived = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(boundMouseButton_[*it])) == GLFW_PRESS;
					shouldProcess = input::shouldProcessInputs();

					// Obtain the hitbox of the GUIelement.
					lowLeftCorner = element->translatedPos_;
					upperRightCorner = element->translatedPos_;

					lowLeftCorner.x -= element->originalSize_.x;
					lowLeftCorner.y -= element->originalSize_.y;
					upperRightCorner.x += element->originalSize_.x;
					upperRightCorner.y += element->originalSize_.y;

					// Obtain the user cursor's position on the graphics API window.
					// Also transform between the origin point that glfwGetCursorPos uses (upper left corner)
					// and the origin that we use (lower left corner).
					glfwGetCursorPos(windowAPIPointer_, &pointX, &pointY);
					pointX = pointX / window_->width();
					pointY = 1 - (pointY / window_->height());

					// Take into account the window's aspect ratio when getting the input coordinates.
					if (aspectRatio > 1)
						pointX *= aspectRatio;
					else
						pointY /= aspectRatio;


					// Only process the action once when the mouse button is properly pressed.
					if (shouldProcess && pointX > lowLeftCorner.x && pointX < upperRightCorner.x && pointY > lowLeftCorner.y && pointY < upperRightCorner.y &&
						!element->oldActivationEventReceived_ && activationEventReceived) {

						element->oldActivationEventReceived_ = activationEventReceived;
						element->executeMouseButtonAction();

						if (someGUIChanged_)
							break;

					}
					else
						element->oldActivationEventReceived_ = activationEventReceived;

					if (someGUIChanged_) {

						someGUIChanged_ = false;

						skipMouseInputs_ = true;

						break;

					}

				}

			}

	}

	void GUImanager::processMainMenuGUIInputs() {
	
		bool activationEventReceived = false;
		double pointX,
			   pointY;
		GUIelement* element = nullptr;
		vec2 lowLeftCorner,
			 upperRightCorner;
		float aspectRatio = window_->aspectRatio();
		bool shouldProcess;


		for (auto it = MMBoundKey_.cbegin(); it != MMBoundKey_.cend(); it++) {

			element = GUIElements_[it->first];

			lastCheckedGUIElement_ = element;

			activationEventReceived = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(it->second)) == GLFW_PRESS;
			shouldProcess = input::shouldProcessInputs();

			// This ensures that we only process the action once when the mouse button is pressed.
			if (shouldProcess && !element->oldActivationEventReceived_ && activationEventReceived) {

				element->oldActivationEventReceived_ = activationEventReceived;
				element->executeKeyAction();

			}
			else
				element->oldActivationEventReceived_ = activationEventReceived;

		}


		if (skipMouseInputs_ && lastMouseButtonInput_ != controlCode::noKey)
			skipMouseInputs_ = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(lastMouseButtonInput_)) == GLFW_PRESS;
		else // Search only for active GUIButtons (not for every GUIButton that has a mouse button bound).
			for (int i = 0; i < N_GUI_LAYERS; i++) {

				for (auto it = MMActiveGUIButtons_[i].cbegin(); it != MMActiveGUIButtons_[i].cend(); it++) {

					element = GUIElements_[*it];

					lastCheckedGUIElement_ = element;

					lastMouseButtonInput_ = boundMouseButton_[*it];
					activationEventReceived = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(boundMouseButton_[*it])) == GLFW_PRESS;
					shouldProcess = input::shouldProcessInputs();

					// Obtain the hitbox of the GUIelement.
					lowLeftCorner = element->translatedPos_;
					upperRightCorner = element->translatedPos_;

					lowLeftCorner.x -= element->originalSize_.x;
					lowLeftCorner.y -= element->originalSize_.y;
					upperRightCorner.x += element->originalSize_.x;
					upperRightCorner.y += element->originalSize_.y;

					// Obtain the user cursor's position on the graphics API window.
					// Also transform between the origin point that glfwGetCursorPos uses (upper left corner)
					// and the origin that we use (lower left corner).
					glfwGetCursorPos(windowAPIPointer_, &pointX, &pointY);
					pointX = pointX / window_->width();
					pointY = 1 - (pointY / window_->height());

					// Take into account the window's aspect ratio when getting the input coordinates.
					if (aspectRatio > 1)
						pointX *= aspectRatio;
					else
						pointY /= aspectRatio;

					// Only process the action once when the mouse button is properly pressed.
					if (shouldProcess && pointX > lowLeftCorner.x && pointX < upperRightCorner.x && pointY > lowLeftCorner.y && pointY < upperRightCorner.y &&
						!element->oldActivationEventReceived_ && activationEventReceived) {

						element->oldActivationEventReceived_ = activationEventReceived;
						element->executeMouseButtonAction();

						if (someGUIChanged_)
							break;

					}
					else
						element->oldActivationEventReceived_ = activationEventReceived;

				}

				if (someGUIChanged_) {

					someGUIChanged_ = false;

					skipMouseInputs_ = true;

					break;

				}

			}

	}

	void GUImanager::addGUIBox(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
							   bool isEnabled, GUIcontainer container, const std::string& parentName, unsigned int layer) {

		unsigned int ID;
		GUIbox* guiBox = new GUIbox(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer);
		float aspectRatio = window_->aspectRatio();

		ID = GUIElementCount_++;
		GUIElements_[ID] = guiBox;

		if (container == GUIcontainer::mainMenu || container == GUIcontainer::both) {

			if (MMGUIElementID_.find(name) == MMGUIElementID_.end()) {

				MMGUIElementID_[name] = ID;

				if (isEnabled)
					MMActiveGUIElements_[layer].insert(ID);
				else
					MMInactiveGUIElements_[layer].insert(ID);

				if (parentName != "") {

					auto it = MMGUIElementID_.find(parentName);
					if (it != MMGUIElementID_.end()) {

						guiBox->parent_ = GUIElements_[it->second];

						if (isEnabled)
							GUIElements_[MMGUIElementID_[parentName]]->children_.insert(ID);
						else
							GUIElements_[MMGUIElementID_[parentName]]->children_.insert(ID);

					}
					else {

						delete guiBox;
						logger::errorLog("The specified GUIElement parent isn't registered in the main menu! GUIElement parent name " + parentName);

					}

				}

			}
			else {

				delete guiBox;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}

		}

		if (container == GUIcontainer::level || container == GUIcontainer::both) {

			if (LevelGUIElementID_.find(name) == LevelGUIElementID_.end()) {

				LevelGUIElementID_[name] = ID;

				if (isEnabled)
					LevelActiveGUIElements_[layer].insert(ID);
				else
					LevelInactiveGUIElements_[layer].insert(ID);

				if (parentName != "") {

					auto it = LevelGUIElementID_.find(parentName);
					if (it != LevelGUIElementID_.end()) {

						guiBox->parent_ = GUIElements_[it->second];

						if (isEnabled)
							GUIElements_[LevelGUIElementID_[parentName]]->children_.insert(ID);
						else
							GUIElements_[MMGUIElementID_[parentName]]->children_.insert(ID);

					}
					else {

						delete guiBox;
						logger::errorLog("The specified GUIElement parent isn't registered in the level! GUIElement parent name " + parentName);

					}

				}

			}
			else {

				delete guiBox;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}

		}

		guiBox->applyAspectRatio(aspectRatio);

		guiBox->genVertexData();

	}
	
	void GUImanager::addGUIButton(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
									   bool isEnabled, GUIcontainer container, const std::string& parentName, unsigned int layer) {

		unsigned int ID;
		GUIbutton* guiButton = new GUIbutton(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer);

		ID = GUIElementCount_++;
		GUIElements_[ID] = guiButton;

		if (container == GUIcontainer::mainMenu || container == GUIcontainer::both) {
		
			if (MMGUIElementID_.find(name) == MMGUIElementID_.end()) {

				MMGUIElementID_[name] = ID;
				
				if (isEnabled)
					MMActiveGUIElements_[layer].insert(ID);
				else
					MMInactiveGUIElements_[layer].insert(ID);

				if (parentName != "") {

					auto it = MMGUIElementID_.find(parentName);
					if (it != MMGUIElementID_.end()) {

						guiButton->parent_ = GUIElements_[it->second];

						if (isEnabled)
							GUIElements_[MMGUIElementID_[parentName]]->children_.insert(ID);
						else
							GUIElements_[MMGUIElementID_[parentName]]->children_.insert(ID);

					}
					else {

						delete guiButton;
						logger::errorLog("The specified GUIElement parent isn't registered in the main menu! GUIElement parent name " + parentName);

					}

				}

			}
			else {
			
				delete guiButton;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}
		
		}

		if (container == GUIcontainer::level || container == GUIcontainer::both) {

			if (LevelGUIElementID_.find(name) == LevelGUIElementID_.end()) {

				LevelGUIElementID_[name] = ID;

				if (isEnabled)
					LevelActiveGUIElements_[layer].insert(ID);
				else
					LevelInactiveGUIElements_[layer].insert(ID);

				if (parentName != "") {

					auto it = LevelGUIElementID_.find(parentName);
					if (it != LevelGUIElementID_.end()) {

						guiButton->parent_ = GUIElements_[it->second];

						if (isEnabled)
							GUIElements_[LevelGUIElementID_[parentName]]->children_.insert(ID);
						else
							GUIElements_[LevelGUIElementID_[parentName]]->children_.insert(ID);

					}
					else {

						delete guiButton;
						logger::errorLog("The specified GUIElement parent isn't registered in the level! GUIElement parent name: " + parentName);

					}

				}

			}
			else {
			
				delete guiButton;
				logger::errorLog("Duplicated name for GUIElement: " + name);
			
			}

		}

		guiButton->applyAspectRatio(window_->aspectRatio());

		guiButton->genVertexData();

	}

	void GUImanager::reset() {

		for (auto it = GUIElements_.begin(); it != GUIElements_.end(); it++)
			delete it->second;
		GUIElements_.clear();

		LevelBoundKey_.clear();

		MMBoundKey_.clear();

		boundMouseButton_.clear();

		for (unsigned int i = 0; i < N_GUI_LAYERS; i++) {
		
			LevelActiveGUIElements_[i].clear();
			LevelInactiveGUIElements_[i].clear();
			MMActiveGUIElements_[i].clear();
			MMInactiveGUIElements_[i].clear();
			LevelActiveGUIButtons_[i].clear();
			MMActiveGUIButtons_[i].clear();
		
		}

		LevelGUIElementID_.clear();

		MMGUIElementID_.clear();

		window_ = nullptr;

		windowAPIPointer_ = nullptr;

		lastCheckedGUIElement_ = nullptr;

		shader_ = nullptr;

		vbo_ = nullptr;

		vao_ = nullptr;

		initialised_ = false;

	}

	void GUImanager::changeGUIState(unsigned int GUIelementID, bool isEnabled) {

		GUIelement* element = nullptr;
		GUIcontainer elementContainer;
		if (GUIElements_.find(GUIelementID) != GUIElements_.cend()) { // Main menu

			element = GUIElements_[GUIelementID];

			someGUIChanged_ = true;

			elementContainer = element->getGUIContainer();

			if (element->enabled_ && !isEnabled) {

				if (elementContainer == GUIcontainer::both) { // Both.
				
					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

					}
				
				}
				else if (elementContainer == GUIcontainer::level) { // Level only.

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}
				else { // Main menu only.
				
					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);
				
				}

				element->enabled_ = false;

			}
			else if (!element->enabled_ && isEnabled) {

				if (elementContainer == GUIcontainer::both) { // Both.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

					}

				}
				else if (elementContainer == GUIcontainer::level) { // Level only.

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}
				else { // Main menu only.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);



					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}

				element->enabled_ = true;

			}

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with ID " + GUIelementID);

	}

	void GUImanager::changeGUIState(unsigned int GUIelementID) {

		GUIelement* element = nullptr;
		GUIcontainer elementContainer;
		if (GUIElements_.find(GUIelementID) != GUIElements_.cend()) { // Main menu

			element = GUIElements_[GUIelementID];

			someGUIChanged_ = true;

			elementContainer = element->getGUIContainer();

			if (element->enabled_) {

				if (elementContainer == GUIcontainer::both) { // Both.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

					}

				}
				else if (elementContainer == GUIcontainer::level) { // Level only.

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}
				else { // Main menu only.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}

			}
			else {

				if (elementContainer == GUIcontainer::both) { // Both.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

					}

				}
				else if (elementContainer == GUIcontainer::level) { // Level only.

					LevelInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					LevelActiveGUIElements_[element->GUILayer_].erase(GUIelementID);

					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						LevelActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}
				else { // Main menu only.

					MMInactiveGUIElements_[element->GUILayer_].insert(GUIelementID);
					MMActiveGUIElements_[element->GUILayer_].erase(GUIelementID);



					// Manage GUIelements that get touch/mouse input.
					if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
						MMActiveGUIButtons_[element->GUILayer_].erase(GUIelementID);

				}

			}

			element->enabled_ = !element->enabled_;

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with ID " + GUIelementID);

	}

}