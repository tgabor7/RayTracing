#include "ComputeShader.h"
#include <fstream>
#include <iostream>

ComputeShader::ComputeShader() {}
ComputeShader::ComputeShader(std::string path, const void* pixels, const int& width, const int& height)
{
	this->width = width;
	this->height = height;

	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT,
		pixels);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
#ifdef CSHADER_VERBOSE
	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);
	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);
	printf("max local work group invocations %i\n", work_grp_inv);
#endif
	program = glCreateProgram();
	GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);
	std::string shaderCode;
	std::ifstream shaderStream(path + ".cs", std::ios::in);
	if (shaderStream.is_open()) {
		std::string Line = "";
		while (getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", (path + ".cs").c_str());
		getchar();

	}
	const char* shaderPointer = shaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderPointer, NULL);
	glCompileShader(shaderID);
	int rvalue;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(shaderID, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
	}
	glAttachShader(program, shaderID);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(program, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
	}
	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "img_output"), 0);
}
void ComputeShader::setTexture(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
}
ComputeShader::ComputeShader(std::string path)
{
	int tex_w = 512, tex_h = 512;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);

	program = glCreateProgram();
	GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);
	std::string shaderCode;
	std::ifstream shaderStream(path + ".cs", std::ios::in);
	if (shaderStream.is_open()) {
		std::string Line = "";
		while (getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", (path + ".cs").c_str());
		getchar();

	}
	const char* shaderPointer = shaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderPointer, NULL);
	glCompileShader(shaderID);
	int rvalue;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(shaderID, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
	}
	glAttachShader(program, shaderID);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in linking compute shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(program, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
	}
	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "img_output"), 0);
}
void ComputeShader::bind() {
	glUseProgram(program);
}
void ComputeShader::setUniform(const char* name, vector4 value) {
	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniform4f(location, value.x, value.y, value.z, value.w);
}
void ComputeShader::setUniform(const char* name, vector3 value) {
	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniform3f(location, value.x, value.y, value.z);
}
void ComputeShader::setUniform(const char* name, vector2 value) {
	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniform2f(location, value.x, value.y);
}
void ComputeShader::setUniform(const char* name, int value) {
	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniform1i(location, value);
}
void ComputeShader::setUniform(const char* name, float value) {
	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniform1f(location, value);
}
void ComputeShader::setUniform(std::string name, vector3* values) {
	float* t = new float[4 * 3];
	int p = 0;
	for (int i = 0; i < 4; i++) {
		t[p] = values[i].x;
		p++;
		t[p] = values[i].y;
		p++;
		t[p] = values[i].z;
		p++;
	}
	glUniform3fv(glGetUniformLocation(program, name.c_str()), 4,
		t);

	delete[] t;

}
void ComputeShader::setUniform(std::string name, mat4x4* values) {
	float* t = new float[16 * 4];
	int p = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 16; j++) {
			t[p] = values[i][j];
			p++;
		}
	}
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 4,
		false, t);
	delete[] t;
}
void ComputeShader::setUniform(const char* name, mat4x4 value)
{

	int location = glGetUniformLocation(program, name);
	if (location != -1)
		glUniformMatrix4fv(location, 1, false, value.getPointer());

}
void ComputeShader::setSSBO(int index, void* data, int size)
{
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}
void ComputeShader::render(int workx, int worky)
{
	glUseProgram(program);
	glDispatchCompute((GLuint)workx, (GLuint)worky, 1);
}
ComputeShader::~ComputeShader()
{


}