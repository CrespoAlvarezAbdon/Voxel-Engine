#include "gui.h"
#include <cmath>
#include <stdexcept>
#include "vertex_buffer_layout.h"
#include "graphics.h"
#include "definitions.h"
#include "texture.h"


namespace VoxelEng {


	/*
	GUIElement.
	*/

	std::vector<GUIElement*>* GUIElement::activeGUIElements_ = nullptr,
							* GUIElement::inactiveGUIElements_ = nullptr;
	const window* GUIElement::window_ = nullptr;

	


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

		void resizeElement();

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
	
		vertices_.push_back({ position_.x * GUIElement::window_->width(), (position_.y + size_.y) * GUIElement::window_->height() });
		vertices_.push_back({ position_.x * GUIElement::window_->width(), position_.y * GUIElement::window_->height() });
		vertices_.push_back({ (position_.x + size_.x) * GUIElement::window_->width(), position_.y * GUIElement::window_->height() });
		vertices_.push_back({ (position_.x + size_.x) * GUIElement::window_->width(), (position_.y + size_.y) * GUIElement::window_->height() });
		vertices_.push_back({ position_.x * GUIElement::window_->width(), (position_.y + size_.y) * GUIElement::window_->height() });
		vertices_.push_back({ (position_.x + size_.x) * GUIElement::window_->width(), position_.y * GUIElement::window_->height() });

		addTextures();

	}

	void GUIBox::addTextures() {

		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  textureWidth = texture::GUItexturesHW().at(textureID_).first,
			  textureHeight = texture::GUItexturesHW().at(textureID_).second,
			  texCoordX = (textureID_ % (int)(atlasWidth / MIN_TEX_RES)) / (atlasWidth / MIN_TEX_RES) - (MIN_TEX_RES / atlasWidth),
			  texCoordY = ceil(textureID_ / (atlasHeight / MIN_TEX_RES)) / (atlasHeight / MIN_TEX_RES);

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


	void GUIBox::resizeElement() {
	
		vertices_[0].positions[0] += 100;
	
	}

	/*
	GUIManager.
	*/

	// Declarations.
	glm::mat4 GUIManager::projectionMatrix_;
	std::vector<GUIElement*> GUIManager::activeGUIElements_,
							 GUIManager::inactiveGUIElements_;
	const window* GUIManager::window_ = nullptr;
	std::recursive_mutex GUIManager::mutex_;
	vertexBuffer* GUIManager::vbo_ = nullptr;
	vertexArray* GUIManager::vao_ = nullptr;
	shader* GUIManager::shader_ = nullptr;
	renderer* GUIManager::renderer_ = nullptr;


	// Methods.
	void GUIManager::initialize(const window& window, shader& shader, renderer& renderer) {

		// Attributes' initialisation.
		projectionMatrix_ = glm::ortho(0.0f, (float)window.width(), 0.0f, (float)window.height(), -1.0f, 1.0f);
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
		
			if ((*it)->isEnabled()) {
			
				size = sizeof(vertex2D) * (*it)->nVertices();

				vbo_->prepareStatic((*it)->vertexData(), size);
				renderer_->draw2D(size);
			
			}
		
		}

		// Undo changes that affect 3D rendering as it is the default rendering mode.
		VoxelEng::graphics::setDepthTest(true);
	
	}

	void GUIManager::updateAspectRatio() {
	
		projectionMatrix_ = glm::ortho(0.0f, (float)window_->width(), 0.0f, (float)window_->height(), -1.0f, 1.0f);

		for (auto it = activeGUIElements_.begin(); it != activeGUIElements_.end(); it++) {
		
			(*it)->resizeElement();
		
		}

	
	}

	void GUIManager::addGUIBox(float posX, float posY, float sizeX, float sizeY, unsigned int textureID, bool isEnabled) {

		activeGUIElements_.push_back(new GUIBox(posX, posY, sizeX, sizeY, textureID, isEnabled));
	
	}
	
}