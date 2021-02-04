#include "FrameBuffer.h"


FrameBuffer::FrameBuffer()
{
}
FrameBuffer::FrameBuffer(int width, int height) {
	this->multisampleAndMultiTarget = true;
	this->width = width;
	this->height = height;
	initialiseFrameBuffer(DEPTH_RENDER_BUFFER);
}
FrameBuffer::FrameBuffer(int width, int height, const int type) {
	this->multisampleAndMultiTarget = false;
	this->width = width;
	this->height = height;
	initialiseFrameBuffer(type);
}
void FrameBuffer::clear()
{
	bindFrameBuffer();
	unBindFrameBuffer();

}
void FrameBuffer::bindFrameBuffer() {
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void FrameBuffer::unBindFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void FrameBuffer::createFrameBuffer() {
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	determineDrawBuffers();
}
void FrameBuffer::resolveFbo(GLuint readBuffer, FrameBuffer fff) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fff.frameBuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->frameBuffer);
	glReadBuffer(readBuffer);
	glBlitFramebuffer(0, 0, width, height, 0, 0, fff.width, fff.height,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	unBindFrameBuffer();
}
void FrameBuffer::determineDrawBuffers() {
	GLuint drawBuffers[5];
	int size = 1;
	drawBuffers[0] = GL_COLOR_ATTACHMENT0;
	if (this->multisampleAndMultiTarget) {
		size = 5;
		drawBuffers[1] = (GL_COLOR_ATTACHMENT1);
		drawBuffers[2] = (GL_COLOR_ATTACHMENT2);
		drawBuffers[3] = (GL_COLOR_ATTACHMENT3);
		drawBuffers[4] = (GL_COLOR_ATTACHMENT4);
	}

	glDrawBuffers(size, drawBuffers);

}
void FrameBuffer::initialiseFrameBuffer(int type) {
	createFrameBuffer();
	if (multisampleAndMultiTarget) {
		colorBuffer = createMultiSampleColourAttachment(width, height, GL_COLOR_ATTACHMENT0);
		colorBuffer2 = createMultiSampleColourAttachment(width, height, GL_COLOR_ATTACHMENT1);
		colorBuffer3 = createMultiSampleColourAttachment(width, height, GL_COLOR_ATTACHMENT2);
		colorBuffer4 = createMultiSampleColourAttachment(width, height, GL_COLOR_ATTACHMENT3);
		colorBuffer5 = createMultiSampleColourAttachment(width, height, GL_COLOR_ATTACHMENT4);

	}
	else {
		createTextureAttachment();
	}

	if (type == DEPTH_RENDER_BUFFER) {
		createDepthBufferAttachment();
	}
	else if (type == DEPTH_TEXTURE) {
		createDepthTextureAttachment();
	}
	FrameBuffer::unBindFrameBuffer();
}
void FrameBuffer::createTextureAttachment() {

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		(void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture,
		0);
}
void FrameBuffer::createDepthTextureAttachment() {
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT,
		GL_FLOAT, (void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
}
void FrameBuffer::createDepthBufferAttachment() {
	glGenRenderbuffers(1, &depthBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	if (!multisampleAndMultiTarget) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

	}
	else {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_DEPTH_COMPONENT24, width, height);
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		depthBuffer);
}
GLuint FrameBuffer::createMultiSampleColourAttachment(int width, int height, GLuint attachment) {

	GLuint colorBuffer;
	glGenRenderbuffers(1, &colorBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_RGBA32F, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, colorBuffer);

	return colorBuffer;
}
FrameBuffer::~FrameBuffer()
{
}
