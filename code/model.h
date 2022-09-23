#ifndef _VOXELENG_MODEL_
#define _VOXELENG_MODEL_
#include <string>
#include <vector>
#include <unordered_map>
#include "vertex.h"
#include "definitions.h"


namespace VoxelEng {

	/////////////////////
	//Type definitions.//
	/////////////////////

	typedef std::vector<vertex> model;


	/*
	A triangle is the collection of vertex indices that form it.
	*/
	// TODO. CHANGE NAME TO 'FACE'
	typedef std::vector<unsigned short> triangle;
	typedef std::vector<triangle> modelTriangles;


	///////////
	//Classes//
	///////////

	class models {

	public:

		// Initialisers.

		static void init();


		// Modifiers.

		/*
		Use a custom model ingame, created using aa oficial
		JSON format.
		*/
		static void loadCustomModel(const std::string& filePath, unsigned int modelID);

		/*
		Get the corresponding model for a certain ID.
		If no model exists for such ID, it returns the basic empty model with ID 0.
		*/
		static const model& getModelAt(unsigned int modelID);

		/*
		Get the corresponding model triangles indices for a certain ID.
		If no model exists for such ID, it returns the basic empty model triangles.
		*/
		static const modelTriangles& getModelTrianglesAt(unsigned int modelID);

		/*
		Get the corresponding model triangles indices for a certain ID.
		If no model exists for such ID, it returns the basic empty model triangles.
		*/
		static const modelTriangles& getModelTriangles(unsigned int modelID);

		/*
		Add a texture for a face of a terrain block.
		WARNING. Only call when just all the face's vertices have been added
		to the model m.
		*/
		static void addTexture(block blockID, unsigned int textureID, model& m);


		// Clean up.

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