#include "gui.h"
#include <cmath>
#include <stdexcept>
#include <string>
#include "vertex_buffer_layout.h"
#include "graphics.h"
#include "definitions.h"
#include "texture.h"
#include "logger.h"


namespace VoxelEng {

	/*
	GUIElement.
	*/

	const window* GUIElement::window_ = nullptr;


	GUIElement::GUIElement() 
		: enabled_(false), textureID_(0), KeyFuncPtr_(nullptr), MouseButtonFuncPtr_(nullptr), oldActivationEventReceived_(false), name_("Unnamed GUIElement"), GUIContainer_(GUIContainer::both) {}

	// Separate them in case someone in the future needs to have this two actions
	// in the same GUIElement.

	void GUIElement::executeKeyAction() {
	
		if (KeyFuncPtr_)
			KeyFuncPtr_();

	}

	void GUIElement::executeMouseButtonAction() {
	
		if (MouseButtonFuncPtr_)
			MouseButtonFuncPtr_();
	
	}


	/*
	GUIBox.
	*/ 
	class GUIBox : public GUIElement {

	public:


		// Constructors.

		/*
		Executes genVertexData().
		*/
		GUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled = true, GUIContainer container = GUIContainer::both,
			   unsigned int layer = 0);

	private:

		// Observers.

		bool getsInputFromTouch();


		// Modifiers.

		void genVertexData();

		void addTextures();

	};

	GUIBox::GUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled, GUIContainer container, unsigned int layer) {
		
		position_.x = posX;
		position_.y = posY;
		size_.x = sizeX;
		size_.y = sizeY;
		textureID_ = textureID;
		enabled_ = isEnabled;
		name_ = name;
		GUIContainer_ = container;
		GUILayer = layer;

		genVertexData();

	}

	void GUIBox::genVertexData() {

		vertices_.push_back({ position_.x , (position_.y + size_.y)});
		vertices_.push_back({ position_.x , position_.y });
		vertices_.push_back({ size_.x + position_.x , position_.y });
		vertices_.push_back({ size_.x + position_.x , (position_.y + size_.y) });
		vertices_.push_back({ position_.x , (position_.y + size_.y)});
		vertices_.push_back({ size_.x + position_.x , position_.y });

		addTextures();

	}

	void GUIBox::addTextures() {

		const std::pair<int, int>& textureHW = texture::getTextureWH(textureID_);
		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  texCoordX = (textureID_ % (int)(atlasWidth / MIN_TEX_RES)) / (atlasWidth / MIN_TEX_RES) - (MIN_TEX_RES / atlasWidth),
			  texCoordY = ceil(textureID_ / (atlasHeight / MIN_TEX_RES)) / (atlasHeight / MIN_TEX_RES);

		if (vertices_.size() == 6) {

			vertices_[0].textureCoords[0] = texCoordX;
			vertices_[0].textureCoords[1] = texCoordY;

			vertices_[1].textureCoords[0] = texCoordX + 0.005 / atlasWidth; // +0.005 / atlasWidth is used to correct some texture bleeding.
			vertices_[1].textureCoords[1] = texCoordY - (textureHW.second / atlasHeight);

			vertices_[2].textureCoords[0] = texCoordX + (textureHW.first / atlasWidth);
			vertices_[2].textureCoords[1] = texCoordY - (textureHW.second / atlasHeight);

			vertices_[3].textureCoords[0] = texCoordX + (textureHW.first / atlasWidth);
			vertices_[3].textureCoords[1] = texCoordY;

			vertices_[4].textureCoords[0] = texCoordX;
			vertices_[4].textureCoords[1] = texCoordY;

			vertices_[5].textureCoords[0] = texCoordX + (textureHW.first / atlasWidth);
			vertices_[5].textureCoords[1] = texCoordY - (textureHW.second / atlasHeight);

		}
		else
			logger::errorLog("Incorrect number of vertices when adding GUI's texture!");

	}

	bool GUIBox::getsInputFromTouch() {
	
		return false;
	
	}


	/*
	GUIButton.
	*/
	class GUIButton : public GUIBox {

	public:

		GUIButton(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled = true, GUIContainer container = GUIContainer::both,
				  unsigned int layer = 0);

	private:

		// Observers.

		bool getsInputFromTouch();

	};

	GUIButton::GUIButton(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, const std::string& name, bool isEnabled, GUIContainer container, unsigned int layer)
		: GUIBox(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer) {}


	bool GUIButton::getsInputFromTouch() {
	
		return true;
	
	}


	/*
	GUIManager.
	*/

	// Declarations.
	bool GUIManager::initialised_ = false, 
		 GUIManager::someGUIChanged_ = false,
		 GUIManager::skipMouseInputs_ = false,
		 GUIManager::levelGUIOpened_ = false;
	controlCode GUIManager::lastMouseButtonInput_ = controlCode::noKey;
	unsigned int GUIManager::GUIElementCount_ = 0;
	glm::mat4 GUIManager::projectionMatrix_;

	std::unordered_map<unsigned int, GUIElement*> GUIManager::GUIElements_;
    std::unordered_set<unsigned int> GUIManager::LevelActiveGUIElements_[N_GUI_LAYERS],
									 GUIManager::LevelInactiveGUIElements_[N_GUI_LAYERS],
									 GUIManager::LevelActiveGUIButtons_[N_GUI_LAYERS],
									 GUIManager::MMActiveGUIElements_[N_GUI_LAYERS],
									 GUIManager::MMInactiveGUIElements_[N_GUI_LAYERS], 
									 GUIManager::MMActiveGUIButtons_[N_GUI_LAYERS];
	std::unordered_map<unsigned int, controlCode> GUIManager::LevelBoundKey_,
												  GUIManager::MMBoundKey_,
												  GUIManager::boundMouseButton_;
	std::unordered_map<std::string, unsigned int> GUIManager::LevelGUIElementID,
											      GUIManager::MMGUIElementID;

	window* GUIManager::window_ = nullptr;
	GraphicsAPIWindow* GUIManager::windowAPIPointer_ = nullptr;
	GUIElement* GUIManager::lastCheckedGUIElement_ = nullptr;
	vertexBuffer* GUIManager::vbo_ = nullptr;
	vertexArray* GUIManager::vao_ = nullptr;
	shader* GUIManager::shader_ = nullptr;
	renderer* GUIManager::renderer_ = nullptr;
	

	// Methods.
	void GUIManager::init(window& window, shader& shader, renderer& renderer) {

		if (initialised_)
			logger::errorLog("GUI management system is already initialised");
		else {
		
			// Attributes' initialisation.
			float aspectRatio = window.aspectRatio();

			projectionMatrix_ = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
			shader_ = &shader;
			renderer_ = &renderer;
			window_ = &window;
			windowAPIPointer_ = window_->windowAPIpointer();
			vbo_ = new vertexBuffer();
			vao_ = new vertexArray();

			// Setup VBO, VAO and VAO layout.
			vbo_->bind();
			vao_->bind();

			vertexBufferLayout layout;

			layout.push<GLfloat>(2);
			layout.push<GLfloat>(2);

			vao_->addLayout(layout);

			// Initialize GUIElement attributes.
			GUIElement::window_ = window_;

			initialised_ = true;
		
		}

	}

	void GUIManager::drawGUI(bool renderMainMenu) {
	
		// Update projection matrix for GUI.
		shader_->setUniformMatrix4f("u_MVPGUI", projectionMatrix_);

		vbo_->bind();
		vao_->bind();

		size_t size = 0;
		GUIElement* element = nullptr;
		if (renderMainMenu)
			for (int i = 0; i < N_GUI_LAYERS; i++)
				for (auto it = MMActiveGUIElements_[i].cbegin(); it != MMActiveGUIElements_[i].cend(); it++) {

					element = GUIElements_[*it];

					size = sizeof(vertex2D) * element->nVertices();

					vbo_->prepareStatic(element->vertexData(), size);
					renderer_->draw2D(size);

				}
		else
			for (int i = 0; i < N_GUI_LAYERS; i++)
				for(auto it = LevelActiveGUIElements_[i].cbegin(); it != LevelActiveGUIElements_[i].cend(); it++) {

					element = GUIElements_[*it];

					size = sizeof(vertex2D) * element->nVertices();

					vbo_->prepareStatic(element->vertexData(), size);
					renderer_->draw2D(size);

				}
	
	}

	void GUIManager::updateOrthoMatrix() {
	
		float aspectRatio = window_->aspectRatio();

		projectionMatrix_ = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

	}

	void GUIManager::bindActKey(const std::string& GUIElementName, controlCode controlCode) {

		if (LevelGUIElementID.find(GUIElementName) == LevelGUIElementID.cend())
			if (MMGUIElementID.find(GUIElementName) == MMGUIElementID.cend())
				logger::errorLog("No GUIElement was found when binding activation key with name " + GUIElementName);
			else
				MMBoundKey_[MMGUIElementID[GUIElementName]] = controlCode;
		else
			LevelBoundKey_[LevelGUIElementID[GUIElementName]] = controlCode;
	
	}

	void GUIManager::bindActKeyFunction(const std::string& GUIElementName, void(*func)()) {
	
		unsigned int ID = 0;
		std::unordered_map<unsigned int, controlCode>::iterator itKey;
		if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			itKey = LevelBoundKey_.find(ID);

			if (itKey == LevelBoundKey_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid key or it's key was key::noKey. GUIElement name: " + GUIElementName);
			else
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend()) {

			ID = MMGUIElementID[GUIElementName];
			itKey = MMBoundKey_.find(ID);

			if (itKey == MMBoundKey_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid key or it's key was key::noKey. GUIElement name: " + GUIElementName);
			else
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else
			logger::errorLog("[ERROR]: No GUIElement was found when binding activation key function with name " + GUIElementName);
	
	}

	void GUIManager::bindActKeyFunction(const std::string& GUIElementName, void(*func)(), controlCode controlCode) {

		unsigned int ID = 0;
		if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			LevelBoundKey_[ID] = controlCode;

			GUIElement* element = GUIElements_[ID];

			if (element->GUIContainer_ == GUIContainer::both)
				MMBoundKey_[ID] = controlCode;

			if (controlCode == controlCode::noKey)
				logger::errorLog("A GUIElement cannot have a bound activation key function with it's activation key equal to key::noKey. GUIElement name: " + GUIElementName);
			else 
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend()) {

			ID = MMGUIElementID[GUIElementName];
			MMBoundKey_[ID] = controlCode;

			if (controlCode == controlCode::noKey)
				logger::errorLog("A GUIElement cannot have a bound activation key function with it's activation key equal to key::noKey. GUIElement name: " + GUIElementName);
			else 
				GUIElements_[ID]->KeyFuncPtr_ = func;

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUIManager::bindActMouseButton(const std::string& GUIElementName, controlCode mouseButton) {
	
		if (LevelGUIElementID.find(GUIElementName) == LevelGUIElementID.cend())
			if (MMGUIElementID.find(GUIElementName) == MMGUIElementID.cend())
				logger::errorLog("No GUIElement was found when binding activation key with name " + GUIElementName);
			else
				boundMouseButton_[MMGUIElementID[GUIElementName]] = mouseButton;
		else {
		
			unsigned int ID = LevelGUIElementID[GUIElementName];

			boundMouseButton_[ID] = mouseButton;

			// Avoid searching again in MMGUIElementID.
			GUIElement* element = GUIElements_[ID];
			if (element->enabled_ && element->GUIContainer_ == GUIContainer::both)
				MMActiveGUIButtons_[element->GUILayer].insert(ID);
		
		}
			
	
	}

	void GUIManager::bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)()) {

		unsigned int ID = 0;
		std::unordered_map<unsigned int, controlCode>::iterator itKey;
		GUIElement* element = nullptr;
		if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			itKey = boundMouseButton_.find(ID);

			if (itKey == boundMouseButton_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_) {

					LevelActiveGUIButtons_[element->GUILayer].insert(ID);

					// Avoid searching again in MMGUIElementID.
					if (element->GUIContainer_ == GUIContainer::both)
						MMActiveGUIButtons_[element->GUILayer].insert(ID);

				}

			}

		}
		else if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend()) {

			ID = MMGUIElementID[GUIElementName];
			itKey = boundMouseButton_.find(ID);

			if (itKey == boundMouseButton_.end() || itKey->second == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_)
					MMActiveGUIButtons_[element->GUILayer].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUIManager::bindActMouseButtonFunction(const std::string& GUIElementName, void(*func)(), controlCode mouseButton) {

		unsigned int ID = 0;
		GUIElement* element = nullptr;
		if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			boundMouseButton_[ID] = mouseButton;

			if (mouseButton == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_) {
				
					// Avoid searching again in MMGUIElementID.
					LevelActiveGUIButtons_[element->GUILayer].insert(ID);

					if (element->GUIContainer_ == GUIContainer::both)
						MMActiveGUIButtons_[element->GUILayer].insert(ID);
				
				}

			}

		}
		else if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend()) {

			ID = MMGUIElementID[GUIElementName];
			boundMouseButton_[ID] = mouseButton;

			if (mouseButton == controlCode::noKey)
				logger::errorLog("The specified GUIElement didn't have bound a valid mouse button or it was mouseButton::noButton. GUIElement name: " + GUIElementName);
			else {

				element = GUIElements_[ID];

				element->MouseButtonFuncPtr_ = func;

				if (element->enabled_)
					MMActiveGUIButtons_[element->GUILayer].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when binding activation key function with name " + GUIElementName);

	}

	void GUIManager::changeGUIState(const std::string& GUIElementName, bool isEnabled) {
	
		unsigned int ID = 0;
		GUIElement* element = nullptr;
		bool elementContainerIsBoth;
		if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend())
		{

			ID = MMGUIElementID[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_ && !isEnabled) {

				MMInactiveGUIElements_[element->GUILayer].insert(ID);
				MMActiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = false;

				elementContainerIsBoth = element->getGUIContainer() == GUIContainer::both;

				if (elementContainerIsBoth) { // This saves looking at the LevelGUIElementsID with find()
				
					LevelInactiveGUIElements_[element->GUILayer].insert(ID);
					LevelActiveGUIElements_[element->GUILayer].erase(ID);
				
				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer].erase(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer].erase(ID);

				}

			}
			else if (isEnabled) {

				MMActiveGUIElements_[element->GUILayer].insert(ID);
				MMInactiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = true;

				elementContainerIsBoth = element->getGUIContainer() == GUIContainer::both;

				if (elementContainerIsBoth) {
				
					LevelActiveGUIElements_[element->GUILayer].insert(ID) ;
					LevelInactiveGUIElements_[element->GUILayer].erase(ID);
				
				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer].insert(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer].insert(ID);

				}

			}

		}
		else if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_ && !isEnabled) {

				LevelInactiveGUIElements_[element->GUILayer].insert(ID);
				LevelActiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = false;

				if (element->getsInputFromTouch())
					LevelActiveGUIButtons_[element->GUILayer].erase(ID);

			}
			else if (isEnabled) {

				LevelActiveGUIElements_[element->GUILayer].insert(ID);
				LevelInactiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = true;

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
					LevelActiveGUIButtons_[element->GUILayer].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with name " + GUIElementName);

	}

	void GUIManager::changeGUIState(const std::string& GUIElementName) {

		unsigned int ID = 0;
		GUIElement* element = nullptr;
		bool elementContainerIsBoth;
		if (MMGUIElementID.find(GUIElementName) != MMGUIElementID.cend())
		{

			ID = MMGUIElementID[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_) {

				MMInactiveGUIElements_[element->GUILayer].insert(ID);
				MMActiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = false;

				elementContainerIsBoth = element->getGUIContainer() == GUIContainer::both;

				if (elementContainerIsBoth) { // This saves looking at the LevelGUIElementsID with find()

					LevelInactiveGUIElements_[element->GUILayer].insert(ID);
					LevelActiveGUIElements_[element->GUILayer].erase(ID);

				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {

					MMActiveGUIButtons_[element->GUILayer].erase(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer].erase(ID);

				}

			}
			else {

				MMActiveGUIElements_[element->GUILayer].insert(ID);
				MMInactiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = true;

				elementContainerIsBoth = element->getGUIContainer() == GUIContainer::both;

				if (elementContainerIsBoth) {

					LevelActiveGUIElements_[element->GUILayer].insert(ID);
					LevelInactiveGUIElements_[element->GUILayer].erase(ID);

				}

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_) {
				
					MMActiveGUIButtons_[element->GUILayer].insert(ID);

					if (elementContainerIsBoth)
						LevelActiveGUIButtons_[element->GUILayer].insert(ID);
				
				}
					

			}

		}
		else if (LevelGUIElementID.find(GUIElementName) != LevelGUIElementID.cend()) {

			ID = LevelGUIElementID[GUIElementName];
			element = GUIElements_[ID];

			someGUIChanged_ = true;

			if (element->enabled_) {

				LevelInactiveGUIElements_[element->GUILayer].insert(ID);
				LevelActiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = false;

				if (element->getsInputFromTouch())
					LevelActiveGUIButtons_[element->GUILayer].erase(ID);

			}
			else if (LevelInactiveGUIElements_[element->GUILayer].find(ID) != LevelInactiveGUIElements_[element->GUILayer].end()) {

				LevelActiveGUIElements_[element->GUILayer].insert(ID);
				LevelInactiveGUIElements_[element->GUILayer].erase(ID);
				element->enabled_ = true;

				if (element->getsInputFromTouch() && element->MouseButtonFuncPtr_)
					LevelActiveGUIButtons_[element->GUILayer].insert(ID);

			}

		}
		else
			logger::errorLog("No GUIElement was found when changing GUI state with name " + GUIElementName);

	}

	void GUIManager::processLevelGUIInputs() {
	
		bool activationEventReceived = false;
		double pointX, 
			   pointY;
		GUIElement* element = nullptr;
		vec2* pos = nullptr,
			* size = nullptr;


		for (auto it = LevelBoundKey_.cbegin(); it != LevelBoundKey_.cend(); it++) {

			element = GUIElements_[it->first];

			lastCheckedGUIElement_ = element;

			activationEventReceived = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(it->second)) == GLFW_PRESS;

			// This ensures that we only process the action once when the mouse button is pressed.
			if (!element->oldActivationEventReceived_ && activationEventReceived) {

				element->oldActivationEventReceived_ = activationEventReceived;
				element->executeKeyAction();

			}
			else
				element->oldActivationEventReceived_ = activationEventReceived;

		}
	

		if (skipMouseInputs_)
			skipMouseInputs_ = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(lastMouseButtonInput_)) == GLFW_PRESS;
		else // Search for only active GUIButtons, not for every GUIButton that has a mouse button bound.
			for (int i = 0; i < N_GUI_LAYERS; i++)
			{
				for (auto it = LevelActiveGUIButtons_[i].cbegin(); it != LevelActiveGUIButtons_[i].cend(); it++) {

					element = GUIElements_[*it];

					lastCheckedGUIElement_ = element;

					lastMouseButtonInput_ = boundMouseButton_[*it];
					activationEventReceived = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(boundMouseButton_[*it])) == GLFW_PRESS;

					pos = &element->position_;
					size = &element->size_;

					glfwGetCursorPos(windowAPIPointer_, &pointX, &pointY);

					pointX /= window_->width();
					pointY = 1 - (pointY / window_->height());

					// This ensures that we only process the action once when the mouse button is pressed.
					if (pointX > pos->x && pointX < pos->x + size->x && pointY > pos->y && pointY < pos->y + size->y &&
						!element->oldActivationEventReceived_ && activationEventReceived) {

						element->oldActivationEventReceived_ = activationEventReceived;
						element->executeMouseButtonAction();

						if (someGUIChanged_) // LevelActiveGUIButtons_'s iterators are invalidated if a new content is added or an already existing element is erased.
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

	void GUIManager::processMainMenuGUIInputs() {
	
		bool activationEventReceived = false;
		double pointX,
			   pointY;
		GUIElement* element = nullptr;
		vec2* pos = nullptr,
			* size = nullptr;


		for (auto it = MMBoundKey_.cbegin(); it != MMBoundKey_.cend(); it++) {

			element = GUIElements_[it->first];

			lastCheckedGUIElement_ = element;

			activationEventReceived = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(it->second)) == GLFW_PRESS;

			// This ensures that we only process the action once when the mouse button is pressed.
			if (!element->oldActivationEventReceived_ && activationEventReceived) {

				element->oldActivationEventReceived_ = activationEventReceived;
				element->executeKeyAction();

			}
			else
				element->oldActivationEventReceived_ = activationEventReceived;

		}


		if (skipMouseInputs_)
			skipMouseInputs_ = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(lastMouseButtonInput_)) == GLFW_PRESS;
		else // Search for only active GUIButtons (not for every GUIButton that has a mouse button bound).
			for (int i = 0; i < N_GUI_LAYERS; i++) {

				for (auto it = MMActiveGUIButtons_[i].cbegin(); it != MMActiveGUIButtons_[i].cend(); it++) {

					element = GUIElements_[*it];

					lastCheckedGUIElement_ = element;

					lastMouseButtonInput_ = boundMouseButton_[*it];
					activationEventReceived = glfwGetMouseButton(window_->windowAPIpointer(), controls::getGLFWControlCode(boundMouseButton_[*it])) == GLFW_PRESS;

					pos = &element->position_;
					size = &element->size_;

					glfwGetCursorPos(windowAPIPointer_, &pointX, &pointY);

					pointX /= window_->width();
					pointY = 1 - (pointY / window_->height());

					// This ensures that we only process the action once when the mouse button is pressed.
					if (pointX > pos->x && pointX < pos->x + size->x && pointY > pos->y && pointY < pos->y + size->y &&
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

	void GUIManager::addGUIBox(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
							   bool isEnabled, GUIContainer container, const std::string& parentName, unsigned int layer) {

		unsigned int ID;
		GUIBox* guiBox = new GUIBox(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer);


		ID = GUIElementCount_++;
		GUIElements_[ID] = guiBox;

		if (container == GUIContainer::mainMenu || container == GUIContainer::both) {

			if (MMGUIElementID.find(name) == MMGUIElementID.end()) {

				MMGUIElementID[name] = ID;

				if (isEnabled)
					MMActiveGUIElements_[layer].insert(ID);
				else
					MMInactiveGUIElements_[layer].insert(ID);

			}
			else {

				delete guiBox;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}
				

			if (parentName != "") {

				if (MMGUIElementID.find(parentName) != MMGUIElementID.end()) {

					if (isEnabled)
						GUIElements_[MMGUIElementID[parentName]]->children_.push_back(ID);
					else
						GUIElements_[MMGUIElementID[parentName]]->children_.push_back(ID);

				}
				else {
				
					delete guiBox;
					logger::errorLog("The specified GUIElement parent isn't registered in the main menu! GUIElement parent name " + parentName);
					
				
				}
					
			}
		
		}

		if (container == GUIContainer::level || container == GUIContainer::both) {

			if (LevelGUIElementID.find(name) == LevelGUIElementID.end()) {

				LevelGUIElementID[name] = ID;

				if (isEnabled)
					LevelActiveGUIElements_[layer].insert(ID);
				else
					LevelInactiveGUIElements_[layer].insert(ID);

			}
			else {

				delete guiBox;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}
				
			if (parentName != "") {

				if (LevelGUIElementID.find(parentName) != LevelGUIElementID.end()) {

					if (isEnabled)
						GUIElements_[LevelGUIElementID[parentName]]->children_.push_back(ID);
					else
						GUIElements_[MMGUIElementID[parentName]]->children_.push_back(ID);

				}
				else {
				
					delete guiBox;
					logger::errorLog("The specified GUIElement parent isn't registered in the level! GUIElement parent name " + parentName);
				
				}
					

			}
		
		}

	}
	
	void GUIManager::addGUIButton(const std::string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID,
									   bool isEnabled, GUIContainer container, const std::string& parentName, unsigned int layer) {

		unsigned int ID;
		GUIButton* guiButton = new GUIButton(posX, posY, sizeX, sizeY, textureID, name, isEnabled, container, layer);


		ID = GUIElementCount_++;
		GUIElements_[ID] = guiButton;

		if (container == GUIContainer::mainMenu || container == GUIContainer::both) {
		
			if (MMGUIElementID.find(name) == MMGUIElementID.end()) {

				MMGUIElementID[name] = ID;
				
				if (isEnabled)
					MMActiveGUIElements_[layer].insert(ID);
				else
					MMInactiveGUIElements_[layer].insert(ID);


			}
			else {
			
				delete guiButton;
				logger::errorLog("Duplicated name for GUIElement " + name);

			}
				
			if (parentName != "") {

				if (MMGUIElementID.find(parentName) != MMGUIElementID.end()) {
				
					if (isEnabled)
						GUIElements_[MMGUIElementID[parentName]]->children_.push_back(ID);
					else
						GUIElements_[MMGUIElementID[parentName]]->children_.push_back(ID);
				
				}
				else {
				
					delete guiButton;
					logger::errorLog("The specified GUIElement parent isn't registered in the main menu! GUIElement parent name " + parentName);
				
				}
	
			}
		
		}

		if (container == GUIContainer::level || container == GUIContainer::both) {

			if (LevelGUIElementID.find(name) == LevelGUIElementID.end()) {

				LevelGUIElementID[name] = ID;

				if (isEnabled)
					LevelActiveGUIElements_[layer].insert(ID);
				else
					LevelInactiveGUIElements_[layer].insert(ID);

			}
			else {
			
				delete guiButton;
				logger::errorLog("Duplicated name for GUIElement: " + name);
			
			}
				

			if (parentName != "") {

				if (LevelGUIElementID.find(parentName) != LevelGUIElementID.end()) {

					if (isEnabled)
						GUIElements_[LevelGUIElementID[parentName]]->children_.push_back(ID);
					else
						GUIElements_[LevelGUIElementID[parentName]]->children_.push_back(ID);

				}
				else {
				
					delete guiButton;
					logger::errorLog("The specified GUIElement parent isn't registered in the level! GUIElement parent name: " + parentName);
				
				}
					
			}
		
		}

	}

	void GUIManager::cleanUp() {

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

		LevelGUIElementID.clear();

		MMGUIElementID.clear();

		window_ = nullptr;

		windowAPIPointer_ = nullptr;

		lastCheckedGUIElement_ = nullptr;

		shader_ = nullptr;

		renderer_ = nullptr;

		if (vbo_)
			delete vbo_;

		if (vao_)
			delete vao_;

		initialised_ = false;

	}

}