#pragma once

#include "json/json.hpp"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "../../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include <unordered_set>
#include "../../Entities/EntityManager.h"

struct NavigationVertex {
    NavigationVertex() : score(0.5f), position(glm::vec3(0.f)) {}

    float score;
	glm::vec3 position;
};

#define RIGBODIES X(RigidStaticComponent), X(RigidDynamicComponent), X(VehicleComponent)
#define X(ARG) std::unordered_set<ARG*>*
using RigidTuple = std::tuple<RIGBODIES>;
#undef X
//#undef RIGBODIES

class NavigationMesh {
public:
	NavigationMesh(nlohmann::json data);
    
    size_t GetVertexCount() const;
    float GetSpacing() const;

    GLuint vbo;
    GLuint vao;

    void UpdateMesh();

	template<class T>
    void UpdateMesh();

	template<class T>
	void RemoveRigidbody(T *rigidbody);

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

	template<class T>
	auto* NavigationMesh::CoveringBodies();

    void UpdateRenderBuffers() const;

    bool IsContainedBy(size_t index, physx::PxBounds3 bounds);
    std::vector<size_t> FindAllContainedBy(physx::PxBounds3 bounds);

    float spacing;
	size_t columnCount;
	size_t rowCount;

    std::unordered_set<size_t> coveredVertices;
    RigidTuple coveringBodies;
    NavigationVertex *vertices;
};

template<class T>
auto* NavigationMesh::CoveringBodies()
{
	return std::get<std::unordered_set<T*>*>(coveringBodies);
}

template<class T>
void NavigationMesh::RemoveRigidbody(T *rigidbody) {
	// Check if covered vertices became uncovered
	for (auto it = coveredVertices.begin(); it != coveredVertices.end(); ) {
		const size_t index = *it;
		NavigationVertex& vertex = vertices[index];

		// Check if any of the bodies covering this vertex are the given rigidbody, and remove them
		auto &vertexCoveringBodies = CoveringBodies<T>()[index];
		for (auto it2 = vertexCoveringBodies.begin(); it2 != vertexCoveringBodies.end(); ) {
			if ((*it2) == rigidbody) {
				it2 = vertexCoveringBodies.erase(it2);
			}
			else {
				++it2;
			}
		}

		// If this vertex is no longer covered, remove it from the set of covered bodies
		if (vertexCoveringBodies.empty()) {
			vertex.score = 0.5f;
			it = coveredVertices.erase(it);
		}
		else {
			++it;
		}
	}
}

template<class T>
void NavigationMesh::UpdateMesh() {
	/*std::vector<T>& rigidbodies = EntityManager::Components<T>();
	// Check if covered vertices became uncovered
	for (auto it = coveredVertices.begin(); it != coveredVertices.end(); ) {
		const size_t index = *it;
		NavigationVertex& vertex = vertices[index];

		// Check if any of the bodies covering this vertex are no longer covering it
		auto &vertexCoveringBodies = CoveringBodies<T>()[index];
		for (auto it2 = vertexCoveringBodies.begin(); it2 != vertexCoveringBodies.end(); ) {
			const physx::PxBounds3 bounds = (*it2)->pxRigid->getWorldBounds(1.f);
			if (!IsContainedBy(index, bounds)) {
				it2 = vertexCoveringBodies.erase(it2);
			}
			else {
				++it2;
			}
		}

		// If this vertex is no longer covered, remove it from the set of covered bodies
		if (vertexCoveringBodies.empty()) {
			vertex.score = 0.5f;
			it = coveredVertices.erase(it);
		}
		else {
			++it;
		}
	}

	// Check if any of the bodies that were updated this frame are covering new vertices
	for (T rigidBody : rigidbodies) {
		//RigidbodyComponent *rigidbody = static_cast<RigidbodyComponent*>(component);
		if (!rigidbody.enabled || !rigidbody.DoesBlockNavigationMesh()) continue;

		// Find all the vertices this body covers
		const physx::PxBounds3 bounds = rigidbody.pxRigid->getWorldBounds(1.f);
		std::vector<size_t> contained = FindAllContainedBy(bounds);

		// Add this body to any vertices that it covers and mark them as covered
		for (size_t index : contained) {
			NavigationVertex &vertex = vertices[index];
			auto &vertexCoveringBodies = CoveringBodies<T>()[index];
			if (vertexCoveringBodies.empty()) {
				vertex.score = 0.f;
				coveredVertices.insert(index);
			}
			vertexCoveringBodies.insert(&rigidbody);
		}
	}

	UpdateRenderBuffers();*/
}