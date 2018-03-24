#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json/json.hpp>

using std::vector;

class Picture;
class Mesh;
struct Triangle;

class HeightMap {
public:
	HeightMap(nlohmann::json data);
	void PrintVertices();
	void PrintElements();
	float GetHeight(glm::vec3 coords) const;
    Mesh* GetMesh();
private:
    void CreateMesh();

    Mesh* mesh = nullptr;

    vector<glm::vec3> vertices;
    vector<glm::vec2> uvs;
    vector<Triangle> triangles;
};