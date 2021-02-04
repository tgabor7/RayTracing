#include "Scene.h"
#include "Window.h"
#include <chrono>
#include <ctime>
#include <iostream>

Scene::Scene(int w, int h)
{
	width = w;
	height = h;
}
int Scene::run()
{
	resolutionScale = 1.0;
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = new Window("Raytracing", width, height, false, true);

	running = true;

	glewInit();
	input = new Input(window, width, height);
	time = getTime();
	frame_cap = 1.0 / 60.0;
	unprocessed = 0;
	init();

	while (running && !window->shouldClose()) {
		bool can_render = false;
		double time_2 = getTime();
		passed = time_2 - time;
		unprocessed += passed;
		frame_time += passed;

		time = time_2;

		while (unprocessed >= frame_cap) {
			delta = (getTime() - lastFrameTime);
			//if (delta > 100) delta = 0;
			lastFrameTime = getTime();
			unprocessed -= frame_cap;
			can_render = true;

			update(delta);

			if (frame_time >= 1.0) {

				frame_time = 0;

				printf("\rFps: %d", frames);
				frames = 0;
			}
		}
		if (can_render) {
			glClearColor(1.0f, .3f, 1.0f, 1);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glEnable(GL_DEPTH_TEST);
			glEnable(GL_MULTISAMPLE);
			render(delta);
			window->swapBuffers();
			glfwPollEvents();

			frames++;
		}

	}

	delete window;
	glfwTerminate();

	return FINISHED;
}
double Scene::getTime()
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / (double)1000000000L;
}