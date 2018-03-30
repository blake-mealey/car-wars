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
    HeightMap(std::string dirPath);

	float GetHeight(glm::vec3 coords) const;
    float GetWidth() const;
    float GetLength() const;
	float GetMaxHeight() const;
	unsigned int GetWallVertices() const;
	float GetWallHeight() const;
    float GetXSpacing() const;
    float GetZSpacing() const;
    Mesh* GetMesh();
private:
    void Initialize(std::string filePath);

    Mesh* mesh = nullptr;

    int rowCount;
    int colCount;
    float** heights;

    float maxHeight;
    float maxWidth;
    float maxLength;
	unsigned int wallVertices;
	float wallHeight;
	float wallInclineRate;
	float variation;
	int wallMoundMaxVertices;
	int wallMoundMinVertices;
	float wallMoundVariation;

    float xSpacing;
    float zSpacing;
};