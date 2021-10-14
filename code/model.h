#ifndef _VOXELENG_MODEL_
#define _VOXELENG_MODEL_
#include <string>
#include <vector>
#include "vertex.h"



//////////////////////////
// Forward declarations //
//////////////////////////
class chunk;

namespace VoxelEng {

	namespace Model {
	
		////////////////////
		//Type definitions//
		////////////////////

		typedef std::vector<vertex> model;
		typedef std::vector<unsigned short> triangle;


		//////////////////
		//Oficial models//
		//////////////////

		extern model blockVertices;
		extern std::vector<triangle> blockTriangles;


		/////////////
		//Functions//
		/////////////

		/*
		Use a custom model ingame, created using a oficial
		JSON format.
		NOTE. W.I.P. Placeholder function.
		*/
		const model& loadCustomModel(const std::string& filePath) = delete;


		/*
		Add a texture for a face.
		WARNING. Only call when just all the face's vertices have been added
		to the model m. That is, in they are the latest vertices added to
		said model.
		*/
		void addTexture(VoxelEng::block blockID, unsigned int textureID, model& m);

	}

}

#endif