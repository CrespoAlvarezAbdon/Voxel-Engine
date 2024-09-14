#include "graphics.h"

#include "Materials/materials.h"
#include "../logger.h"
#include "../definitions.h"
#include "../Registry/registries.h"
#include "../Registry/RegistryInsOrdered/registryInsOrdered.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif

namespace VoxelEng {

	// 'color' struct.

	color::color(float r, float g, float b, float a) {
	
		if (r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
			logger::errorLog("Atleast one of the channels defined in a 'VoxelEng::color' object is negative");
		else {

			red_ = r;
			green_ = g;
			blue_ = b;
			alpha_ = a;

		}
	
	}

	// 'graphics' class.

	bool graphics::initialised_ = false;
	window* graphics::mainWindow_ = nullptr;
	std::unordered_map<std::string, vertexBuffer> graphics::vbos_;
	std::unordered_map<std::string, vertexArray> graphics::vaos_;
	std::unordered_map<std::string, vertexBufferLayout> graphics::vboLayouts_;
	shader* graphics::opaqueShader_ = nullptr;
	shader* graphics::translucidShader_ = nullptr;
	shader* graphics::compositeShader_ = nullptr;
	shader* graphics::screenShader_ = nullptr;
	UBO<material>* graphics::materialsUBO_ = nullptr;


	void graphics::init(window& mainWindow) {

		if (initialised_)
			logger::errorLog("Graphics API is already initialised");
		else {

			#if GRAPHICS_API == OPENGL

				// GLFW initialization.
				if (!glfwInit())
					logger::errorLog("Failed to initialize the GLFW library!");

				// Select GLFW version.
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

				// Create API rendering window.
				mainWindow.windowAPIpointer() = glfwCreateWindow((int)mainWindow.width(), (int)mainWindow.height(), mainWindow.name().data(), NULL, NULL);

				if (!mainWindow.windowAPIpointer()) {

					glfwTerminate();
					logger::errorLog("Failed to create window named " + mainWindow.name());

				}

				mainWindow_ = &mainWindow;
				glfwMakeContextCurrent(mainWindow.windowAPIpointer());

				// With the previously created context, now we can initialize the GLEW library.
				if (glewInit() != GLEW_OK)
					logger::errorLog("GLEW_INIT_FAILED");

				graphics::setVSync(true);
				graphics::setDepthTest(true);
				graphics::setBasicFaceCulling(true);
				graphics::blending(true);

				// Initialise VBO, VAO and VBO layouts.

				vaos_.insert({ "3D", vertexArray()});
				vaos_.insert({ "3Dentities", vertexArray() });
				vaos_.insert({ "2D", vertexArray() });
				vaos_.insert({ "screen", vertexArray() });

				vbos_.insert({ "chunks", vertexBuffer() });
				vbos_.insert({ "entities", vertexBuffer() });
				vbos_.insert({ "GUI", vertexBuffer() });
				vbos_.insert({ "screen", vertexBuffer() });

				vaos_.at("3D").generate();
				vaos_.at("3Dentities").generate();
				vaos_.at("2D").generate();
				vaos_.at("screen").generate();
				vbos_.at("chunks").generate();
				vbos_.at("entities").generate();
				vbos_.at("GUI").generate();
				vbos_.at("screen").generate();

				vertexBufferLayout& layout3D = vboLayouts_.insert({ "3D", vertexBufferLayout() }).first->second,
								  & layout2D = vboLayouts_.insert({ "2D", vertexBufferLayout() }).first->second,
								  & layoutScreen = vboLayouts_.insert({ "screen", vertexBufferLayout() }).first->second;

				// Configure the vertex layout for 3D rendering.
				layout3D.push<GLfloat>(3, true);
				layout3D.push<GLfloat>(2, true);
				layout3D.push<unsigned char>(4, true);
				layout3D.push<unsigned char>(4, false);
				layout3D.push<normalVec>(1, false);
				vaos_.at("3D").bind();
				vbos_.at("chunks").bind();
				vaos_.at("3D").addLayout(layout3D);

				vaos_.at("3Dentities").bind();
				vbos_.at("entities").bind();
				vaos_.at("3Dentities").addLayout(layout3D);

				// The same for 2D rendering.
				layout2D.push<GLfloat>(2, true);
				layout2D.push<GLfloat>(2, true);
				vaos_.at("2D").bind();
				vbos_.at("GUI").bind();
				vaos_.at("2D").addLayout(layout2D);

				// The same for screen rendering.
				layoutScreen.push<GLfloat>(2, true);
				layoutScreen.push<GLfloat>(2, true);
				vaos_.at("screen").bind();
				vbos_.at("screen").bind();
				vaos_.at("screen").addLayout(layoutScreen);

				// Initialize shaders.
				opaqueShader_ = new shader("opaqueGeometry", "resources/Shaders/opaqueVertex.shader", "resources/Shaders/opaqueFragment.shader");
				translucidShader_ = new shader("translucidGeometry", "resources/Shaders/translucidVertex.shader", "resources/Shaders/translucidFragment.shader");
				compositeShader_ = new shader("composite", "resources/Shaders/compositeVertex.shader", "resources/Shaders/compositeFragment.shader");
				screenShader_ = new shader("screenQuad", "resources/Shaders/screenVertex.shader", "resources/Shaders/screenFragment.shader");

				// Initialize and link shaders' UBOs.
				materialsUBO_ = new UBO<material>("Materials", registries::materials(), 1); // binding point 1 is used for materials' UBO.
				opaqueShader_->bind();
				opaqueShader_->bindUFO(*materialsUBO_);
				opaqueShader_->unbind();

				translucidShader_->bind();
				translucidShader_->bindUFO(*materialsUBO_);
				translucidShader_->unbind();

			#else

				

			#endif

			initialised_ = true;
		
		}

	}

	void graphics::GLCheckErrors(std::ostream& os, const char* file, const char* function, unsigned int line) {

		bool errorsDetected = false;

		os << "[OpenGL error checking]" << std::endl;
		while (GLenum error = glGetError()) {

			errorsDetected = true;

			os << "Error: " << error << std::endl
				<< "at function " << function << ", instruction before line " << line << std::endl
				<< "in file " << file << std::endl << std::endl;

		}

		if (errorsDetected)
			abort();

	}

	const vertexBuffer& graphics::cVbo(const std::string& vboName) {

		if (vbos_.contains(vboName))
			return vbos_[vboName];
		else
			logger::errorLog(vboName + " is not a registered vertex buffer");

	}

	const vertexArray& graphics::cVao(const std::string& vaoName) {

		if (vaos_.contains(vaoName))
			return vaos_[vaoName];
		else
			logger::errorLog(vaoName + " is not a registered vertex array");

	}

	const vertexBufferLayout& graphics::cVboLayout(const std::string& vboLayoutName) {

		if (vboLayouts_.contains(vboLayoutName))
			return vboLayouts_[vboLayoutName];
		else
			logger::errorLog(vboLayoutName + " is not a registered vertex array");

	}

	void graphics::textureTypeToAPITextureType(const textureType& type, std::vector<unsigned int>& APIValues) {

		switch (type) {
		
		case textureType::NONE:
			logger::errorLog("No textureType was specified");
			break;

		case textureType::COLOR:
		case textureType::REVEAL:
			APIValues = {GL_COLOR};
			break;

		case textureType::DEPTH_AND_STENCIL:
			APIValues = {GL_DEPTH, GL_STENCIL};
			break;

		case textureType::IMAGE:
			logger::errorLog("Getting a graphics API's value for image texture type is unsupported");
			break;

		default:
			logger::errorLog("Unknown texture type " + std::to_string(static_cast<int>(type)));
			break;
		
		}
	
	}

	vertexBuffer& graphics::vbo(const std::string& vboName) {

		if (vbos_.contains(vboName))
			return vbos_[vboName];
		else
			logger::errorLog(vboName + " is not a registered vertex buffer");

	}

	vertexArray& graphics::vao(const std::string& vaoName) {

		if (vaos_.contains(vaoName))
			return vaos_[vaoName];
		else
			logger::errorLog(vaoName + " is not a registered vertex array");

	}

	vertexBufferLayout& graphics::vboLayout(const std::string& vboLayoutName) {

		if (vboLayouts_.contains(vboLayoutName))
			return vboLayouts_[vboLayoutName];
		else
			logger::errorLog(vboLayoutName + " is not a registered vertex array");

	}

	void graphics::setDepthTest(bool isEnabled) {

		if (isEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

	}

	void graphics::setBasicFaceCulling(bool isEnabled) {

		if (isEnabled) {

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

		}
		else
			glDisable(GL_CULL_FACE);

	}

	void graphics::blending(bool isEnabled) {

		if (isEnabled) {

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		}
		else
			glDisable(GL_BLEND);

	}

	void graphics::setOpaquePassConfig() {
	
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		graphics::blending(false);
	
	}

	void graphics::setTranslucidPassConfig() {

		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glBlendEquation(GL_FUNC_ADD);

	}

	void graphics::setCompositePassConfig() {

		glDepthFunc(GL_ALWAYS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void graphics::setScreenPassConfig() {

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE); // this way glClear will clear the depth buffer
		glDisable(GL_BLEND);

	}

	shader& graphics::opaqueShader() {

		if (initialised_) {

			return *opaqueShader_;

		}
		else {

			logger::errorLog("Graphics system is not initialised when accessing opaque shader");

		}

	}

	shader& graphics::translucidShader() {

		if (initialised_) {

			return *translucidShader_;

		}
		else {

			logger::errorLog("Graphics system is not initialised when accessing translucid shader");

		}

	}

	shader& graphics::compositeShader() {

		if (initialised_) {

			return *compositeShader_;

		}
		else {

			logger::errorLog("Graphics system is not initialised when accessing composite shader");

		}

	}

	shader& graphics::screenShader() {

		if (initialised_) {

			return *screenShader_;

		}
		else {

			logger::errorLog("Graphics system is not initialised when accessing screen shader");

		}

	}
	
	void graphics::reset() {
	
		if (initialised_) {
		
			#if GRAPHICS_API == OPENGL

				vbos_.clear();
				vaos_.clear();
				vboLayouts_.clear();
				glfwTerminate();

			#else



			#endif

			mainWindow_ = nullptr;

			initialised_ = false;

			if (opaqueShader_) {
			
				delete opaqueShader_;
				opaqueShader_ = nullptr;
			
			}

			if (translucidShader_) {

				delete translucidShader_;
				translucidShader_ = nullptr;

			}

			if (compositeShader_) {

				delete compositeShader_;
				compositeShader_ = nullptr;

			}
			if (screenShader_) {

				delete screenShader_;
				screenShader_ = nullptr;

			}
		
		}
		else {

			logger::errorLog("Graphics system is not initialised");

		}

	}

}