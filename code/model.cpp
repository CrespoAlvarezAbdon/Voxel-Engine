#include "model.h"
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "texture.h"
#include "chunk.h"
#include "logger.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

#endif


// TODO. QUE EL MAP DE MODELOS VAYA POR NOMBRE NO POR ID.

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

            // Block model.
            model* blockVertices = new model {

                {0, 0, 0},
                {1, 0, 0},
                {1, 1, 0},
                {0, 1, 0},
                {0, 0, 1},
                {1, 0, 1},
                {1, 1, 1},
                {0, 1, 1}

            };

            modelTriangles* blockTriangles = new modelTriangles {

                {0, 3, 1, 1, 3, 2}, // Back Face.
                {5, 6, 4, 4, 6, 7}, // Front Face.
                {3, 7, 2, 2, 7, 6}, // Top Face.
                {1, 5, 0, 0, 5, 4}, // Bottom Face.
                {4, 7, 0, 0, 7, 3}, // Left Face.
                {1, 2, 5, 5, 2, 6}  // Right Face.

            };

            // Plane model.
            model* planeVertices = new model { // This is read as a .OBJ model and therefore it has no vertex indexing.
            
                // Front face.
                /*{-0.5,0,-0.5},// 0
                {-0.5,0,0.5}, // 1
                {0.5,0,0.5}, // 2
                {0.5,0,0.5}, // 2
                {0.5,0,-0.5}, // 3
                {-0.5,0,-0.5},// 0

                // Back face.
                {-0.5,0,0.5}, // 0
                {-0.5,0,-0.5}, // 1
                {0.5,0,-0.5}, // 2
                {0.5,0,-0.5}, // 2
                {0.5,0,0.5}, // 3
                {-0.5,0,0.5}, // 0*/

                // Front face.
                {-0.5,0,-0.5, 0,0, 255,0,0,255},// 0
                {-0.5,0,0.5, 0,0, 255,0,0,255}, // 1
                {0.5,0,0.5, 0,0, 255,0,0,255}, // 2
                {0.5,0,0.5, 0,0, 0,255,0,0}, // 2
                {0.5,0,-0.5, 0,0, 0,255,255,255}, // 3
                {-0.5,0,-0.5, 0,0, 0,0,255,255},// 0
                
                // Back face.
                {-0.5,0,0.5, 1, 1}, // 0
                {-0.5,0,-0.5, 0, 1}, // 1
                {0.5,0,-0.5, 0, 0}, // 2
                {0.5,0,-0.5, 0, 0}, // 2
                {0.5,0,0.5, 1, 0}, // 3
                {-0.5,0,0.5, 1, 1}, // 0
            
            };

            /////////////////////////////
            //Oficial model dictionary.//
            /////////////////////////////

            models_ = {

                {0, emptyVertices},
                {1, blockVertices},
                {planeModelID, planeVertices}

            };

            triangles_ = {

                {0, emptyTriangles},
                {1, blockTriangles},
                {planeModelID, nullptr}

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
                        
                        // Normals will not be supported for now.

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

    void models::addBlockFaceTexture(const block& block, model& m, const std::string& textureName) {

        unsigned int textureID = 0;
        textureID = block.textureID(textureName);

        const std::pair<int, int>& widthHeight = texture::getTextureWH(textureID);

        float atlasWidth = texture::blockTextureAtlas()->width(),
              atlasHeight = texture::blockTextureAtlas()->height(),
              widthRatio = atlasWidth / widthHeight.first,
              heightRatio = atlasHeight / widthHeight.second,
              texCoordX = (textureID % (int)widthRatio) / (widthRatio),
              texCoordY = std::ceil(textureID / heightRatio) / heightRatio,
              texCoordX2 = texCoordX - 1 / widthRatio,
              texCoordY2 = texCoordY - 1 / heightRatio;

        if (textureID) {

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

    void models::applyTransform(model& aModel, const transform& transform, applyRotationMode rotMode,
        bool rotateX, bool rotateY, bool rotateZ) {
    
        std::size_t size = aModel.size();
        if (size) {

            float sinAngleX = transform.sinRotX();
            float cosAngleX = transform.cosRotX();
            float sinAngleY = transform.sinRotY();
            float cosAngleY = transform.cosRotY();
            float sinAngleZ = transform.sinRotZ();
            float cosAngleZ = transform.cosRotZ();
           
            float oldFirstCoord = 0.0f;
            float oldSecondCoord = 0.0f;
            vec3 vertexAux = vec3Zero;

            vec3 axis = vec3Zero;
            float angle = 0.0f;
            if (rotMode == applyRotationMode::DIRECTION_VECTOR)
                transform.getRotationFromYaxis(axis, angle);

            // Translate the model's copy to the entity's position and apply rotations if necessary.
            for (unsigned int j = 0; j < size; j++) {

                // Scale.
                vertexAux.x = aModel[j].positions[0] * transform.scale.x;
                vertexAux.y = aModel[j].positions[1] * transform.scale.y;
                vertexAux.z = aModel[j].positions[2] * transform.scale.z;

                // Rotate.
                if (rotMode == applyRotationMode::EULER_ANGLES) {
                
                    if (rotateX) {

                        oldFirstCoord = aModel[j].positions[1];
                        oldSecondCoord = aModel[j].positions[2];

                        aModel[j].positions[1] = oldFirstCoord * cosAngleX -
                            oldSecondCoord * sinAngleX;
                        aModel[j].positions[2] = oldFirstCoord * sinAngleX +
                            oldSecondCoord * cosAngleX;

                    }

                    if (rotateY) {

                        oldFirstCoord = aModel[j].positions[0];
                        oldSecondCoord = aModel[j].positions[2];

                        aModel[j].positions[0] = oldFirstCoord * cosAngleY +
                            oldSecondCoord * sinAngleY;
                        aModel[j].positions[2] = oldSecondCoord * cosAngleY -
                            oldFirstCoord * sinAngleY;

                    }

                    if (rotateZ) {

                        oldFirstCoord = aModel[j].positions[0];
                        oldSecondCoord = aModel[j].positions[1];

                        aModel[j].positions[0] = oldFirstCoord * cosAngleZ -
                            oldSecondCoord * sinAngleZ;
                        aModel[j].positions[1] = oldFirstCoord * sinAngleZ +
                            oldSecondCoord * cosAngleZ;

                    }
                
                }
                else { // applyRotationMode::DIRECTION_VECTOR
                    
                    vertexAux = glm::rotate(vertexAux, glm::radians(angle), axis);

                }
                
                // Translate.
                aModel[j].positions[0] = vertexAux.x + transform.position.x;
                aModel[j].positions[1] = vertexAux.y + transform.position.y;
                aModel[j].positions[2] = vertexAux.z + transform.position.z;

            }

        }
    
    }

    void models::applyTransform(model& aModel, const transform& transform, model& batchModel, applyRotationMode rotMode,
        bool rotateX, bool rotateY, bool rotateZ) {

        std::size_t size = aModel.size();
        if (size) {

            float sinAngleX = transform.sinRotX();
            float cosAngleX = transform.cosRotX();
            float sinAngleY = transform.sinRotY();
            float cosAngleY = transform.cosRotY();
            float sinAngleZ = transform.sinRotZ();
            float cosAngleZ = transform.cosRotZ();

            float oldFirstCoord = 0.0f;
            float oldSecondCoord = 0.0f;
            vec3 vertexAux = vec3Zero;

            vec3 axis = vec3Zero;
            float angle = 0.0f;
            if (rotMode == applyRotationMode::DIRECTION_VECTOR)
                transform.getRotationFromYaxis(axis, angle);

            // Translate the model's copy to the entity's position and apply rotations if necessary.
            for (unsigned int j = 0; j < size; j++) {

                // Scale.
                vertexAux.x = aModel[j].positions[0] * transform.scale.x;
                vertexAux.y = aModel[j].positions[1] * transform.scale.y;
                vertexAux.z = aModel[j].positions[2] * transform.scale.z;

                // Rotate.
                if (rotMode == applyRotationMode::EULER_ANGLES) {

                    if (rotateX) {

                        oldFirstCoord = vertexAux.y;
                        oldSecondCoord = vertexAux.z;

                        vertexAux.y = oldFirstCoord * cosAngleX -
                            oldSecondCoord * sinAngleX;
                        vertexAux.z = oldFirstCoord * sinAngleX +
                            oldSecondCoord * cosAngleX;

                    }

                    if (rotateY) {

                        oldFirstCoord = vertexAux.x;
                        oldSecondCoord = vertexAux.z;

                        vertexAux.x = oldFirstCoord * cosAngleY +
                            oldSecondCoord * sinAngleY;
                        vertexAux.z = oldSecondCoord * cosAngleY -
                            oldFirstCoord * sinAngleY;

                    }

                    if (rotateZ) {

                        oldFirstCoord = vertexAux.x;
                        oldSecondCoord = vertexAux.y;

                        vertexAux.x = oldFirstCoord * cosAngleZ -
                            oldSecondCoord * sinAngleZ;
                        vertexAux.y = oldFirstCoord * sinAngleZ +
                            oldSecondCoord * cosAngleZ;

                    }

                }
                else { // applyRotationMode::DIRECTION_VECTOR

                    vertexAux = glm::rotate(vertexAux, glm::radians(angle), axis);

                }

                // Translate.
                aModel[j].positions[0] = vertexAux.x + transform.position.x;
                aModel[j].positions[1] = vertexAux.y + transform.position.y;
                aModel[j].positions[2] = vertexAux.z + transform.position.z;

                batchModel.push_back(aModel[j]);

            }

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