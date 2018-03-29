#include "Mesh.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "../../Entities/Transform.h"

Triangle::Triangle() : vertexIndex0(0), vertexIndex1(0), vertexIndex2(0) {}
Triangle::Triangle(unsigned int _v0, unsigned int _v1, unsigned int _v2) : vertexIndex0(_v0), vertexIndex1(_v1), vertexIndex2(_v2) { }

Mesh::Mesh(size_t _triangleCount, size_t _vertexCount, Triangle* _triangles, glm::vec3* _vertices, glm::vec2* _uvs,
    glm::vec3* _normals) : triangleCount(_triangleCount), vertexCount(_vertexCount) {
    
	// Generate normals if they were not provided
    if (!_normals) {
		_normals = new glm::vec3[vertexCount];
        GenerateNormals(_triangles, _vertices, _normals);
    }

	// Compute the radius in case this mesh is later attached to the cylinder (?)
	CalculateRadius(_vertices);

	// Initialize OpenGL buffers for the provided data
    InitializeBuffers(_triangles, _vertices, _uvs, _normals);
}

Mesh::~Mesh() {
    glDeleteBuffers(EABs::Count, eabs);
    glDeleteBuffers(VBOs::Count, vbos);
    glDeleteVertexArrays(VAOs::Count, vaos);
}

void Mesh::GenerateNormals(Triangle* triangles, glm::vec3* vertices, glm::vec3* normals) {
	for (size_t i = 0; i < vertexCount; ++i) {
		normals[i] = glm::vec3(0.f);;
	}

    for (size_t i = 0; i < triangleCount; ++i) {
        const Triangle triangle = triangles[i];
        const glm::vec3 v0 = vertices[triangle.vertexIndex0];
        const glm::vec3 v1 = vertices[triangle.vertexIndex1];
        const glm::vec3 v2 = vertices[triangle.vertexIndex2];
        const glm::vec3 triangleNormal = normalize(cross(v1 - v0, v2 - v0));
        normals[triangle.vertexIndex0] += triangleNormal;
        normals[triangle.vertexIndex1] += triangleNormal;
        normals[triangle.vertexIndex2] += triangleNormal;
    }

	for (size_t i = 0; i < vertexCount; i++) {
		normals[i] = normalize(normals[i]);
	}
}

float Mesh::GetRadius() const {
	return radius;
}

void Mesh::CalculateRadius(glm::vec3 *vertices) {
	float maxX = vertices[0].x;
	float minX = vertices[0].x;
	for (size_t i = 1; i < vertexCount; ++i) {
		maxX = std::max(maxX, vertices[i].x);
		minX = std::min(minX, vertices[i].x);
	}
	radius = (maxX - minX) / 2.f;
}

void Mesh::InitializeBuffers(Triangle *triangles, glm::vec3 *vertices, glm::vec2 *uvs, glm::vec3 *normals) {
    glGenBuffers(EABs::Count, eabs);
    InitializeIndexBuffer(triangles);

    glGenBuffers(VBOs::Count, vbos);
    InitializeGeometryBuffers(vertices, uvs, normals);

    glGenVertexArrays(VAOs::Count, vaos);
    InitializeGeometryVao();
    InitializeVerticesVao();
    InitializeUvsVao();
}

void Mesh::InitializeIndexBuffer(Triangle *triangles) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabs[EABs::Triangles]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Triangle) * triangleCount, triangles, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::InitializeGeometryBuffers(glm::vec3 *vertices, glm::vec2 *uvs, glm::vec3 *normals) {
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::Vertices]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::UVs]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertexCount, uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::Normals]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::InitializeGeometryVao() {
    glBindVertexArray(vaos[VAOs::Geometry]);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::Vertices]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    // UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::UVs]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    // Normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::Normals]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::InitializeVerticesVao() {
    glBindVertexArray(vaos[VAOs::Vertices]);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::Vertices]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::InitializeUvsVao() {
    glBindVertexArray(vaos[VAOs::UVs]);

    // UVs
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[VBOs::UVs]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}