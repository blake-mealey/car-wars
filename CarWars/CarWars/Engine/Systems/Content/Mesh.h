#pragma once

#include <glm/glm.hpp>


class Transform;

struct Triangle {
    Triangle();
    Triangle(unsigned short _v0, unsigned short _v1, unsigned short _v2);
    unsigned short vertexIndex0;
    unsigned short vertexIndex1;
    unsigned short vertexIndex2;
};

class Mesh {
public:
    Mesh(size_t _triangleCount, size_t _vertexCount, Triangle *_triangles, glm::vec3 *_vertices, glm::vec2 *_uvs = nullptr, glm::vec3 *_normals = nullptr);

    Triangle *triangles;
	glm::vec3 *vertices;
	glm::vec2 *uvs;
	glm::vec3 *normals;

	const size_t triangleCount;
	const size_t vertexCount;
	void GenerateNormals();

    Mesh* TransformMesh(Transform t);
};
