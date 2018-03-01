#pragma once

#include "json/json.hpp"
#include "Mesh.h"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"

struct NavigationVertex {
    NavigationVertex() : score(1.f), position(glm::vec3(0.f)) {}

    float score;
	glm::vec3 position;
};

class NavigationMesh {
public:
	NavigationMesh(nlohmann::json data);
    
    size_t GetVertexCount() const;

    GLuint vbo;
    GLuint vao;

    void UpdateMesh();
    void UpdateMesh(std::vector<Component*> rigidbodies);

private:
    void UpdateMesh(RigidbodyComponent* rigidbody);
	void Initialize();
    void InitializeRenderBuffers();

    void UpdateRenderBuffers() const;

    size_t FindClosestVertex(glm::vec3 worldPosition);
    std::vector<size_t> FindAllContainedBy(physx::PxBounds3 bounds);

    int GetForward(size_t index);
    int GetBackward(size_t index);
    int GetLeft(size_t index);
    int GetRight(size_t index);

    float spacing;
	size_t columnCount;
	size_t rowCount;

    NavigationVertex *vertices;
};