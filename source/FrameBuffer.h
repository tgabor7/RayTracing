#pragma once
#include <GL\glew.h>

class FrameBuffer
{
public:
	GLuint frameBuffer;

	GLuint colorTexture;
	GLuint depthTexture;

	GLuint colorBuffer;
	GLuint colorBuffer2;
	GLuint colorBuffer3;
	GLuint colorBuffer4;
	GLuint colorBuffer5;
	GLuint depthBuffer;

	int width;
	int height;
	bool multisampleAndMultiTarget = false;
	const static int NONE = 0;
	const static int DEPTH_TEXTURE = 1;
	const static int DEPTH_RENDER_BUFFER = 2;


	FrameBuffer();
	FrameBuffer(int width, int height);
	FrameBuffer(int width, int height, const int tpye);
	void bindFrameBuffer();
	void unBindFrameBuffer();
	void bindToRead();
	void clear();
	void resolveFbo(GLuint readBuffer, FrameBuffer fbo);
	void initialiseFrameBuffer(int type);

	~FrameBuffer();
private:
	void createFrameBuffer();
	void determineDrawBuffers();
	GLuint createMultiSampleColourAttachment(int width, int height, GLuint attachment);
	void createTextureAttachment();
	void createDepthTextureAttachment();
	void createDepthBufferAttachment();
};

