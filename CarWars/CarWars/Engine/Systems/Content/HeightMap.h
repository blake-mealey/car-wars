#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json/json.hpp>

using std::vector;

class Image;
class Mesh;
struct Triangle;

class HeightMap {
	vector<glm::vec3> vertices;
	vector<glm::vec2> uvs;
	vector<Triangle> elements;
public:
	HeightMap(char* file, const int& maxHeight, const int& maxWidth, const int& maxLength, const float& uvstep);
	HeightMap(Image& image, const int& height, const int& width, const int& length);
	HeightMap(char* file, const int& height, const int& width, const int& length);
	void PrintVertices();
	void PrintElements();
	vector<glm::vec3> Vertices();
	vector<Triangle> Triangles();
	vector<glm::vec2> UVS();
	static Mesh* CreateMesh(nlohmann::json data);
	//vector<glm::vec3> Vec3Vertices();
	//vector<glm::vec2> Vec2UVS();
	//vector<Triangle> Triangles();
};