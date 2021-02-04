#pragma once

#include "Scene.h"
class FrameBuffer;
class TestScene : public Scene
{
public:
	TestScene();
	TestScene(int,int);
	void init();
	void update(double);
	void render(double);
	void updateCamera(Camera& camera);
	void clearFBO(FrameBuffer &fbo, int &sample);
	~TestScene();
private:
};

