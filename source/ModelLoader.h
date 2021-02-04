#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>
#include <sstream>

struct vec3 {
	float x;
	float y;
	float z;
};
struct Box {
	vec3 min;
	vec3 max;
};
struct vec2 {
	float x;
	float y;
};
struct Model {
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> textures;
	std::vector<Box> boxes;
};
class ModelLoader {
public:
	template<typename Out>
	inline static void split(const std::string& s, char delim, Out result) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}
	inline static std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> elements;
		split(s, delim, std::back_inserter(elements));
		return elements;
	}
	inline static Model loadSTL(const char* path)
	{
		Model m;
		try {
			std::ifstream file(path, std::ios::binary);
			std::string header = readHeader(file);
			int size = getInt(file);

			for (int i = 0; i < size; i++) {
				for (int j = 0; j < 3; j++) {
					m.normals.push_back(getFloat(file));
				}
				for (int j = 0; j < 9; j++) {
					m.vertices.push_back(getFloat(file));
				}

				

				getShort(file);
			}
#ifdef MODELLOADER_VERBOSE
			std::cout << header;
			std::cout << "\nSize: " << size << "\n";
#endif
			file.close();
		}
		catch (std::ifstream::failure ex) {
			assert("File not found");
		}

		float minx = min(m.vertices, 0);
		float miny = min(m.vertices, 1);
		float minz = min(m.vertices, 2);

		float maxx = max(m.vertices, 0);
		float maxy = max(m.vertices, 1);
		float maxz = max(m.vertices, 2);

		for (float i = minx; i < maxx; i += 1) {
			for (float j = miny; j < maxy; j += 1) {
				for (float k = minz; k < maxz; k += 1) {
					Box b;
					b.min.x = i - 1;
					b.min.y = j - 1;
					b.min.z = k - 1;

					b.max.x = i + 1;
					b.max.y = j + 1;
					b.max.z = k + 1;


					m.boxes.push_back(b);
				}
			}
		}
		return m;
	}
private:
	inline static float min(std::vector<float> vertices, int d)
	{
		float m = vertices[d];
		for (int i = 0; i < vertices.size(); i+=3) {
			if (vertices[i] < m) {
				m = vertices[i];
			}
		}
		return m;
	}
	inline static float max(std::vector<float> vertices, int d)
	{
		float m = vertices[d];
		for (int i = 0; i < vertices.size(); i += 3) {
			if (vertices[i] > m) {
				m = vertices[i];
			}
		}
		return m;
	}
	inline static float getFloat(std::ifstream& stream)
	{
		float f;
		stream.read((char*)&f, 4);

		return f;
	}
	inline static int getInt(std::ifstream& stream)
	{
		int i;
		stream.read((char*)&i, 4);

		return i;
	}
	inline static short getShort(std::ifstream& stream)
	{
		short i;
		stream.read((char*)&i, 2);

		return i;
	}
	inline static std::string readHeader(std::ifstream& stream)
	{
		char* head = new char[80];
		stream.read(head, 80);

		return std::string(head);
	}
};