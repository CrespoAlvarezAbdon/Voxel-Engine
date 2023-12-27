#include "model.h"
#include <stdexcept>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <cmath>
#include "texture.h"
#include "chunk.h"
#include "logger.h"


namespace VoxelEng {

    //////////////
    //Constants.//
    //////////////

    const unsigned int VERTEX_PER_FACE = 3;


    ////////////
    //Classes.//
    ////////////

    // 'models' class. 

    bool models::initialised_ = false;
    std::unordered_map<unsigned int, model*> models::models_;
    std::unordered_map<unsigned int, modelTriangles*> models::triangles_;

    void models::init() {
    
        if (initialised_)
            logger::errorLog("Models system is already initialised");
        else {
        
           ///////////////////
           //Oficial models.//
           ///////////////////

            model* emptyVertices = new model{};
            modelTriangles* emptyTriangles = new modelTriangles{};

            model* blockVertices = new model{

                vertex{0, 0, 0},
                vertex{1, 0, 0},
                vertex{1, 1, 0},
                vertex{0, 1, 0},
                vertex{0, 0, 1},
                vertex{1, 0, 1},
                vertex{1, 1, 1},
                vertex{0, 1, 1}

            };

            modelTriangles* blockTriangles = new modelTriangles{

                {0, 3, 1, 1, 3, 2}, // Back Face.
                {5, 6, 4, 4, 6, 7}, // Front Face.
                {3, 7, 2, 2, 7, 6}, // Top Face.
                {1, 5, 0, 0, 5, 4}, // Bottom Face.
                {4, 7, 0, 0, 7, 3}, // Left Face.
                {1, 2, 5, 5, 2, 6}  // Right Face.

            };


            /////////////////////////////
            //Oficial model dictionary.//
            /////////////////////////////

            models_ = {

                {0, emptyVertices},
                {1, blockVertices}

            };

            triangles_ = {

                {0, emptyTriangles},
                {1, blockTriangles}

            };

            initialised_ = true;
        
        }

    }

    void models::loadCustomModel(const std::string& filePath, unsigned int modelID) {
        
        if (initialised_) {
        
            if (models_.contains(modelID))
                logger::errorLog("Cannot assign more than two models to the same model ID = " + modelID);
            else {

                if (std::filesystem::exists(filePath)) {

                    model* newModel = new model;
                    std::vector<unsigned short> verticesInds,
                                                uvsInds,
                                                normalInds;
                    std::vector<vec3> tempVerticesCoords,
                                      tempNormals;
                    std::vector<vec2> tempUVs;
                    std::string lineRead;
                    std::istringstream lineReadStream;
                    vec3 tempVertexCoords,
                         tempNormal;
                    vec2 tempUV;
                    unsigned short vertexInd,
                                   uvInd,
                                   normalInd;
                    char discardedChar;
                    vertex tempVertex;

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

                        // Normals will not be implemented for now.

                        newModel->push_back(tempVertex);

                    }

                    models_[modelID] = newModel;
                    triangles_[modelID] = nullptr; // Generic models do not make use of triangle indexing for now.

                }
                else
                    logger::errorLog("No model file found at " + filePath);

            }
        
        }
        else
            logger::errorLog("Models system is not initialised");
        
    }

    const model& models::getModelAt(unsigned int modelID) {

        if (models_.find(modelID) == models_.cend())
            return *models_[0];
        else
            return *models_[modelID];

    }

    const modelTriangles& models::getModelTrianglesAt(unsigned int modelID) {

        if (models_.find(modelID) == models_.cend())
            logger::errorLog("No model with ID " + std::to_string(modelID) + " was found");
        else
            return getModelTriangles(modelID);

    }

	void models::addTexture(const block& block, unsigned int textureID, model& m) {
		
		float atlasWidth = texture::blockTextureAtlas()->width(),
			  atlasHeight = texture::blockTextureAtlas()->height(),
			  textureWidth = texture::blockAtlasResolution(), // This will depend on texture ID in the future.
			  textureHeight = texture::blockAtlasResolution(), // This will depend on texture ID in the future.
			  texCoordX = (textureID % (int)(atlasWidth / textureWidth)) / (atlasWidth / textureWidth),
			  texCoordY = std::ceil(textureID / (atlasHeight / textureHeight)) / (atlasHeight / textureHeight),
		      texCoordX2 = texCoordX - 1 / (atlasWidth / textureWidth),
			  texCoordY2 = texCoordY - 1 / (atlasHeight / textureHeight);
		

        if (!block.isEmptyBlock() && textureID != 0) {
            
            unsigned int modelSize = m.size();

            if (modelSize >= 6) {

                m.operator[](modelSize - 6).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 6).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 5).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 5).textureCoords[1] = texCoordY;

                m.operator[](modelSize - 4).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 4).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 3).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 3).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 2).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 2).textureCoords[1] = texCoordY;

                m.operator[](modelSize - 1).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 1).textureCoords[1] = texCoordY;

            }
            else
                logger::errorLog("Not enough vertices to add face's texture.");
            
        }

	}

    void models::addTexture(const block& block, model& m) {

        unsigned int textureID = block.textureID();
        float atlasWidth = texture::blockTextureAtlas()->width(),
              atlasHeight = texture::blockTextureAtlas()->height(),
              textureWidth = texture::blockAtlasResolution(), // This will depend on block type in the future.
              textureHeight = texture::blockAtlasResolution(), // This will depend on block type in the future.
              widthRatio = atlasWidth / textureWidth,
              heightRatio = atlasHeight / textureHeight,
              texCoordX = (textureID % (int)widthRatio) / (widthRatio),
              texCoordY = std::ceil(textureID / heightRatio) / heightRatio,
              texCoordX2 = texCoordX - 1 / widthRatio,
              texCoordY2 = texCoordY - 1 / heightRatio;


        if (&block && textureID) {

            unsigned int modelSize = m.size();

            if (modelSize >= 6) {

                m.operator[](modelSize - 6).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 6).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 5).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 5).textureCoords[1] = texCoordY;

                m.operator[](modelSize - 4).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 4).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 3).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 3).textureCoords[1] = texCoordY2;

                m.operator[](modelSize - 2).textureCoords[0] = texCoordX2;
                m.operator[](modelSize - 2).textureCoords[1] = texCoordY;

                m.operator[](modelSize - 1).textureCoords[0] = texCoordX;
                m.operator[](modelSize - 1).textureCoords[1] = texCoordY;

            }
            else
                logger::errorLog("Not enough vertices to add face's texture.");

        }

    }

    void models::reset() {
    
        for (auto it = models_.cbegin(); it != models_.cend(); it++)
            delete it->second;
        models_.clear();

        for (auto it = triangles_.cbegin(); it != triangles_.cend(); it++)
            delete it->second;
        triangles_.clear();

        initialised_ = false;
    
    }

}