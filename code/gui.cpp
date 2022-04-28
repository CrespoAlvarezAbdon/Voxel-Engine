#include "gui.h"
#include <cmath>
#include <stdexcept>
#include <string>
#include "vertex_buffer_layout.h"
#include "graphics.h"
#include "definitions.h"
#include "texture.h"


namespace VoxelEng {


	/*
	GUIElement.
	*/

	std::unordered_map<unsigned int, GUIElement*>* GUIElement::activeGUIElements_ = nullptr,
												 * GUIElement::inactiveGUIElements_ = nullptr;
	const window* GUIElement::window_ = nullptr;

	GUIElement::GUIElement() 
		: enabled_(false), textureID_(0), children_(nullptr), KeyFuncPtr(nullptr), actKeyPressed_(false) {}

	void GUIElement::executeKeyAction() {
	
		if (KeyFuncPtr)
			KeyFuncPtr();

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
		GUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, bool isEnabled = true);

	private:

		// Other methods.

		void genVertexData();

		void addTextures();

	};

	GUIBox::GUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, bool isEnabled) {
		

		position_.x = posX;
		position_.y = posY;
		size_.x = sizeX;
		size_.y = sizeY;
		textureID_ = textureID;
		enabled_ = isEnabled;

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

		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  textureWidth = texture::GUItexturesHW().at(textureID_).first,
			  textureHeight = texture::GUItexturesHW().at(textureID_).second,
			  texCoordX = (textureID_ % (int)(atlasWidth / MIN_TEX_RES)) / (atlasWidth / MIN_TEX_RES) - (MIN_TEX_RES / atlasWidth),
			  texCoordY = ceil(textureID_ / (atlasHeight / MIN_TEX_RES)) / (atlasHeight / MIN_TEX_RES);

		// Texture bleeding correction.
		// Basically avoid using the edge coordinates of a texel.
		// A texel is the basic minimal unit of a texture.
		// There can be one or more pixels representing a texel.
		texCoordX = texCoordX + 0.05 / atlasWidth;
		texCoordY = texCoordY + 0.1 / atlasHeight;

		if (vertices_.size() == 6) {

			vertices_[0].textureCoords[0] = texCoordX;
			vertices_[0].textureCoords[1] = texCoordY;

			vertices_[1].textureCoords[0] = texCoordX;
			vertices_[1].textureCoords[1] = texCoordY - (textureHeight / atlasHeight);

			vertices_[2].textureCoords[0] = texCoordX + (textureWidth / atlasWidth);
			vertices_[2].textureCoords[1] = texCoordY - (textureHeight / atlasHeight);

			vertices_[3].textureCoords[0] = texCoordX + (textureWidth / atlasWidth);
			vertices_[3].textureCoords[1] = texCoordY;

			vertices_[4].textureCoords[0] = texCoordX;
			vertices_[4].textureCoords[1] = texCoordY;

			vertices_[5].textureCoords[0] = texCoordX + (textureWidth / atlasWidth);
			vertices_[5].textureCoords[1] = texCoordY - (textureHeight / atlasHeight);

		}
		else {

			throw runtime_error("[ERROR]: Incorrect number of vertices when adding GUI's texture!");

		}

	}

	/*
	GUIManager.
	*/

	// Declarations.
	glm::mat4 GUIManager::projectionMatrix_;
	std::unordered_map<unsigned int, GUIElement*> GUIManager::activeGUIElements_,
												  GUIManager::inactiveGUIElements_;
	window* GUIManager::window_ = nullptr;
	vertexBuffer* GUIManager::vbo_ = nullptr;
	vertexArray* GUIManager::vao_ = nullptr;
	shader* GUIManager::shader_ = nullptr;
	renderer* GUIManager::renderer_ = nullptr;
	std::unordered_map<unsigned int, key> GUIManager::boundKey_;
	std::unordered_map<string, unsigned int> GUIManager::GUIElementID;


	// Methods.
	void GUIManager::initialize(window& window, shader& shader, renderer& renderer) {

		// Attributes' initialisation.
		float aspectRatio = window.aspectRatio();

		projectionMatrix_ = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
		shader_ = &shader;
		renderer_ = &renderer;
		window_ = &window;
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
		GUIElement::activeGUIElements_ = &activeGUIElements_;
		GUIElement::inactiveGUIElements_ = &inactiveGUIElements_;
		GUIElement::window_ = window_;

	}

	void GUIManager::drawGUI() {
	
		// Update projection matrix for GUI.
		shader_->setUniformMatrix4f("u_MVPGUI", projectionMatrix_);

		shader_->setUniform1i("u_renderMode", 1); // renderMode = 1 stands for 2d render mode.
		VoxelEng::graphics::setDepthTest(false);

		vbo_->bind();
		vao_->bind();

		size_t size = 0;
		for (auto it = activeGUIElements_.cbegin(); it != activeGUIElements_.cend(); it++) {

			size = sizeof(vertex2D) * it->second->nVertices();

			vbo_->prepareStatic(it->second->vertexData(), size);
			renderer_->draw2D(size);

		}

		// Undo changes that affect 3D rendering as it is the default rendering mode.
		VoxelEng::graphics::setDepthTest(true);
	
	}

	void GUIManager::updateOrthoMatrix() {
	
		float aspectRatio = window_->aspectRatio();

		projectionMatrix_ = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

	}

	void GUIManager::bindActKey(unsigned int ID, key key) {

		boundKey_[ID] = key;
	
	}

	void GUIManager::bindActKeyFunction(unsigned int ID, void(*func)()) {
	
		if (activeGUIElements_.find(ID) != activeGUIElements_.end())
			activeGUIElements_[ID]->KeyFuncPtr = func;
		else if(inactiveGUIElements_.find(ID) != inactiveGUIElements_.end())
			inactiveGUIElements_[ID]->KeyFuncPtr = func;
		else
		{

			string errorMsg("[ERROR]:No GUIElement with ID " + ID);
			errorMsg += " was found when binding activation key function!";
			throw std::runtime_error(errorMsg);

		}
	
	}

	void GUIManager::changeGUIState(unsigned int ID, bool isEnabled) {
	

		if (activeGUIElements_.find(ID) != activeGUIElements_.end() && !isEnabled) {
		
			inactiveGUIElements_[ID] = activeGUIElements_[ID];
			activeGUIElements_.erase(ID);
		
		}
		else if (inactiveGUIElements_.find(ID) != inactiveGUIElements_.end() && isEnabled) {
		
			activeGUIElements_[ID] = inactiveGUIElements_[ID];
			inactiveGUIElements_.erase(ID);
		
		}
	
	}

	void GUIManager::changeGUIState(unsigned int ID) {

		if (activeGUIElements_.find(ID) != activeGUIElements_.end()) {

			inactiveGUIElements_[ID] = activeGUIElements_[ID];
			activeGUIElements_.erase(ID);

		}
		else if (inactiveGUIElements_.find(ID) != inactiveGUIElements_.end()) {

			activeGUIElements_[ID] = inactiveGUIElements_[ID];
			inactiveGUIElements_.erase(ID);

		}

	}

	void GUIManager::processGUIInputs() {
	
		
		bool keyPressed = false;
		for (auto it = boundKey_.cbegin(); it != boundKey_.cend(); it++) {

			const auto& guiElement = activeGUIElements_.find(it->first);

			if (guiElement != activeGUIElements_.end()) {

				keyPressed = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWKey(it->second)) == GLFW_PRESS;

				if (!guiElement->second->actKeyPressed_ && keyPressed) {

					guiElement->second->actKeyPressed_ = keyPressed;
					guiElement->second->executeKeyAction();

				}
				else
					guiElement->second->actKeyPressed_ = keyPressed;

			}
			else {
			
				const auto& inactiveGuiElement = inactiveGUIElements_.find(it->first);

				if (inactiveGuiElement != inactiveGUIElements_.end()) {

					keyPressed = glfwGetKey(window_->windowAPIpointer(), controls::getGLFWKey(it->second)) == GLFW_PRESS;

					if (!inactiveGuiElement->second->actKeyPressed_ && keyPressed) {

						inactiveGuiElement->second->actKeyPressed_ = keyPressed;
						inactiveGuiElement->second->executeKeyAction();

					}
					else
						inactiveGuiElement->second->actKeyPressed_ = keyPressed;

				}

			}
		
		}
	
	}

	unsigned int GUIManager::addGUIBox(const string& name, float posX, float posY, float sizeX, float sizeY, unsigned int textureID, bool isEnabled) {

		unsigned int ID = activeGUIElements_.size();

		// GUI's aren't created or removed ingame so activeGUIElements_.size() can be used to assign a new ID safely.
		if (GUIElementID.find(name) == GUIElementID.end()) {

			GUIElementID[name] = ID;
			activeGUIElements_[ID] = new GUIBox(posX, posY, sizeX, sizeY, textureID, isEnabled);

		}
		else
			throw std::runtime_error("Duplicated name for GUIElement: " + name);
		

		return ID;
	
	}
	
}