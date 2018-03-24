#pragma once
#include <iostream>

using std::cout;

class Picture {
	float* pixels;
	int width;
	int height;
	int channels;
public:
	~Picture();
	Picture(std::string filePath);
	Picture(float* _pixels, int& _width, int& _height, int& _channels);
	float* Pixels() { return pixels; }
	int Width() { return width;	}
	int Height() { return height; }
	int Channels() { return channels; }
	int Count() { return width*height*channels; }
	void PrintPixels();

    float Sample(float row, float col);
};