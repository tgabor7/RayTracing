#pragma once
#include <vector>
#include <fstream>

struct Image {
public:
	std::vector<unsigned char> pixels;
	int width;
	int height;
};
class BMPLoader {
public:
	inline static Image loadBMP(const char* path)
	{
		Image im;
		std::ifstream file(path, std::ios::in | std::ios::binary);
		char* signature = new char[2];
		char* i = new char[4];
		char* s = new char[2];
		int start;
		int file_size;
		int inf_size;
		int width;
		int height;
		short bit_depth;
		int compression;
		int bit_map_size;
		int palett_colors;
		int used_colors;

		file.read(signature, 2);
		file.read((char*)&file_size, 4);

		//additional information
		file.read(i, 4);

		file.read((char*)&start, 4);

		file.read((char*)&inf_size, 4);
		file.read((char*)&width, 4);
		file.read((char*)&height, 4);

		//Skip next 2 bytes
		file.read(s, 2);

		file.read((char*)&bit_depth, 2);

		file.read((char*)&compression, 4);

		file.read((char*)&bit_map_size, 4);

		//skip next 8 bytes
		file.read(i, 4);
		file.read(i, 4);

		file.read((char*)&palett_colors, 4);
		file.read((char*)&used_colors, 4);

		int rowSize = ((bit_depth * width) / (float)32) * 4;
		int pixel_array_size = rowSize * height;
		std::vector<unsigned char> pixels;
		for (int j = 0; j < height; j++) {
			int p = 0;
			for (int i = 0; i < rowSize; i += 3) {
				if (bit_depth == 24) {
					unsigned char* pixel = new unsigned char[3];
					file.read((char*)pixel, 3);
					p += 3;
					pixels.push_back(pixel[2]);
					pixels.push_back(pixel[1]);
					pixels.push_back(pixel[0]);
				}
				if (bit_depth == 32) {
					unsigned char* pixel = new unsigned char[4];
					file.read((char*)pixel, 4);
					p += 4;
					pixels.push_back(pixel[2]);
					pixels.push_back(pixel[1]);
					pixels.push_back(pixel[0]);
					pixels.push_back(pixel[3]);

				}

			}
			while (p % 4 != 0) {
				p++;
				file.read(new char(), 1);
			}
		}
#ifdef BMPLOADER_VERBOSE
		std::cout << "\nSignature: " << signature;
		std::cout << "\nFileSize: " << file_size;

		std::cout << "\nWidth: " << width;
		std::cout << "\nHeight: " << height;

		std::cout << "\nStart: " << start;

		std::cout << "\nBit Depth: " << bit_depth;

		std::cout << "\nCompression: " << compression;
		std::cout << "\nBitMapSize: " << bit_map_size;

		std::cout << "\nPalett Colors: " << palett_colors;
		std::cout << "\nUsed Colors: " << used_colors;
#endif


		file.close();

		im.pixels = pixels;
		im.width = width;
		im.height = height;
		return im;
	}
};