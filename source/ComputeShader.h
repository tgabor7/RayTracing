#pragma once
#include <GL/glew.h>
#include <string>
#include "Maths.h"

class ComputeShader
{
public:
	ComputeShader();
	ComputeShader(std::string path);
	ComputeShader(std::string path, const void* pixels, const int& width, const int& height);
	void render(int, int);
	void bind();
	GLuint tex_output;
	void setUniform(const char* name, int value);
	void setUniform(const char* name, float value);
	void setUniform(std::string name, vector3* value);
	void setUniform(std::string name, mat4x4* value);
	void setUniform(const char* name, mat4x4 value);
	void setUniform(const char* name, vector4 value);
	void setUniform(const char* name, vector3 value);
	void setUniform(const char* name, vector2 value);
	void setSSBO(int index, void* data, int size);
	void setTexture(GLuint texure);
	virtual ~ComputeShader();
private:
	GLuint program;
	int width;
	int height;
};

