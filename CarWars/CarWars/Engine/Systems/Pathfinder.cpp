#include "Pathfinder.h"

std::deque<glm::vec3> Pathfinder::FindPath(NavigationMesh* navigationMesh, glm::vec3 startPosition,
    glm::vec3 goalPosition) {

    const size_t startIndex = navigationMesh->FindClosestVertex(startPosition);
    const size_t goalIndex = navigationMesh->FindClosestVertex(goalPosition);

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
            const float cost = score == 0.f ? INFINITY : (1.f - score) * HeuristicCostEstimate(navigationMesh, current, neighbour);
            const float tentativeGScore = GetScore(gScore, current) + cost;

            if (tentativeGScore >= GetScore(gScore, neighbour))
                continue;

            cameFrom[neighbour] = current;
            gScore[neighbour] = tentativeGScore;
            fScore[neighbour] = tentativeGScore + HeuristicCostEstimate(navigationMesh, neighbour, goalIndex);
        }
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

std::deque<glm::vec3> Pathfinder::ReconstructPath(NavigationMesh *navigationMesh, std::unordered_map<size_t, size_t>& cameFrom, size_t goal) {
    std::deque<glm::vec3> totalPath = { navigationMesh->GetPosition(goal) };

    while (true) {
        const auto it = cameFrom.find(goal);
        if (it == cameFrom.end()) break;
        goal = it->second;
        totalPath.push_front(navigationMesh->GetPosition(goal));
    }

    return totalPath;
}
