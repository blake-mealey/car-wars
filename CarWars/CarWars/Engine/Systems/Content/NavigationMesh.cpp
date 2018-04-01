#include "NavigationMesh.h"

#include "ContentManager.h"

#include "../../Entities/EntityManager.h"
#include "../../Components/RigidbodyComponents/RigidbodyComponent.h"
#include "../Game.h"
#include "Picture.h"
#include <glm/gtx/string_cast.hpp>

NavigationMesh::NavigationMesh(nlohmann::json data) : heightMap(nullptr), defaults(nullptr) {
	columnCount = ContentManager::GetFromJson<size_t>(data["ColumnCount"], 100);
	rowCount = ContentManager::GetFromJson<size_t>(data["RowCount"], 100);
	spacing = ContentManager::GetFromJson<float>(data["Spacing"], 2.f);

	Initialize();
}

NavigationMesh::NavigationMesh(std::string dirPath) {
    // Load spacing and columns/rows from height map
    heightMap = ContentManager::GetHeightMap(dirPath);
    spacing = 5.f;
    columnCount = heightMap->GetLength() / spacing;
    rowCount = heightMap->GetWidth() / spacing;

    // Load defaults from image
    Picture* image = new Picture(ContentManager::MAP_DIR_PATH + dirPath + "Nav.png");
    
    defaults = new float[GetVertexCount()];

    size_t index = 0;
    for (unsigned int row = 0; row < rowCount; ++row) {
        for (unsigned int col = 0; col < columnCount; ++col) {
            const float y = 0.01f + image->Sample(row / static_cast<float>(rowCount), col / static_cast<float>(columnCount)) * 0.7f;
            defaults[index++] = y;
        }
    }

    delete image;



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
            const size_t index = row * columnCount + col;

			glm::vec3 position = glm::vec3(
				(row * spacing) - ((rowCount * spacing) * 0.5f) + (0.5f * spacing),
				1.f,
				(col * spacing) - ((columnCount * spacing) * 0.5f) + (0.5f * spacing)
			);

            if (heightMap) position.y += heightMap->GetHeight(position);

            vertices[index].position = position;
            vertices[index].score = GetDefault(index);
		}
	}

    InitializeRenderBuffers();
}

void NavigationMesh::UpdateMesh() {
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidStatic));
    UpdateMesh(EntityManager::GetComponents(ComponentType_RigidDynamic));
//    UpdateMesh(EntityManager::GetComponents(ComponentType_Vehicle));
}

void NavigationMesh::UpdateMesh(vector<Component*> rigidbodies) {
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
            vertex.score = GetDefault(index);
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
        vector<size_t> contained = FindAllContainedBy(bounds);

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

void NavigationMesh::ResetMesh() {
    coveredVertices.clear();
    for (size_t i = 0; i < GetVertexCount(); ++i) {
        coveringBodies[i].clear();
        vertices[i].score = defaults[i];
    }
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
            vertex.score = GetDefault(index);
			it = coveredVertices.erase(it);
		} else {
			++it;
		}
	}
}

size_t NavigationMesh::FindClosestVertex(glm::vec3 worldPosition) const {
    // Initial mapping
    int row = worldPosition.x/spacing + static_cast<float>(rowCount - 1)*0.5f;
    int column = worldPosition.z/spacing + static_cast<float>(columnCount - 1)*0.5f;

    // Bounds-check rows
    if (row < 0) row = 0;
    else if (row > rowCount - 1) row = rowCount - 1;

    // Bounds check columns
    if (column < 0) column = 0;
    else if (column > columnCount - 1) column = columnCount - 1;

    // Return index
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

NavigationVertex NavigationMesh::GetVertex(size_t index) const {
    return vertices[index];
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

// TODO: Make less ugly (split into sub-functions)
std::vector<size_t> NavigationMesh::FindAllContainedBy(physx::PxBounds3 bounds) {
    std::vector<size_t> contained;

    const physx::PxVec3 offset = physx::PxVec3(spacing) + bounds.getDimensions()*0.5f;
    bounds = physx::PxBounds3(bounds.getCenter() - offset, bounds.getCenter() + offset);

    const size_t bottomLeft = FindClosestVertex(Transform::FromPx(bounds.getCenter() - bounds.getDimensions()*0.5f));
    const size_t topRight = FindClosestVertex(Transform::FromPx(bounds.getCenter() + bounds.getDimensions()*0.5f));

    const size_t startRow = bottomLeft / columnCount;
    const size_t startCol = bottomLeft % columnCount;

    const size_t endRow = topRight / columnCount;
    const size_t endCol = topRight % columnCount;

    for (size_t r = startRow; r <= endRow; r++) {
        for (size_t c = startCol; c <= endCol; c++) {
            size_t i = r * columnCount + c;
            if (bounds.contains(Transform::ToPx(vertices[i].position))) {
                contained.push_back(i);
            }
        }
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

float NavigationMesh::GetDefault(size_t index) const {
    if (!defaults) return 0.5f;
    return defaults[index];
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
    const physx::PxVec3 offset = physx::PxVec3(spacing) + bounds.getDimensions()*0.5f;
    bounds = physx::PxBounds3(bounds.getCenter() - offset, bounds.getCenter() + offset);

    return bounds.contains(Transform::ToPx(vertices[index].position));
}
