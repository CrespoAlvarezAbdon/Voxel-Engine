/**
* @file model.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Entity models.
* @brief Contains the definition of entity model as well
* as the declaration of the "models" class, used to provide
* all utilities required for managing the models that represent
* the entities inside a level.
*/
#ifndef _VOXELENG_MODEL_
#define _VOXELENG_MODEL_

#include <string>
#include <vector>
#include <unordered_map>
#include "vertex.h"
#include "block.h"
#include "definitions.h"


namespace VoxelEng {

	/////////////////////
	//Type definitions.//
	/////////////////////

	/**
	* @brief An entity's model is simply a ordered collection
	* of vertices, so that the first one connects with the next, and
	* that one with the next one, etc... in order for the engine
	* to properly draw it.
	*/
	typedef std::vector<vertex> model;


	/**
	* @brief A triangle is a collection of three vertex indices that
	* correspond to the ones that form it.
	*/
	typedef std::vector<unsigned short> triangle;
	/**
	* @brief Representation of the collection of triangles that forms up
	* an entity's model.
	*/
	typedef std::vector<triangle> modelTriangles;


	///////////
	//Classes//
	///////////

	/**
	* @brief Provides all utilities required for managing the models that represent
	* the entities inside a level.
	*/
	class models {

	public:

		// Initialisers.

		/**
		* @brief Initialise the static members of this class and allocate any
		* resources that are needed on initialisation.
		*/
		static void init();


		// Modifiers.

		/**
		* @brief Use a custom model ingame, created using the
		* .OBJ format. Two models cannot have the same model ID.
		* Using an already used ID will replace the old model with the new one.
		*/
		static void loadCustomModel(const std::string& filePath, unsigned int modelID);

		/**
		* @brief Get the corresponding model for a certain ID.
		* If no model exists for such ID, it returns the basic empty model with ID 0.
		*/
		static const model& getModelAt(unsigned int modelID);

		/**
		* @brief Get the corresponding model triangles indices for a certain ID.
		* If no model exists for such ID, it returns the basic empty model triangles.
		*/
		static const modelTriangles& getModelTrianglesAt(unsigned int modelID);

		/**
		* @brief Get the corresponding model triangles indices for a certain ID.
		* If no model exists for such ID, it returns the basic empty model triangles.
		*/
		static const modelTriangles& getModelTriangles(unsigned int modelID);

		/**
		* @brief Add a specified texture for a face of a terrain block.
		* WARNING. Only call when just all the face's vertices have been added
		* to the model.
		*/
		static void addTexture(const block& block, unsigned int textureID, model& m);

		/**
		* @brief Add a texture for a face of a terrain block. The added texture is 
		* the one corresponding to the specified block.
		* WARNING. Only call when just all the face's vertices have been added
		* to the model.
		*/
		static void addTexture(const block& block, model& m);


		// Clean up.

		/**
		* @brief Free any resources allocated to the static members of this class
		* and deinitialise it.
		*/
		static void cleanUp();

	private:

		static bool initialised_;
		static std::unordered_map<unsigned int, model*> models_;
		static std::unordered_map<unsigned int, modelTriangles*> triangles_;

	};

	inline const modelTriangles& models::getModelTriangles(unsigned int modelID) {

		return *triangles_[modelID];

	}

}

#endif