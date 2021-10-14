#include <cmath>
#include <stdexcept>
#include "model.h"
#include "texture.h"
#include "vertex.h"
#include "chunk.h"

namespace VoxelEng {

	namespace Model {


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

        std::vector<triangle> blockTriangles =
        {

            {0, 3, 1, 1, 3, 2}, // Back Face
            {5, 6, 4, 4, 6, 7}, // Front Face
            {3, 7, 2, 2, 7, 6}, // Top Face
            {1, 5, 0, 0, 5, 4}, // Bottom Face
            {4, 7, 0, 0, 7, 3}, // Left Face
            {1, 2, 5, 5, 2, 6}  // Right Face

        };


        /////////////
        //Functions//
        ///////////// 

		void addTexture(VoxelEng::block blockID, unsigned int textureID, model& m) {
		
			// TODO. MAKE ALL POSSIBLE OPERATIONS BE COMPUTED OR UPDATED ONLY WHEN NECESSARY.
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

}