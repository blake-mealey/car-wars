#pragma once

#include <string>

class Picture;
class NavigationMesh;
class HeightMap;

class Map {
public:
    explicit Map(std::string dirPath);

    NavigationMesh* navigationMesh;
    HeightMap* heightMap;

private:
    void LoadObjects(Picture* objectsMap);

    float mapWidth;
    float mapLength;
};
