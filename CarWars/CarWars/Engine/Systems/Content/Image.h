#pragma once
#include <iostream>

using std::cout;

class Image {
	float* pixels;
	int width;
	int height;
	int channels;
public:
	~Image();
	Image(const char* file);
	Image(float* _pixels, int& _width, int& _height, int& _channels);
	float* Pixels() { return pixels; }
	int Width() { return width;	}
	int Height() { return height; }
	int Channels() { return channels; }
	int Count() { return width*height*channels; }
	void PrintPixels();
};