#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "../Graphics.h"


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
    ~Mesh();

    Triangle *triangles;
	glm::vec3 *vertices;
	glm::vec2 *uvs;
	glm::vec3 *normals;

    GLuint eabs[EABs::Count];
    GLuint vbos[VBOs::Count];
    GLuint vaos[VAOs::Count];

	const size_t triangleCount;
	const size_t vertexCount;
	void GenerateNormals();

    Mesh* TransformMesh(Transform t);
private:
    void InitializeBuffers();

    void InitializeIndexBuffer();
    void InitializeGeometryBuffers();

    void InitializeGeometryVao();
    void InitializeVerticesVao();
    void InitializeUvsVao();
};
