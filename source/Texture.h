#pragma once
#include <GL/glew.h>

class Texture
{
public:
	Texture() {}
	Texture(const unsigned char*, const int&, const int&);
	Texture(const char*);
	void update(const char* data);
	GLuint getTextureBuffer() const;
	GLuint textureBuffer;

private:
	int width;
	int height;
};