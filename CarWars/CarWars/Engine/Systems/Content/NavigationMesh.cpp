#include "NavigationMesh.h"

#include "ContentManager.h"

#include <iostream>
#include "../../Entities/EntityManager.h"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"

NavigationMesh::NavigationMesh(nlohmann::json data) {
	columnCount = ContentManager::GetFromJson<size_t>(data["ColumnCount"], 100);
	rowCount = ContentManager::GetFromJson<size_t>(data["RowCount"], 100);
	spacing = ContentManager::GetFromJson<float>(data["Spacing"], 2.f);

	Initialize();
}

size_t NavigationMesh::GetVertexCount() const {
    return columnCount * rowCount;
}

float NavigationMesh::GetSpacing() const {
    return spacing;
}

void NavigationMesh::Initialize() {
    vertices = new NavigationVertex[GetVertexCount()];

	for (size_t row = 0; row < rowCount; ++row) {
        for (size_t col = 0; col < columnCount; ++col) {
            const size_t index = row*columnCount + col;
            // TODO: Cylinder positions
            vertices[index].position = glm::vec3(rowCount * -0.5f*spacing + row*spacing, 1.f, columnCount * -0.5f*spacing + col*spacing);
		}
	}

    InitializeRenderBuffers();
    UpdateMesh();
}

void NavigationMesh::UpdateMesh() {
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidDynamic));
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidStatic));
    UpdateMesh(EntityManager::GetComponents(ComponentType_Vehicle));
}

void NavigationMesh::UpdateMesh(std::vector<Component*> rigidbodies) {
    for (Component* component : rigidbodies) {
        RigidbodyComponent *rigidbody = static_cast<RigidbodyComponent*>(component);
        UpdateMesh(rigidbody);
    }

    UpdateRenderBuffers();
}

void NavigationMesh::UpdateMesh(RigidbodyComponent* rigidbody) {
    physx::PxBounds3 bounds = rigidbody->pxRigid->getWorldBounds(1.f);
    std::vector<size_t> contained = FindAllContainedBy(bounds);
    for (size_t index : contained) {
        vertices[index].score = 0.f;
    }
}

size_t NavigationMesh::FindClosestVertex(glm::vec3 worldPosition) {     // TODO: improve efficiency
    size_t closest = 0;
    float distance = std::numeric_limits<float>::max();
    for (size_t i = 1; i < GetVertexCount(); i++) {
        const glm::vec3 vertex = vertices[i].position;
        const float dist = glm::length(vertex - worldPosition);
        if (dist < distance) {
            closest = i;
            distance = dist;
        }
    }
    std::cout << closest << std::endl;
    return closest;
}

glm::vec3 NavigationMesh::GetPosition(size_t index) const {
    return GetVertex(index).position;
}

float NavigationMesh::GetScore(size_t index) const {
    return GetVertex(index).score;
}

std::vector<size_t> NavigationMesh::GetNeighbours(size_t index) {
    std::vector<size_t> neighbours;
    
    const int left = GetLeft(index);
    if (left != -1) neighbours.push_back(left);

    const int right = GetRight(index);
    if (right != -1) neighbours.push_back(right);

    const int forward = GetForward(index);
    if (forward != -1) neighbours.push_back(forward);

    const int backward = GetBackward(index);
    if (backward != -1) neighbours.push_back(backward);

    return neighbours;
}

NavigationVertex NavigationMesh::GetVertex(size_t index) const {
    return vertices[index];
}

// TODO: Make less ugly (split into sub-functions)
std::vector<size_t> NavigationMesh::FindAllContainedBy(physx::PxBounds3 bounds) {
    std::vector<size_t> contained;

    const size_t closest = FindClosestVertex(Transform::FromPx(bounds.getCenter()));
    const physx::PxVec3 position = Transform::ToPx(vertices[closest].position);
    if (!bounds.contains(position)) return contained;
    contained.push_back(closest);
    
    int left = closest;
    while ((left = GetLeft(left)) != -1) {
        const physx::PxVec3 position = Transform::ToPx(vertices[left].position);
        if (!bounds.contains(position)) break;
        contained.push_back(left);

        int forward = left;
        while ((forward = GetForward(forward)) != -1) {
            const physx::PxVec3 position = Transform::ToPx(vertices[forward].position);
            if (!bounds.contains(position)) break;
            contained.push_back(forward);
        }

        int backward = left;
        while ((backward = GetBackward(backward)) != -1) {
            const physx::PxVec3 position = Transform::ToPx(vertices[backward].position);
            if (!bounds.contains(position)) break;
            contained.push_back(backward);
        }
    }

    int right = closest;
    while ((right = GetRight(right)) != -1) {
        const physx::PxVec3 position = Transform::ToPx(vertices[right].position);
        if (!bounds.contains(position)) break;
        contained.push_back(right);

        int forward = right;
        while ((forward = GetForward(forward)) != -1) {
            const physx::PxVec3 position = Transform::ToPx(vertices[forward].position);
            if (!bounds.contains(position)) break;
            contained.push_back(forward);
        }

        int backward = right;
        while ((backward = GetBackward(backward)) != -1) {
            const physx::PxVec3 position = Transform::ToPx(vertices[backward].position);
            if (!bounds.contains(position)) break;
            contained.push_back(backward);
        }
    }

    int forward = closest;
    while ((forward = GetForward(forward)) != -1) {
        const physx::PxVec3 position = Transform::ToPx(vertices[forward].position);
        if (!bounds.contains(position)) break;
        contained.push_back(forward);
    }

    int backward = closest;
    while ((backward = GetBackward(backward)) != -1) {
        const physx::PxVec3 position = Transform::ToPx(vertices[backward].position);
        if (!bounds.contains(position)) break;
        contained.push_back(backward);
    }

    return contained;
}

int NavigationMesh::GetForward(size_t index) {
    const int forward = index + columnCount;
    return forward < GetVertexCount() ? forward : -1;
}

int NavigationMesh::GetBackward(size_t index) {
    const int backward = index - columnCount;
    return backward >= 0 ? backward : -1;
}

int NavigationMesh::GetLeft(size_t index) {        // TODO: Error check
    const int left = index - 1;
    return left;
}

int NavigationMesh::GetRight(size_t index) {        // TODO: Error check
    const int right = index + 1;
    return right;
}

void NavigationMesh::InitializeRenderBuffers() {
    glGenBuffers(1, &vbo);
    UpdateRenderBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(NavigationVertex), reinterpret_cast<void*>(0));        // score

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NavigationVertex), reinterpret_cast<void*>(4));        // position
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void NavigationMesh::UpdateRenderBuffers() const {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * rowCount * columnCount, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}