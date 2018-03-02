#pragma once

#include "json/json.hpp"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"
#include <unordered_set>

struct NavigationVertex {
    NavigationVertex() : score(0.5f), position(glm::vec3(0.f)) {}

    float score;
	glm::vec3 position;
};

class NavigationMesh {
public:
	NavigationMesh(nlohmann::json data);
    
    size_t GetVertexCount() const;
    float GetSpacing() const;

    GLuint vbo;
    GLuint vao;

    void UpdateMesh();
    void UpdateMesh(std::vector<Component*> rigidbodies);

    size_t FindClosestVertex(glm::vec3 worldPosition) const;

    NavigationVertex GetVertex(size_t index) const;
    glm::vec3 GetPosition(size_t index) const;
    float GetScore(size_t index) const;

    NavigationVertex GetVertex(size_t row, size_t col) const;
    glm::vec3 GetPosition(size_t row, size_t col) const;
    float GetScore(size_t row, size_t col) const;

    std::vector<size_t> GetNeighbours(size_t index);

    int GetForward(size_t index) const;
    int GetBackward(size_t index) const;
    int GetLeft(size_t index) const;
    int GetRight(size_t index) const;

private:
	void Initialize();
    void InitializeRenderBuffers();

    void UpdateRenderBuffers() const;

    std::vector<size_t> FindAllContainedBy(physx::PxBounds3 bounds);

    float spacing;
	size_t columnCount;
	size_t rowCount;

    std::unordered_set<size_t> coveredVertices;
    std::unordered_set<RigidbodyComponent*> *coveringBodies;
    NavigationVertex *vertices;
};