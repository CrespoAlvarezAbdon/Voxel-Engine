#ifndef _VOXELENG_MODEL_
#define _VOXELENG_MODEL_
#include <string>
#include <vector>
#include <unordered_map>
#include "vertex.h"
#include "definitions.h"


//////////////////////////
// Forward declarations //
//////////////////////////
class chunk;

namespace VoxelEng {

	////////////////////
	//Type definitions//
	////////////////////

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

		static std::unordered_map<block, model*> models_;
		static std::unordered_map<block, modelTriangles*> triangles_;

		/*
		Use a custom model ingame, created using a oficial
		JSON format.
		NOTE. W.I.P. Placeholder function.
		*/
		static void loadCustomModel(const std::string& filePath, block ID);

		/*
		Get the corresponding model for a certain block ID.
		If no model exists for such ID, it returns the basic block model.
		*/
		static const model& getModel(block ID);

		/*
		Get the corresponding model triangles indices for a certain block ID.
		If no model exists for such ID, it returns the basic block model.
		*/
		static const modelTriangles& getModelTriangles(block ID);

		/*
		Add a texture for a face of a terrain block.
		WARNING. Only call when just all the face's vertices have been added
		to the model m.
		*/
		static void addTexture(VoxelEng::block blockID, unsigned int textureID, model& m);

	private:

		

	};

	inline const model& models::getModel(block ID)
	{

		return *models_[ID];

	}

	inline const modelTriangles& models::getModelTriangles(block ID)
	{

		return *triangles_[ID];

	}

}

#endif