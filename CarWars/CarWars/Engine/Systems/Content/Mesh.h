#pragma once

#include <glm/glm.hpp>

class Mesh {
public:
	Mesh(glm::vec3 *_vertices, glm::vec2 *_uvs, glm::vec3 *_normals, size_t _vertexCount);
	Mesh(glm::vec3 *_vertices, glm::vec2 *_uvs, size_t _vertexCount);

	glm::vec3 *vertices;
	glm::vec2 *uvs;
	glm::vec3 *normals;

	const size_t vertexCount;
	void GenerateNormals();
private:
	
};
