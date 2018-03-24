#include "Picture.h"
#include "stb/stb_image.h"

Picture::~Picture() {
	stbi_image_free(pixels);
}

Picture::Picture(std::string filePath) {
	stbi_set_flip_vertically_on_load(false);
	pixels = stbi_loadf(filePath.c_str(), &width, &height, &channels, 0);
	if (pixels == nullptr) {
		cout << "Failed to Load Height Map\n";
	}
}

Picture::Picture(float* _pixels, int& _width, int& _height, int& _channels) : pixels(_pixels), width(_width), height(_height), channels(_channels) {}


void Picture::PrintPixels() {
	float* a = pixels;
	int count = 0;
	int count2 = 0;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			cout << a[0] << ", " << a[1] << ", " << a[2] << "\t";
			a += channels;
		}
        cout << std::endl;
	}

	cout << std::endl << std::endl;
}