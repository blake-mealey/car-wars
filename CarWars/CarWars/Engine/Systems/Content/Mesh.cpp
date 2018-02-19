#include "Mesh.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "../../Entities/Transform.h"

Triangle::Triangle() : vertexIndex0(0), vertexIndex1(0), vertexIndex2(0) {}
Triangle::Triangle(unsigned short _v0, unsigned short _v1, unsigned short _v2) : vertexIndex0(_v0), vertexIndex1(_v1), vertexIndex2(_v2) { }

Mesh::Mesh(size_t _triangleCount, size_t _vertexCount, Triangle* _triangles, glm::vec3* _vertices, glm::vec2* _uvs,
    glm::vec3* _normals) : triangleCount(_triangleCount), vertexCount(_vertexCount), triangles(_triangles), vertices(_vertices), uvs(_uvs), normals(_normals) {
    
    if (!normals) {     // Do we need this still? Taken care of by Assimp?
        GenerateNormals();
    }
}

void Mesh::GenerateNormals() {
	glm::vec3 *_normals = new glm::vec3[vertexCount];

	for (size_t i = 0; i < vertexCount; i++) {
		_normals[i] = glm::vec3(0, 0, 0);;
	}

	for (size_t i = 0; i < vertexCount; i+=3) {
		const glm::vec3 v1 = vertices[i];
		const glm::vec3 triangleNormal = glm::normalize(glm::cross(vertices[i + 1] - v1, vertices[i + 2] - v1));
		for (size_t j = i; j < i + 3; j++) {
			_normals[j] = _normals[j] + triangleNormal;
		}
	}

	for (size_t i = 0; i < vertexCount; i++) {
		_normals[i] = glm::normalize(_normals[i]);
	}

	normals = _normals;
}

Mesh* Mesh::TransformMesh(Transform t) {
    glm::vec3 *transformedVerts = new glm::vec3[vertexCount];

    const glm::mat4 matrix = t.GetTransformationMatrix();
    for (size_t i = 0; i < vertexCount; ++i) {
        transformedVerts[i] = matrix * glm::vec4(vertices[i], 1.f);
    }

    return new Mesh(triangleCount, vertexCount, triangles, transformedVerts, uvs, normals);
}
