/**
* @file vertex.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex.
* @brief Contains the definition of the 3D and 2D vertices used by the engine.
*/
#ifndef _VOXELENG_VERTEX_
#define _VOXELENG_VERTEX_

#include <definitions.h>
#include <vec.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

	///////////////////
	//Classes/Structs//
	///////////////////

	/**
	* @brief Represents a 3D vertex with texture coordinates.
	*/
	struct vertex {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		vertex();

		/**
		* @brief Class constructor.
		* @param posX Vertex coordinate in the X-axis.
		* @param posY Vertex coordinate in the Y-axis.
		* @param posZ Vertex coordinate in the Z-axis.
		*/
		vertex(vertexCoord posX, vertexCoord posY, vertexCoord posZ);

		/**
		* @brief Class constructor.
		* @param posX Vertex coordinate in the X-axis.
		* @param posY Vertex coordinate in the Y-axis.
		* @param posZ Vertex coordinate in the Z-axis.
		* @param texPosX Vertex texture coordinate in the X-axis.
		* @param texPosY Vertex texture coordinate in the Y-axis.
		* @param colorR Red channel color value.
		* @param colorG Green channel color value.
		* @param colorB Blue channel color value.
		* @param colorA Alpha channel color value.
		*/
		vertex(vertexCoord posX, vertexCoord posY, vertexCoord posZ, textureCoord texPosX, textureCoord texPosY, 
			byte colorR, byte colorG, byte colorB, byte colorA);


		/*
		Attributes.
		*/

		/**
		* @brief Vertex position. Index 0 -> X axis, 1 -> Y axis and 2 -> Z axis.
		*/
		vertexCoord positions[3];

		/**
		* @brief Vertex texture coordinates in corresponding texture atlas.
		*/
		textureCoord textureCoords[2];

		/**
		* @brief Vertex color stored as RGBA in 32-bits.
		*/
		byte color[4];

		/**
		* @brief Color light value applied to the vertex.
		*/
		basicVec4 additionalData;

		/**
		* @brief First two bytes are barycentric coordinates of the vertex. Third byte is material index.
		*/
		basicVec4 lightExtraData; 

		// TODO. THIS MUST BE GENERALIZED WHEN NON-BLOCK SHAPES ARE IMPLEMENTED.
		/**
		* @brief The colors of the four vertices of the same face block added like this: -ColorVertexA + ColorVertexB - ColorVertexC + ColorVertexD 
		*/
		basicVec4 colorExtraData;

	};

	inline vertex::vertex()
	: positions{ 0, 0, 0 }, textureCoords{ 0, 0 }, color{ 255, 255, 255, 255 }, 
	additionalData(basicVec4Zero), lightExtraData(basicVec4Zero), colorExtraData(basicVec4Zero) {}

	inline vertex::vertex(vertexCoord posX, vertexCoord posY, vertexCoord posZ)
	: positions{ posX, posY, posZ }, textureCoords{ 0, 0 }, color{ 255, 255, 255, 255 },
	additionalData(basicVec4Zero), lightExtraData(basicVec4Zero), colorExtraData(basicVec4Zero) {}

	inline vertex::vertex(vertexCoord posX, vertexCoord posY, vertexCoord posZ, textureCoord texPosX, textureCoord texPosY,
		byte colorR, byte colorG, byte colorB, byte colorA)
	: positions{ posX, posY, posZ }, textureCoords{ texPosX, texPosY }, color{ colorR, colorG, colorB, colorA },
	additionalData(basicVec4Zero), lightExtraData(basicVec4Zero), colorExtraData(basicVec4Zero) {}

	/**
	* @brief Represents a 2D vertex (usually used for drawing GUI).
	*/
	struct vertex2D {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		vertex2D();

		/**
		* @brief Class constructor.
		* @param posX Vertex coordinate in X-axis.
		* @param posY Vertex coordinate in Y-axis.
		* @param texPosX Vertex texture coordinate in X-axis.
		* @param texPosY Vertex texture coordinate in Y-axis.
		*/
		vertex2D(float posX, float posY);

		/**
		* @brief Class constructor.
		* @param pos Vertex coordinates.
		* @param texPos Vertex texture coordinates.
		*/
		vertex2D(vertexCoord posX, vertexCoord posY, textureCoord texPosX, textureCoord texPosY);


		/*
		Attributes.
		*/

		/**
		* @brief Vertex position.
		*/
		vertexCoord positions[2];

		/**
		* @brief Vertex texture coordinates in corresponding texture atlas.
		*/
		textureCoord textureCoords[2];

	};

	inline vertex2D::vertex2D() 
	: positions{ 0, 0 }, textureCoords{ 0, 0 } {}

	inline vertex2D::vertex2D(vertexCoord posX, vertexCoord posY)
	: positions{ posX, posY }, textureCoords{ 0, 0 } {}

	inline vertex2D::vertex2D(vertexCoord posX, vertexCoord posY, textureCoord texPosX, textureCoord texPosY)
	: positions{ posX, posY }, textureCoords{ texPosX, texPosY } {}

}

#endif