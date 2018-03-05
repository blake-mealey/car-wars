#pragma once
#include "Content/NavigationMesh.h"
#include <unordered_map>
#include <deque>

class Pathfinder {
public:
    static std::vector<glm::vec3> FindPath(NavigationMesh *navigationMesh, glm::vec3 startPosition, glm::vec3 goalPosition);

private:
    static float HeuristicCostEstimate(NavigationMesh *navigationMesh, size_t index0, size_t index1);
    static float HeuristicCostEstimate(glm::vec3 pos0, glm::vec3 pos1);

    static size_t GetCurrent(std::vector<size_t> &openSet, std::unordered_map<size_t, float> &fScore);

    static float GetScore(std::unordered_map<size_t, float> &scoreMap, size_t index);

    static void SimplifyPath(std::vector<glm::vec3> &path);
    static void SmoothPath(std::vector<glm::vec3> &path, size_t iterations);

    static std::vector<glm::vec3> ReconstructPath(NavigationMesh *navigationMesh, std::unordered_map<size_t, size_t> &cameFrom, size_t goal);

    static glm::vec3 CatmullRom(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
};
