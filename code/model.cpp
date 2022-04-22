#include <cmath>
#include <stdexcept>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include "model.h"
#include "texture.h"
#include "vertex.h"
#include "chunk.h"

#include <ostream>

// Implementation-specific definitions.
#define NUMBER_PER_FACE_ELEMENT 3;

namespace VoxelEng {

    //////////////////
    //Oficial models//
    //////////////////

    model blockVertices = {

        vertex{0, 0, 0},
        vertex{1, 0, 0},
        vertex{1, 1, 0},
        vertex{0, 1, 0},
        vertex{0, 0, 1},
        vertex{1, 0, 1},
        vertex{1, 1, 1},
        vertex{0, 1, 1}

    };

    modelTriangles blockTriangles = {

        {0, 3, 1, 1, 3, 2}, // Back Face.
        {5, 6, 4, 4, 6, 7}, // Front Face.
        {3, 7, 2, 2, 7, 6}, // Top Face.
        {1, 5, 0, 0, 5, 4}, // Bottom Face.
        {4, 7, 0, 0, 7, 3}, // Left Face.
        {1, 2, 5, 5, 2, 6}  // Right Face.

    };


    ////////////////////////////
    //Oficial model dictionary//
    ////////////////////////////

    std::unordered_map<block, model*> models::models_ = {

        {0, &blockVertices}

    };

    std::unordered_map<block, modelTriangles*> models::triangles_ = {

        {0, &blockTriangles}

    };


    /////////////
    //Functions//
    ///////////// 

    void models::loadCustomModel(const std::string& filePath, block ID) {
        
        model* newModel = new model;
        std::vector<unsigned short> verticesInds, 
                                    uvsInds,
                                    normalInds;
        std::vector<vec3<float>> tempVerticesCoords,
                                 tempNormals;
        std::vector<vec2<float>> tempUVs;
        std::string lineRead;
        std::istringstream lineReadStream;
        vec3<float> tempVertexCoords,
                    tempNormal;
        vec2<float> tempUV;
        unsigned short vertexInd,
                       uvInd,
                       normalInd;
        char discardedChar;
        vertex tempVertex;


        // TODO. EXCEPTION HANDLING
        // Get model's file and read the entire file.
        std::ifstream modelFile(filePath);
        while (getline(modelFile, lineRead)) {

            lineReadStream.str(lineRead);

            // If a vertex texture line was read.
            if (lineRead[0] == 'v' && lineRead[1] == 't') {
                
                lineReadStream.seekg(2);

                lineReadStream >> tempUV.x;
                lineReadStream >> tempUV.y;

                tempUVs.push_back(tempUV);
                
            }
            else // If a normal line was read.
                if (lineRead[0] == 'v' && lineRead[1] == 'n') {
                
                    lineReadStream.seekg(2);

                    lineReadStream >> tempNormal.x;
                    lineReadStream >> tempNormal.y;
                    lineReadStream >> tempNormal.z;

                    tempNormals.push_back(tempNormal);

                
                }
                else // If a vertex line was read.
                    if (lineRead[0] == 'v') {

                        lineReadStream.seekg(1);

                        lineReadStream >> tempVertexCoords.x;
                        lineReadStream >> tempVertexCoords.y;
                        lineReadStream >> tempVertexCoords.z;

                        tempVerticesCoords.push_back(tempVertexCoords);

                    }
                    else // If a face line was read.
                        if (lineRead[0] == 'f') {
                        
                            lineReadStream.seekg(1);

                            while (!lineReadStream.eof()) {
                            
                                lineReadStream >> vertexInd >> discardedChar >> uvInd >> discardedChar >> normalInd;

                                verticesInds.push_back(vertexInd);
                                uvsInds.push_back(uvInd);
                                normalInds.push_back(normalInd);

                            }
                        
                        }
        
        }

        // File reading completed.
        modelFile.close();

        
        for (unsigned int i = 0; i < verticesInds.size(); i++) {
            
            // Note: OBJ indexing starts at 1.
            tempVertex.positions[0] = tempVerticesCoords[verticesInds[i] - 1].x;
            tempVertex.positions[1] = tempVerticesCoords[verticesInds[i] - 1].y;
            tempVertex.positions[2] = tempVerticesCoords[verticesInds[i] - 1].z;

            tempVertex.textureCoords[0] = tempUVs[uvsInds[i] - 1].x;
            tempVertex.textureCoords[1] = tempUVs[uvsInds[i] - 1].y;
            
            newModel->push_back(tempVertex);

        }

        if (models_.contains(ID))
            throw std::exception("Cannot assign more than two models to ID = " + ID);
        else {
        
            models_[ID] = newModel;
            triangles_[ID] = nullptr; // Generic models do not make use of triangle indexing for now.
        
        }

    }

	void models::addTexture(VoxelEng::block blockID, unsigned int textureID, model& m) {
		
		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  textureWidth = texture::blockAtlasResolution(), // This will depend on blockID in the future.
			  textureHeight = texture::blockAtlasResolution(), // This will depend on blockID in the future.
			  texCoordX = (textureID % (int)(atlasWidth / textureWidth)) / (atlasWidth / textureWidth),
			  texCoordY = ceil(textureID / (atlasHeight / textureHeight)) / (atlasHeight / textureHeight),
		      texCoordX2 = texCoordX - 1 / (atlasWidth / textureWidth),
			  texCoordY2 = texCoordY - 1 / (atlasHeight / textureHeight);
		

        // Get a function to tell block model type from a lookup table.
        if (blockID != 0) {
            
            unsigned int modelSize = m.size();

            if (modelSize >= 6) {


                m[modelSize - 6].textureCoords[0] = texCoordX2;
                m[modelSize - 6].textureCoords[1] = texCoordY2;

                m[modelSize - 5].textureCoords[0] = texCoordX2;
                m[modelSize - 5].textureCoords[1] = texCoordY;

                m[modelSize - 4].textureCoords[0] = texCoordX;
                m[modelSize - 4].textureCoords[1] = texCoordY2;

                m[modelSize - 3].textureCoords[0] = texCoordX;
                m[modelSize - 3].textureCoords[1] = texCoordY2;

                m[modelSize - 2].textureCoords[0] = texCoordX2;
                m[modelSize - 2].textureCoords[1] = texCoordY;

                m[modelSize - 1].textureCoords[0] = texCoordX;
                m[modelSize - 1].textureCoords[1] = texCoordY;

            }
            else {
                
                throw runtime_error("[ERROR]: Not enough vertices to add face's texture!");
                
            }
            
        }

	}

}