#pragma once
#include <vector>
#include <glm/glm.hpp>

using std::vector;

class Image;
struct Triangle;

class HeightMap {
	vector<float> vertices;
	vector<float> uvs;
	vector<unsigned int> elements;
public:
	HeightMap(char* file, const int& maxHeight, const int& maxWidth, const int& maxLength, const float& uvstep);
	HeightMap(Image& image, const int& height, const int& width, const int& length);
	HeightMap(char* file, const int& height, const int& width, const int& length);
	void PrintVertices();
	void PrintElements();
	const vector<float>& Vertices();
	const vector<unsigned int>& Elements();
	const vector<float>& UVS();
	vector<glm::vec3>& Vec3Vertices();
	vector<glm::vec2>& Vec2UVS();
	vector<Triangle>& Triangles();
};