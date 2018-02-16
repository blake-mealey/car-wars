#include "Mesh.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

Mesh::Mesh(glm::vec3* _vertices, glm::vec2* _uvs, glm::vec3* _normals, size_t _vertexCount) :
	vertices(_vertices), uvs(_uvs), normals(_normals), vertexCount(_vertexCount) { }

Mesh::Mesh(glm::vec3 *_vertices, glm::vec2 *_uvs, size_t _vertexCount) :
	vertices(_vertices), uvs(_uvs), vertexCount(_vertexCount) {

	GenerateNormals();
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
