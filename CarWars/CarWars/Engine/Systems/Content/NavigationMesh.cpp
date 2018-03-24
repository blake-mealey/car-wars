#include "NavigationMesh.h"

#include "ContentManager.h"

#include <iostream>
#include "../../Entities/EntityManager.h"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"
#include "../Game.h"

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
    coveringBodies = new std::unordered_set<RigidbodyComponent*>[GetVertexCount()];

	for (size_t row = 0; row < rowCount; ++row) {
        for (size_t col = 0; col < columnCount; ++col) {
            const size_t index = row*columnCount + col;

            glm::vec3 position = glm::vec3(
                (spacing + rowCount) * -0.5f*spacing + row*spacing,
                1.f,
                (spacing + columnCount) * -0.5f*spacing + col*spacing
            );

            HeightMap* map = Game::Instance().GetHeightMap();
            if (map) position.y = map->GetHeight(position);

            vertices[index].position = position;
		}
	}

    InitializeRenderBuffers();
    UpdateMesh();
}

void NavigationMesh::UpdateMesh() {
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidStatic));
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidDynamic));
    UpdateMesh(EntityManager::GetComponents(ComponentType_Vehicle));
}

void NavigationMesh::UpdateMesh(std::vector<Component*> rigidbodies) {
    // Check if covered vertices became uncovered
    for (auto it = coveredVertices.begin(); it != coveredVertices.end(); ) {
        const size_t index = *it;
        NavigationVertex& vertex = vertices[index];
        
        // Check if any of the bodies covering this vertex are no longer covering it
        auto &vertexCoveringBodies = coveringBodies[index];
        for (auto it2 = vertexCoveringBodies.begin(); it2 != vertexCoveringBodies.end(); ) {
            const physx::PxBounds3 bounds = (*it2)->pxRigid->getWorldBounds(1.f);
            if (!IsContainedBy(index, bounds)) {
                it2 = vertexCoveringBodies.erase(it2);
            } else {
                ++it2;
            }
        }

        // If this vertex is no longer covered, remove it from the set of covered bodies
        if (vertexCoveringBodies.empty()) {
            vertex.score = 0.5f;
            it = coveredVertices.erase(it);
        } else {
            ++it;
        }
    }

    // Check if any of the bodies that were updated this frame are covering new vertices
    for (Component* component : rigidbodies) {
        RigidbodyComponent *rigidbody = static_cast<RigidbodyComponent*>(component);
        if (!rigidbody->enabled || !rigidbody->DoesBlockNavigationMesh()) continue;

        // Find all the vertices this body covers
        const physx::PxBounds3 bounds = rigidbody->pxRigid->getWorldBounds(1.f);
        std::vector<size_t> contained = FindAllContainedBy(bounds);

        // Add this body to any vertices that it covers and mark them as covered
        for (size_t index : contained) {
            NavigationVertex &vertex = vertices[index];
            auto &vertexCoveringBodies = coveringBodies[index];
            if (vertexCoveringBodies.empty()) {
                vertex.score = 0.f;
                coveredVertices.insert(index);
            }
            vertexCoveringBodies.insert(rigidbody);
        }
    }

    UpdateRenderBuffers();
}

void NavigationMesh::RemoveRigidbody(RigidbodyComponent *rigidbody) {
	// Check if covered vertices became uncovered
	for (auto it = coveredVertices.begin(); it != coveredVertices.end(); ) {
		const size_t index = *it;
		NavigationVertex& vertex = vertices[index];

		// Check if any of the bodies covering this vertex are the given rigidbody, and remove them
		auto &vertexCoveringBodies = coveringBodies[index];
		for (auto it2 = vertexCoveringBodies.begin(); it2 != vertexCoveringBodies.end(); ) {
			if ((*it2) == rigidbody) {
				it2 = vertexCoveringBodies.erase(it2);
			} else {
				++it2;
			}
		}

		// If this vertex is no longer covered, remove it from the set of covered bodies
		if (vertexCoveringBodies.empty()) {
			vertex.score = 0.5f;
			it = coveredVertices.erase(it);
		} else {
			++it;
		}
	}
}

size_t NavigationMesh::FindClosestVertex(glm::vec3 worldPosition) const {
    // Bounds-check rows
    size_t row = 0;
    if (worldPosition.x < GetPosition(0, 0).x) {
        row = 0;
    } else if (worldPosition.x > GetPosition(rowCount - 1, 0).x) {
        row = rowCount - 1;
    } else {
        // Binary search over rows
        size_t left = 0;
        size_t right = rowCount - 1;
        glm::vec3 position;
        while (left < right) {
            row = (left + right) / 2;
            position = GetPosition(row, 0);
            if (position.x < worldPosition.x) {
                left = row + 1;
            } else if (position.x > worldPosition.x) {
                right = row - 1;
            } else {
                break;
            }
        }

        // Find shortest distance from best guess row
        float shortestDist = abs(position.x - worldPosition.x);
        for (size_t r = row - 2; r < row + 2; ++r) {
            const float dist = abs(GetPosition(r, 0).x - worldPosition.x);
            if (dist < shortestDist) {
                shortestDist = dist;
                row = r;
            }
        }
    }

    // Bounds-check columns
    size_t column = 0;
    if (worldPosition.z < GetPosition(row, 0).z) {
        column = 0;
    } else if (worldPosition.z > GetPosition(row, columnCount - 1).z) {
        column = columnCount - 1;
    } else {
        // Binary search over columns
        size_t left = 0;
        size_t right = columnCount - 1;
        glm::vec3 position;
        while (left < right) {
            column = (left + right) / 2;
            position = GetPosition(row, column);
            if (position.z < worldPosition.z) {
                left = column + 1;
            } else if (position.z > worldPosition.z) {
                right = column - 1;
            } else {
                break;
            }
        }

        // Find shortest distance from best guess column
        float shortestDist = abs(position.z - worldPosition.z);
        for (size_t c = column - 2; c < column + 2; ++c) {
            const float dist = abs(GetPosition(row, c).z - worldPosition.z);
            if (dist < shortestDist) {
                shortestDist = dist;
                column = c;
            }
        }
    }

    return row * columnCount + column;
}

glm::vec3 NavigationMesh::GetPosition(size_t index) const {
    return GetVertex(index).position;
}

float NavigationMesh::GetScore(size_t index) const {
    return GetVertex(index).score;
}

NavigationVertex NavigationMesh::GetVertex(size_t row, size_t col) const {
    return GetVertex(row * columnCount + col);
}

glm::vec3 NavigationMesh::GetPosition(size_t row, size_t col) const {
    return GetVertex(row, col).position;
}

float NavigationMesh::GetScore(size_t row, size_t col) const {
    return GetVertex(row, col).score;
}

std::vector<size_t> NavigationMesh::GetNeighbours(size_t index) {
    std::vector<size_t> neighbours;
    
    const int left = GetLeft(index);
    if (left != -1) neighbours.push_back(left);

    const int forwardLeft = GetForward(left);
    if (forwardLeft != -1) neighbours.push_back(forwardLeft);

    const int backwardLeft = GetBackward(left);
    if (backwardLeft != -1) neighbours.push_back(backwardLeft);

    const int right = GetRight(index);
    if (right != -1) neighbours.push_back(right);

    const int forwardRight = GetForward(right);
    if (forwardRight != -1) neighbours.push_back(forwardRight);

    const int backwardRight = GetBackward(right);
    if (backwardRight != -1) neighbours.push_back(backwardRight);

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
    if (!bounds.contains(Transform::ToPx(vertices[closest].position))) return contained;
    contained.push_back(closest);

    int left = closest;
    while ((left = GetLeft(left)) != -1) {
        contained.push_back(left);

        int forward = left;
        while ((forward = GetForward(forward)) != -1) {
            contained.push_back(forward);
            if (!bounds.contains(Transform::ToPx(vertices[forward].position))) break;
        }

        int backward = left;
        while ((backward = GetBackward(backward)) != -1) {
            contained.push_back(backward);
            if (!bounds.contains(Transform::ToPx(vertices[backward].position))) break;
        }

        if (!bounds.contains(Transform::ToPx(vertices[left].position))) break;
    }

    int right = closest;
    while ((right = GetRight(right)) != -1) {
        contained.push_back(right);

        int forward = right;
        while ((forward = GetForward(forward)) != -1) {
            contained.push_back(forward);
            if (!bounds.contains(Transform::ToPx(vertices[forward].position))) break;
        }

        int backward = right;
        while ((backward = GetBackward(backward)) != -1) {
            contained.push_back(backward);
            if (!bounds.contains(Transform::ToPx(vertices[backward].position))) break;
        }

        if (!bounds.contains(Transform::ToPx(vertices[right].position))) break;
    }

    int forward = closest;
    while ((forward = GetForward(forward)) != -1) {
        contained.push_back(forward);
        if (!bounds.contains(Transform::ToPx(vertices[forward].position))) break;
    }

    int backward = closest;
    while ((backward = GetBackward(backward)) != -1) {
        contained.push_back(backward);
        if (!bounds.contains(Transform::ToPx(vertices[backward].position))) break;
    }

    return contained;
}

int NavigationMesh::GetForward(size_t index) const {
    const int forward = index + columnCount;
    return forward < GetVertexCount() ? forward : -1;
}

int NavigationMesh::GetBackward(size_t index) const {
    const int backward = index - columnCount;
    return backward >= 0 ? backward : -1;
}

int NavigationMesh::GetLeft(size_t index) const {
    const int left = index - 1;
    return left % columnCount != columnCount - 1 ? left : -1;
}

int NavigationMesh::GetRight(size_t index) const {
    const int right = index + 1;
    return right % columnCount != 0 ? right : -1;
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

bool NavigationMesh::IsContainedBy(size_t index, physx::PxBounds3 bounds) {
    if (bounds.contains(Transform::ToPx(GetPosition(index)))) return true;
    
    std::vector<size_t> neighbours = GetNeighbours(index);
    for (size_t neighbour : neighbours) {
        if (bounds.contains(Transform::ToPx(GetPosition(neighbour)))) return true;
    }

    return false;
}
