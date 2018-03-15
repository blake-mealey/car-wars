#include "Image.h"
//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
//#endif
Image::~Image() {
	stbi_image_free(pixels);
}

Image::Image(const char* file) {
	stbi_set_flip_vertically_on_load(false);
	pixels = stbi_loadf(file, &width, &height, &channels, 0);
	if (pixels == nullptr)
		cout << "Failed to Load Height Map\n";
}

Image::Image(float* _pixels, int& _width, int& _height, int& _channels) : pixels(_pixels), width(_width), height(_height), channels(_channels) {}


void Image::PrintPixels() {
	float* a = pixels;
	int count = 0;
	int count2 = 0;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			cout << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
			a += channels;
		}
	}
	/*while (count < width*height*channels) {
		cout << *a;
		a++;
		count++;
		count2++;
		if (count2 % channels == 0) {
			if (count2 == channels*width) {
				count2 = 0;
				cout << std::endl;
			}
			else
				cout << "     ";
		}
		else
			cout << ", ";

	}*/
	cout << std::endl << std::endl;
	//cout << (int)pixels << ", " << (int)&pixels[0] << ", " << (int)&pixels[1] << std::endl << std::endl;
}