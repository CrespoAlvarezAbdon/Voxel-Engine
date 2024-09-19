/**
* @file graphics.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Engine Graphics API.
* @brief Contains the engine's graphics API, which is in charge
* of abstracting the details of the actual graphics API used
* by the engine among other auxiliar data structures.
*/
#ifndef _VOXELENG_GRAPHICS_
#define _VOXELENG_GRAPHICS_

#include <atomic>
#include <initializer_list>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "Shaders/shader.h"
#include "Materials/materials.h"
#include "UBOs/UBOs.h"
#include "../batch.h"
#include "../indexBuffer.h"
#include "../vertexArray.h"
#include "../vertexBuffer.h"
#include "../gameWindow.h"
#include "Vertex/VertexBufferLayout/vertexBufferLayout.h"

namespace VoxelEng {	

	//////////
	//Enums.//
	//////////

	enum class textureType { NONE = 0, COLOR, DEPTH_AND_STENCIL, REVEAL, IMAGE };


	////////////
	//Classes.//
	////////////

	/**
	* @brief Class used to abstract the graphics operations independently
	* of the underlying graphics API (directX, OpenGL...) that is
	* being used.
	* WARNING. All calls to methods from this class must be made in a thread with valid graphics API context.
	*/
	class graphics {
	
	public:

		// Initialisers.

		/**
		* @brief Initializes the underlying graphics APIs/libraries that
		* are used by the engine.
		*/
		static void init(window& renderingWindow);


		// Observers.

		/**
		* @brief Gets the pointer to the 'VoxelEng::window' class object that is being used
		* to handle the game window's callbacks..
		*/
		static window* getMainWindow();

		/**
		* @brief Returns true if the engine graphics API is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Clear all previous thrown graphics API errors prior to this function's call.
		*/
		static void eraseErrors();

		/**
		* @brief Obtain the graphics API's last errors.
		* The last three parameters should be used to help locate the error when debugging.
		*/
		static void GLCheckErrors(std::ostream& os, const char* file, const char* function, unsigned int line);

		/**
		* @brief Get a previously registered vertex buffer.
		*/
		static const vertexBuffer& cVbo(const std::string& vboName);

		/**
		* @brief Get a previously registered vertex array.
		*/
		static const vertexArray& cVao(const std::string& vboName);

		/**
		* @brief Get a previously registered vertex buffer layout.
		*/
		static const vertexBufferLayout& cVboLayout(const std::string& vboName);

		/**
		* @brief Get the corresponding textureType value to the underlying graphics API texture type values.
		*/
		static void textureTypeToAPITextureType(const textureType& type, std::vector<unsigned int>& APIValues);

		/**
		* @brief Get the shader used for opaque geometry.
		*/
		static const shader& cOpaqueShader();

		/**
		* @brief Get the shader used for translucid geometry.
		*/
		static const shader& cTranslucidShader();

		/**
		* @brief Get the shader used for compositing the opaque and translucid rendering passes.
		*/
		static const shader& cCompositeShader();

		/**
		* @brief Get the shader used for rendering the game's screen quad.
		*/
		static const shader& cScreenShader();


		// Modifiers.

		/**
		* @brief Get a previously registered vertex buffer.
		*/
		static vertexBuffer& vbo(const std::string& vboName);

		/**
		* @brief Get a previously registered vertex array.
		*/
		static vertexArray& vao(const std::string& vboName);

		/**
		* @brief Get a previously registered vertex buffer layout.
		*/
		static vertexBufferLayout& vboLayout(const std::string& vboName);

		/**
		* @brief Set Vertical Synchronization (VSync) on (true) or off (false).
		*/
		static void setVSync(bool isEnabled);

		/**
		* @brief Set Depth test on (true) or off (false).
		*/
		static void setDepthTest(bool isEnabled);

		/**
		* @brief Set basic face culling on (true) or off (false).
		*/
		static void setBasicFaceCulling(bool isEnabled);

		/**
		* @brief Set blending on (true) or off (false).
		*/
		static void blending(bool isEnabled);

		/**
		* @brief Configure the graphics API for the opaque rendering pass.
		*/
		static void setOpaquePassConfig();

		/**
		* @brief Configure the graphics API for the translucid rendering pass.
		*/
		static void setTranslucidPassConfig();

		/**
		* @brief Configure the graphics API for the composite rendering pass.
		*/
		static void setCompositePassConfig();

		/**
		* @brief Configure the graphics API for the screen rendering pass.
		*/
		static void setScreenPassConfig();

		/**
		* @brief Get the shader used for opaque geometry.
		*/
		static shader& opaqueShader();

		/**
		* @brief Get the shader used for translucid geometry.
		*/
		static shader& translucidShader();

		/**
		* @brief Get the shader used for compositing the opaque and translucid rendering passes.
		*/
		static shader& compositeShader();

		/**
		* @brief Get the shader used for rendering the game's screen quad.
		*/
		static shader& screenShader();


		// Clean up.

		/**
		* @brief Deallocate any heap memory used by the engine graphics API and shut it down.
		*/
		static void reset();

	private:

		static bool initialised_;
		static window* mainWindow_;
		static std::unordered_map<std::string, vertexBuffer> vbos_;
		static std::unordered_map<std::string, vertexArray> vaos_;
		static std::unordered_map<std::string, vertexBufferLayout> vboLayouts_;
		static shader* opaqueShader_;
		static shader* translucidShader_;
		static shader* compositeShader_;
		static shader* screenShader_;
		static UBO<material>* materialsUBO_;
		
	};

	inline void graphics::eraseErrors() {

		while (glGetError());

	}

	inline window* graphics::getMainWindow() {
	
		return mainWindow_;
	
	}

	inline bool graphics::initialised() {
	
		return initialised_;
	
	}

	inline const shader& graphics::cOpaqueShader() {

		return opaqueShader();

	}

	inline const shader& graphics::cTranslucidShader() {

		return translucidShader();

	}

	inline const shader& graphics::cCompositeShader() {

		return compositeShader();

	}

	inline const shader& graphics::cScreenShader() {

		return screenShader();

	}

	inline void graphics::setVSync(bool isEnabled) {

		glfwSwapInterval(isEnabled);

	}

}

#endif