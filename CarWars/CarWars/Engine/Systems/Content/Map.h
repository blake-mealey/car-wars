#pragma once

#include <string>

class NavigationMesh;
class HeightMap;

class Map {
public:
    explicit Map(std::string dirPath);

    NavigationMesh* navigationMesh;
    HeightMap* heightMap;
};
