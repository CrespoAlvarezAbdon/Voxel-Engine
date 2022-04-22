#ifndef _TEXTURE_
#define _TEXTURE_

#include <string>
#include <unordered_map>
#include <utility>
#include <GL/glew.h>
using namespace std;


///////////////
//Definitions//
///////////////


///////////
//Classes//
///////////

/*
Represents a texture (a 2D image).
It can (and should be) used to create texture atlases.
*/
class texture
{

public:

	// Constructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	texture(const string& filepath);


	// Observers.
	int width() const;
	int height() const;
	GLuint rendererID() const;
	static const texture* blockTextureAtlas();
	static int blockAtlasResolution();
	static const std::unordered_map<unsigned int, std::pair<int, int>>& GUItexturesHW();


	// Modifiers.

	/*
	Set 'blockTextureAtlas' as the texture atlas for the blocks.
	*/
	static void setBlockAtlas(const texture& blockTextureAtlas);

	/*
	Set the block texture atlas' resolution.
	The resolution will always make block textures be square.
	That is, if you execute texture::blockAtlasResolution() = 32, now
	the block textures will have a resolution of 32x32 pixels.
	WARNING. You must set the resolution at least once before the method
	chunkManager::manageChunks starts executing in the chunk management thread.
	NOTE. W.I.P. In the future, a texture pack will include information
	about the resolution of the textures.
	*/
	static void setBlockAtlasResolution(int resolution);


	/*
	Bind a texture to an OpenGL texture slot (slot 0 by default).
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void bind(unsigned int slot = 0) const;

	/*
	Unbind a texture.
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	void unbind() const;


	// Destructors.

	/*
	WARNING. Must be called in a thread with valid OpenGL context.
	*/
	~texture();

private:

	static texture const* blockTextureAtlas_;
	static unsigned int blockAtlasResolution_;
	static std::unordered_map<unsigned int, std::pair<int, int>> GUItexturesHW_;


	GLuint rendererID_;
	string textureFilepath_; 
	// Local buffer to store the texture data when loading it from disk.
	unsigned char* buffer_;
	int width_,
		height_,
		bitsPerPixel_;

};

inline int texture::width() const
{

	return width_;

}

inline int texture::height() const
{

	return height_;

}

inline GLuint texture::rendererID() const
{

	return rendererID_;

}

inline const texture* texture::blockTextureAtlas()
{

	return blockTextureAtlas_;

}

inline int texture::blockAtlasResolution() 
{

	return blockAtlasResolution_;

}

inline void texture::setBlockAtlas(const texture& blockTextureAtlas)
{

	blockTextureAtlas_ = &blockTextureAtlas;

}

inline void texture::setBlockAtlasResolution(int resolution)
{

	blockAtlasResolution_ = resolution;

}

inline const std::unordered_map<unsigned int, std::pair<int, int>>& texture::GUItexturesHW() {

	return GUItexturesHW_;

}

#endif