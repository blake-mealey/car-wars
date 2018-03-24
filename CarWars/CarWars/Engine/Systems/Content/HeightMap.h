#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json/json.hpp>

using std::vector;

class Picture;
class Mesh;
struct Triangle;

class HeightMap {
	vector<glm::vec3> vertices;
	vector<glm::vec2> uvs;
	vector<Triangle> triangles;
public:
//	HeightMap(const char* file, const int maxHeight, const int maxWidth, const int maxLength, const float uvstep);
//	HeightMap(Picture& image, const int height, const int width, const int length);
//	HeightMap(const char* file, const int height, const int width, const int length);
	HeightMap(nlohmann::json data);
	void PrintVertices();
	void PrintElements();
	float GetHeight(glm::vec3 coords);
	vector<glm::vec3> Vertices();
	vector<Triangle> Triangles();
	vector<glm::vec2> UVS();
    Mesh* GetMesh();
//    static Mesh* CreateMesh(nlohmann::json data);
	//vector<glm::vec3> Vec3Vertices();
	//vector<glm::vec2> Vec2UVS();
	//vector<Triangle> Triangles();
private:
    void CreateMesh();
    Mesh* mesh = nullptr;
};