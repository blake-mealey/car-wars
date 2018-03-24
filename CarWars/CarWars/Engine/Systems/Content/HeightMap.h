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
    float GetWidth() const;
    float GetLength() const;
    float GetXSpacing() const;
    float GetZSpacing() const;
    Mesh* GetMesh();
private:
    Mesh* mesh = nullptr;

    int rowCount;
    int colCount;
    float** heights;

    float maxHeight;
    float maxWidth;
    float maxLength;

    float xSpacing;
    float zSpacing;
};