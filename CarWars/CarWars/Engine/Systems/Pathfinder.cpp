#include "Pathfinder.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

std::vector<glm::vec3> Pathfinder::FindPath(NavigationMesh* navigationMesh, glm::vec3 startPosition,
    glm::vec3 goalPosition) {
	
	size_t startIndex;
	size_t goalIndex;

	glm::vec3 start = startPosition;
	glm::vec3 end = goalPosition;

	glm::vec3 pathDirection = glm::normalize(end - start);

	size_t i = 0;
	// need to wrap this better
	do {
		startIndex = navigationMesh->FindClosestVertex(start);
		start += pathDirection; 
		i++;
	} while (navigationMesh->GetScore(startIndex) == 0.f && i < 10);
	i = 0;
	do {
		goalIndex = navigationMesh->FindClosestVertex(end);
		end -= pathDirection;
		i++;
	} while (navigationMesh->GetScore(goalIndex) == 0.f && i < 10);

    // Unreachable goal or start
    if (navigationMesh->GetScore(startIndex) == 0.f || navigationMesh->GetScore(goalIndex) == 0.f) {
        return {};
    }
    
    std::vector<size_t> closedSet;
    std::vector<size_t> openSet = {startIndex};

    // TODO: Use array instead?
    std::unordered_map<size_t, size_t> cameFrom;

    // TODO: Use array instead?
    std::unordered_map<size_t, float> gScore;
    gScore[startIndex] = 0.f;

    // TODO: Use array instead?
    std::unordered_map<size_t, float> fScore;
    fScore[startIndex] = HeuristicCostEstimate(navigationMesh, startIndex, goalIndex);

    int iterationsLeft = 2500;
    while (!openSet.empty()) {
        size_t current = GetCurrent(openSet, fScore);

        if (current == goalIndex) {
            return ReconstructPath(navigationMesh, cameFrom, current);
        }

        openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
        closedSet.push_back(current);

        std::vector<size_t> neighbours = navigationMesh->GetNeighbours(current);
        for (size_t neighbour : neighbours) {
            if (std::find(closedSet.begin(), closedSet.end(), neighbour) != closedSet.end()) continue;

            if (std::find(openSet.begin(), openSet.end(), neighbour) == openSet.end()) {
                openSet.push_back(neighbour);
            }

            const float score = navigationMesh->GetScore(current);
            const float cost = score == 0.f ? INFINITY : (1.f - score) * HeuristicCostEstimate(navigationMesh, current, neighbour) * 1000.f;
            const float tentativeGScore = GetScore(gScore, current) + cost;

            if (tentativeGScore >= GetScore(gScore, neighbour))
                continue;

            cameFrom[neighbour] = current;
            gScore[neighbour] = tentativeGScore;
            fScore[neighbour] = tentativeGScore + HeuristicCostEstimate(navigationMesh, neighbour, goalIndex);
        }

        if (--iterationsLeft == 0) break;
    }

    return {};
}

float Pathfinder::HeuristicCostEstimate(NavigationMesh *navigationMesh, size_t index0, size_t index1) {
    return HeuristicCostEstimate(navigationMesh->GetPosition(index0), navigationMesh->GetPosition(index1));
}

float Pathfinder::HeuristicCostEstimate(glm::vec3 pos0, glm::vec3 pos1) {
    return glm::length(pos0 - pos1);
}

size_t Pathfinder::GetCurrent(std::vector<size_t>& openSet, std::unordered_map<size_t, float>& fScore) {
    size_t lowest = openSet[0];
    float lowestScore = GetScore(fScore, lowest);
    for (size_t i = 1; i < openSet.size(); ++i) {
        size_t index = openSet[i];
        const float score = GetScore(fScore, index);
        if (score < lowestScore) {
            lowest = index;
            lowestScore = score;
        }
    }
    return lowest;
}

float Pathfinder::GetScore(std::unordered_map<size_t, float>& scoreMap, size_t index) {
    const auto it = scoreMap.find(index);
    if (it == scoreMap.end()) {
        return INFINITY;
    }
    return it->second;
}

std::vector<glm::vec3> Pathfinder::ReconstructPath(NavigationMesh *navigationMesh, std::unordered_map<size_t, size_t>& cameFrom, size_t goal) {
    std::vector<glm::vec3> totalPath = { navigationMesh->GetPosition(goal) };

    while (true) {
        const auto it = cameFrom.find(goal);
        if (it == cameFrom.end()) break;
        goal = it->second;
        totalPath.push_back(navigationMesh->GetPosition(goal));
    }

    SimplifyPath(totalPath);
    SmoothPath(totalPath, 1);

    return totalPath;
}

void Pathfinder::SimplifyPath(std::vector<glm::vec3>& path) {
    if (path.size() <= 2) return;

    // Remove redundant nodes
    for (auto it = path.begin() + 1; it != path.end() - 1; ) {
        const glm::vec3 previous = *(it - 1);
        const glm::vec3 current = *it;
        const glm::vec3 next = *(it + 1);

        if (current - previous == next - current) {
            it = path.erase(it);
        } else {
            ++it;
        }
    }
}

glm::vec3 Pathfinder::CatmullRom(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    return 0.5f * ((2.f * p1) + (-p0 + p2)*t + (2.f * p0 - 5.f*p1 + 4.f*p2 - p3)*t*t + (-p0 + 3.f*p1 - 3.f*p2 + p3)*t*t*t);
}

void Pathfinder::SmoothPath(std::vector<glm::vec3>& path, size_t iterations) {
    size_t size = path.size();
    if (size <= 3) return;

    // Add smoothing nodes
    for (size_t i = 2; i < size - 1; ++i) {
        const glm::vec3 p0 = path[i - 2];
        const glm::vec3 p1 = path[i - 1];
        const glm::vec3 p2 = path[i];
        const glm::vec3 p3 = path[i + 1];
        for (float j = 0.f; j < iterations; ++j) {
            path.insert(path.begin() + i, CatmullRom((j + 1.f) / (iterations + 2.f), p0, p1, p2, p3));
            ++i;
            ++size;
        }
    }
}
