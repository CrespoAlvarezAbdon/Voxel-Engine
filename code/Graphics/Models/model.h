/**
* @file model.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Models.
* @brief Contains the definition of model as well
* as the declaration of the "models" class, used to provide
* all utilities required for managing them.
*/
#ifndef _VOXELENG_MODEL_
#define _VOXELENG_MODEL_

#include <string>
#include <vector>
#include <unordered_map>
#include "../../vertex.h"
#include "../../block.h"
#include "../../definitions.h"
#include "../../Graphics/transform.h"


namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int planeModelID = 3;


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
	* @brief A vertex normal is a vector used for lighting calculations in the shaders.
	*/
	typedef vec3 normal;

	/**
	* @brief Representation of the collection of triangles that forms up
	* an entity's model.
	*/
	typedef std::vector<triangle> modelTriangles;

	/**
	* @brief Representation of the collection of normals that forms up
	* an entity's model.
	*/
	typedef std::vector<normal> modelNormals;

	
	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief The mode in which to apply the rotation of a transform to a model.
	*/
	enum class applyRotationMode {NOT_SPECIFIED = 0, EULER_ANGLES = 1, DIRECTION_VECTOR = 2};


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
		* If no model exists for such ID, throws exception.
		*/
		static const model& getModelAt(unsigned int modelID);

		/**
		* @brief Get the corresponding model triangles indices for a certain ID.
		* If no such indices exist for the specified model, throws exception.
		*/
		static const modelTriangles& getModelTrianglesAt(unsigned int modelID);

		/**
		* @brief Get the corresponding model normals for a certain ID.
		* If no such normals exist for the specified model, throws exception.
		*/
		static const modelNormals& getModelNormalsAt(unsigned int modelID);

		/**
		* @brief Add a texture for a face of a terrain block. The added texture is specified
		* by giving the name of a texture corresponding to said block.
		* WARNING. Only call when just all the face's vertices have been added
		* to the model.
		*/
		static void addBlockFaceTexture(const block& block, model& m, const std::string& textureName);

		/**
		* @brief Apply the given transform to the provided model and add it to the given batch model.
		*/
		static void applyTransform(model& aModel, const transform& transform, applyRotationMode rotMode,
								   bool rotateX = false, bool rotateY = false, bool rotateZ = false, model* batchModel = nullptr);


		// Clean up.

		/**
		* @brief Free any resources allocated to the static members of this class
		* and deinitialise it.
		*/
		static void reset();

	private:

		/**
		* Attributes.
		*/

		static bool initialised_;
		static std::unordered_map<unsigned int, model*> models_; // RENOMBRAR ESTO COMO modelsVertices_ Y METER UN models_ QUE YA TENGA LOS MODELOS "BAKEADOS" Y METER MÉTODO REGISTER MODELO PARA TODOS CON UNA OPCION DE GUARDAR SUS VERTICES Y TRIANGULOS POR SEPARADO O SOLO SU MODELO BAKEADO O AMBAS COSAS.
		static std::unordered_map<unsigned int, modelTriangles*> triangles_; // NOTE. If a model has a nullptr here, it means that no triangle indexing is used.
		static std::unordered_map<unsigned int, modelNormals*> normals_;

	};

}

#endif