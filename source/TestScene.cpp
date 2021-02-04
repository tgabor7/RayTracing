#include "TestScene.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "BufferLoader.h"
#include <ctime>
#include <random>
#include <chrono>
#include "BMPParser.h"
#include "Texture.h"
#include "ModelLoader.h"
#include "FrameBuffer.h"

Shader* shader = nullptr;
Shader* addShader = nullptr;
int vao;
ComputeShader* computeshader = nullptr;
Camera camera;
mat4x4 view;
mat4x4 projection;
Texture* texture = nullptr;

vector3 pos;
vector4 light;
Model model;

Texture* skybox = nullptr;
FrameBuffer color_buffer;
FrameBuffer copy_buffer;
FrameBuffer paste_buffer;
FrameBuffer fbo;
int sample = 0;
vector2 randpx;
int resx = 0;
int resy = 0;
float seed = 0;
int number_of_spheres = 50;
int denoise = 0;

struct Sphere
{
	float px;
	float py;
	float pz;

	float colorx;
	float colory;
	float colorz;

	float emissionx;
	float emissiony;
	float emissionz;

	float specularx;
	float speculary;
	float specularz;

	float smoothness;
	float radius;
};

TestScene::TestScene() : Scene()
{

}
TestScene::TestScene(int _width, int _height) : Scene(_width,_height)
{

}
void setupScene()
{
	std::vector<Sphere> spheres;
	for (int i = 0; i < number_of_spheres; i++) {
		Sphere s;
		s.smoothness = (rand() / (float)RAND_MAX)*2 + 1;
		s.px = (rand() / (float)RAND_MAX) * 60 - 30;
		s.py = (rand() / (float)RAND_MAX) * 30 + 2;
		s.pz = (rand() / (float)RAND_MAX) * 60 - 30;

		s.radius = 1 + (rand() / (float)RAND_MAX) * 2;

		s.colorx = (rand() / (float)RAND_MAX);
		s.colory = (rand() / (float)RAND_MAX);
		s.colorz = (rand() / (float)RAND_MAX);

		s.specularx = s.colorx;
		s.speculary = s.colory;
		s.specularz = s.colorz;

		s.emissionx = (rand() % 2) * s.colorx * 2;
		s.emissiony = (rand() % 2) * s.colory * 2;
		s.emissionz = (rand() % 2) * s.colorz * 2;

		spheres.push_back(s);
	}
	computeshader->setSSBO(2, (void*)&spheres[0], spheres.size() * 56);
}
void TestScene::init()
{
	srand(std::time(0));
	resx = width / 4;
	resy = height / 4;
	shader = new Shader("shaders/shader", { {0,"vertices"} });
	addShader = new Shader("shaders/addShader", { {0,"vertices"} });
	vao = BufferLoader::createVao({ 0 });
	float vertices[12] = { -1,1,1,1,-1,-1,
						1,1,-1,-1,1,-1 };
	BufferLoader::storeDataInAttributeList(0, 2, vertices, 12);
	Image im = BMPLoader::loadBMP("images/wood.bmp");
	texture = new Texture(&im.pixels[0], im.width, im.height);
	Image skyboxImage = BMPLoader::loadBMP("images/skybox.bmp");
	skybox = new Texture(&skyboxImage.pixels[0], skyboxImage.width, skyboxImage.height); 
	computeshader = new ComputeShader("shaders/compute", NULL, width, height);
	camera.anglex = 20;
	camera.angley = -50;
	camera.distance = 200;
	vector4 light(1.0, 1.0, 1.0, 1.0);
	model = ModelLoader::loadSTL("models/cone.stl");
	
	camera.pointToLookAt = vector3(0, 0, 0);
	projection = maths::createProjectionMatrix(70.0, .1, 1000.0, width, height);
	//printf("%d", model.vertices.size());

	std::vector<Box> boxes;
	Box b;
	b.min.x = -2;
	b.min.y = -2;
	b.min.z = -2;

	b.max.x = 2;
	b.max.y = 2;
	b.max.z = 2;
	boxes.push_back(b);


	//printf("Number of boxes: %d", model.boxes.size());
	computeshader->setUniform("size", (float)model.vertices.size());
	computeshader->setSSBO(1, (void*)&model.vertices[0], model.vertices.size() * sizeof(float));
	//computeshader->setSSBO(2, (void*)&boxes[0], boxes.size() * sizeof(float)*6);
	setupScene();

	color_buffer = FrameBuffer(width, height, FrameBuffer::DEPTH_RENDER_BUFFER);
	copy_buffer = FrameBuffer(width, height, FrameBuffer::DEPTH_RENDER_BUFFER);
	paste_buffer = FrameBuffer(width, height, FrameBuffer::DEPTH_RENDER_BUFFER);
	fbo = FrameBuffer(width, height);
	
}
void TestScene::updateCamera(Camera& camera)
{
	if (camera.angley >= 90.0) {
		camera.angley = 90;
	}
	if (camera.angley <= -90) {
		camera.angley = -90;
	}


	camera.position.x = (camera.distance * -(float)sin(camera.anglex * 3.14159265 / 180)
		* (float)cos(camera.angley * 3.14159265 / 180) + camera.pointToLookAt.x);
	camera.position.y = (camera.distance * -(float)sin(camera.angley * 3.14159265 / 180)
		+ camera.pointToLookAt.y);
	camera.position.z = (-camera.distance * (float)cos(camera.anglex * 3.14159265 / 180)
		* (float)cos(camera.angley * 3.14159265 / 180) + camera.pointToLookAt.z);
}
void TestScene::clearFBO(FrameBuffer& fbo, int &sample)
{
	sample = 0;
	fbo.clear();
	
}

void TestScene::update(double delta)
{
	if (input->isKeyDown(keys::KEY_W)) {
		clearFBO(paste_buffer, sample);
		pos.z += cos(maths::toRad(camera.anglex)) * 0.1;
		pos.x += sin(maths::toRad(camera.anglex)) * 0.1;
	}
	if (input->isKeyDown(keys::KEY_S)) {
		clearFBO(paste_buffer, sample);

		pos.z -= cos(maths::toRad(camera.anglex)) * 0.1;
		pos.x -= sin(maths::toRad(camera.anglex)) * 0.1;
	}
	if (input->isKeyDown(keys::KEY_A)) {
		clearFBO(paste_buffer, sample);

		pos.z -= sin(maths::toRad(camera.anglex)) * 0.1;
		pos.x += cos(maths::toRad(camera.anglex)) * 0.1;
	}
	if (input->isKeyDown(keys::KEY_D)) {
		clearFBO(paste_buffer, sample);

		pos.z += sin(maths::toRad(camera.anglex)) * 0.1;
		pos.x -= cos(maths::toRad(camera.anglex)) * 0.1;
	}
	if (input->isKeyDown(keys::KEY_SPACE)) {
		denoise = 1;
	}
	camera.pointToLookAt = pos;
	
	float cdx = input->getMouseXf(keys::MOUSE_BUTTON_LEFT) * 0.1;
	float cdy = input->getMouseYf(keys::MOUSE_BUTTON_LEFT) * 0.1;
	float ds = input->scroll;

	if (cdx != 0 || cdy != 0 || ds != 0) clearFBO(paste_buffer, sample);

	camera.anglex -= cdx;
	camera.angley -= cdy;
	camera.distance -= ds;

	updateCamera(camera);

	input->update();
}
void TestScene::render(double delta)
{

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	///////////////////////////
	fbo.bindFrameBuffer();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture->textureBuffer);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, skybox->textureBuffer);

	randpx = vector2(((float)rand() / RAND_MAX)-0.5, ((float)rand() / RAND_MAX)-0.5);
	seed = (float)rand() / RAND_MAX;
	view = maths::createViewMatrix(camera);
	computeshader->bind();
	computeshader->setUniform("skybox_texture", 2);
	computeshader->setUniform("view", view);
	computeshader->setUniform("campos", camera.position);
	computeshader->setUniform("projection", projection);
	computeshader->setUniform("width", width);
	computeshader->setUniform("height", height);
	computeshader->setUniform("pos", pos);
	computeshader->setUniform("seed", seed);
	computeshader->setUniform("floor_texture", 1);
	computeshader->setUniform("light", light);
	computeshader->setUniform("transformation", maths::createTransformationMatrix(vector3(0,10,0),vector3(-1.56,0,0),vector3(10)) );
	computeshader->setUniform("randpx", randpx);
	computeshader->setUniform("number_of_spheres", number_of_spheres);
	
	light = vector4(1, 1, 1, 1);

	mat4x4 invview = maths::invert(view);
	computeshader->setUniform("invview", invview);

	mat4x4 inv = maths::invert(projection);
	computeshader->setUniform("inverseProjection", inv);
	 
	computeshader->render(resx, resy);
	glUseProgram(0);
	
	
	shader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, computeshader->tex_output);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	shader->unBind();

	fbo.unBindFrameBuffer();
	///////////////////////////
	fbo.resolveFbo(GL_COLOR_ATTACHMENT0, color_buffer);
	
	copy_buffer.bindFrameBuffer();
	addShader->bind();
	addShader->setUniform("sample", sample);
	addShader->setUniform("copy", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_buffer.colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paste_buffer.colorTexture);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	addShader->unBind();
	copy_buffer.unBindFrameBuffer();
	
	paste_buffer.bindFrameBuffer();
	shader->bind();
	shader->setUniform("denoise", denoise);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, copy_buffer.colorTexture);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	shader->unBind();
	paste_buffer.unBindFrameBuffer();

	shader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, paste_buffer.colorTexture);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

	shader->unBind();

	sample++;
	printf("\rSample: %d", sample);
	denoise = 0;
}
TestScene::~TestScene()
{

}