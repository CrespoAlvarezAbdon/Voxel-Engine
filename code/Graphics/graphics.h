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
#include "../batch.h" // CUIDADO POR SI ESTO METE ERRORES
#include "../indexBuffer.h"
#include "../vertexArray.h"
#include "../vertexBuffer.h"
#include "../vertexBufferLayout.h"
#include "../gameWindow.h"


namespace VoxelEng {	

	//////////
	//Enums.//
	//////////

	enum class textureType { NONE = 0, COLOR, DEPTH_AND_STENCIL, REVEAL, IMAGE };


	////////////
	//Classes.//
	////////////

	/**
	* @brief Represents the diffent color formats supported by the engine.
	* Only RGBA colour supported for now.
	*/
	class color {
	
	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		color(float red, float green, float blue, float alpha);


		// Observers.

		/**
		* @brief Returns the color's red component.
		*/
		float red() const;

		/**
		* @brief Returns the color's green component.
		*/
		float green() const;

		/**
		* @brief Returns the color's blue component.
		*/
		float blue() const;

		/**
		* @brief Returns the color's alpha component.
		*/
		float alpha() const;


		// Modifiers.

		/**
		* @brief Sets the color's red component.
		*/
		float& red();

		/**
		* @brief Sets the color's green component.
		*/
		float& green();

		/**
		* @brief Sets the color's blue component.
		*/
		float& blue();

		/**
		* @brief Sets the color's alpha component.
		*/
		float& alpha();

	private:

		float red_,
			  green_,
			  blue_,
			  alpha_;
	
	};

	inline float color::red() const {
	
		return red_;
	
	}

	inline float color::green() const {
	
		return green_;
	
	}

	inline float color::blue() const {
	
		return blue_;
	
	}

	inline float color::alpha() const {
	
		return alpha_;
	
	}

	inline float& color::red() {

		return red_;

	}

	inline float& color::green() {

		return green_;

	}

	inline float& color::blue() {

		return blue_;

	}

	inline float& color::alpha() {

		return alpha_;

	}


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

	inline void graphics::setVSync(bool isEnabled) {

		glfwSwapInterval(isEnabled);

	}

}

#endif