#ifndef _TEXTURE_
#define _TEXTURE_

#include <string>
#include <GL/glew.h>
using namespace std;

class Texture
{

public:

	Texture(const string& filepath);

	void bind(unsigned int slot = 0) const; // "slot" is the slot where you want to bind the texture
	void unbind() const;

	int width() const noexcept;
	int height() const noexcept;

	GLuint renderer_ID() const noexcept;

	~Texture();

private:

	GLuint renderer_ID_;
	string texture_filepath_; // For debuggin purposes
	unsigned char* local_buffer_; // Buffer to hold texture's data
	int width_, height_, bits_per_pixel;

};

inline int Texture::width() const noexcept
{

	return width_;

}

inline int Texture::height() const noexcept
{

	return height_;

}

inline GLuint Texture::renderer_ID() const noexcept
{

	return renderer_ID_;

}

#endif