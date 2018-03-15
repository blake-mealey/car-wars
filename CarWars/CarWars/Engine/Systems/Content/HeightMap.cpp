#include "HeightMap.h"
#include "Image.h"
#include "Mesh.h"

using glm::vec3;

HeightMap::HeightMap(Image& image, const int& height, const int& width, const int& length) {
	if (image.Height() < 2 || image.Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	float* pixels = image.Pixels();
	float x = 0.0f, z = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0;
		for (unsigned int j = 0; j < image.Width(); j++) {
			vertices.push_back(x);
			vertices.push_back((1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*height);
			vertices.push_back(z);
			cout << (int)pixels << std::endl;
			pixels += image.Channels();//sizeof(float) * image.Channels();
			x += width;
		}
		z += length;
	}
	unsigned int r = 0;
	unsigned int w = image.Width();
	for (unsigned int i = 0; i < image.Height() - 1; i++) {
		for (unsigned int j = 0; j < image.Width() - 1; j++) {
			elements.push_back(r + j);
			elements.push_back(r + j + 1);
			elements.push_back(r + j + w);
			elements.push_back(r + j + 1);
			elements.push_back(r + j + w);
			elements.push_back(r + j + w + 1);
		}
		r += w;
	}
}

HeightMap::HeightMap(char* file, const int& height, const int& width, const int& length) : HeightMap(Image(file), height, width, length) { }

//This is the one we will be using
HeightMap::HeightMap(char* file, const int& maxHeight, const int& maxWidth, const int& maxLength, const float& uvstep) {
	Image image = Image(file);
	const int length = maxLength / image.Height();
	const int width = maxWidth / image.Width();

	float uvstepx;
	float uvstepy;

	float ratio;

	if (width > length) {
		ratio = (float)length / (float)width;
		uvstepy = uvstep * ratio;
		uvstepx = uvstep * (1 - ratio);
	}
	else {
		ratio = (float)width / (float)length;
		uvstepx = uvstep * ratio;
		uvstepy = uvstep * (1 - ratio);
	}

	if (image.Height() < 2 || image.Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	float* pixels = image.Pixels();
	float x = 0.0f, z = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0;
		for (unsigned int j = 0; j < image.Width(); j++) {
			vertices.push_back(x);
			vertices.push_back((1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*maxHeight);
			vertices.push_back(z);
			//cout << (int)pixels << std::endl;
			pixels += image.Channels();//sizeof(float) * image.Channels();
			x += width;
		}
		z += length;
	}
	unsigned int r = 0;
	unsigned int w = image.Width();
	for (unsigned int i = 0; i < image.Height() - 1; i++) {
		for (unsigned int j = 0; j < image.Width() - 1; j++) {
			elements.push_back(r + j);
			elements.push_back(r + j + 1);
			elements.push_back(r + j + w);
			elements.push_back(r + j + 1);
			elements.push_back(r + j + w);
			elements.push_back(r + j + w + 1);
		}
		r += w;
	}

	float y = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0.0f;
		for (unsigned int j = 0; j < image.Width(); j++) {
			uvs.push_back(x);
			uvs.push_back(y);
			x += uvstepx;
		}
		y += uvstepy;
	}
	
}

const vector<float>& HeightMap::Vertices() {
	return vertices;
}

const vector<unsigned int>& HeightMap::Elements() {
	return elements;
}

const vector<float>& HeightMap::UVS() {
	return uvs;
}

void HeightMap::PrintVertices() {
	for (int i = 0; i < vertices.size(); i += 3) {
		cout << vertices[i] << ", " << vertices[i + 1] << ", " << vertices[i + 2] << std::endl;
	}
	cout << std::endl << std::endl;
}

void HeightMap::PrintElements() {
	for (int i = 0; i < elements.size(); i += 3) {
		cout << elements[i] << ", " << elements[i + 1] << ", " << elements[i + 2] << std::endl;
	}
	cout << std::endl << std::endl;
}

vector<Triangle>& HeightMap::Triangles() {
	vector<Triangle> triangles;
	for (unsigned int i = 0; i < elements.size(); i += 3) {
		triangles.push_back(Triangle(elements[i], elements[i+1], elements[i+2]));
	}
	return triangles;
}

vector<glm::vec3>& HeightMap::Vec3Vertices() {
	vector<glm::vec3> vecs;
	for (unsigned int i = 0; i < vertices.size(); i += 3) {
		vecs.push_back(glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]));
	}
	return vecs;
}

vector<glm::vec2>& HeightMap::Vec2UVS() {
	vector<glm::vec2> UVs;
	for (unsigned int i = 0; i < uvs.size(); i += 2) {
		UVs.push_back(glm::vec2(uvs[i], uvs[i + 1]));
	}
	return UVs;
}