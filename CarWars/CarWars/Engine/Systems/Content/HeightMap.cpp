#include "HeightMap.h"
#include "Picture.h"
#include "Mesh.h"
#include "../Engine/Systems/Content/ContentManager.h"

using namespace glm;

HeightMap::HeightMap(nlohmann::json data) {
	const unsigned int maxHeight = ContentManager::GetFromJson<unsigned int>(data["MaxHeight"], 20);
	const unsigned int maxWidth = ContentManager::GetFromJson<unsigned int>(data["MaxWidth"], 20);
	const unsigned int maxLength = ContentManager::GetFromJson<unsigned int>(data["MaxLength"], 20);
	const float uvstep = ContentManager::GetFromJson<float>(data["UVStep"], 0.5f);
	const std::string filePath = ContentManager::SCENE_DIR_PATH + "Maps/" + data["Map"].get<std::string>();

	Picture* image = new Picture(filePath);
	const int length = maxLength / image->Height();
	const int width = maxWidth / image->Width();

	float uvstepx;
	float uvstepy;

	float ratio;

	if (width > length) {
		ratio = static_cast<float>(length) / static_cast<float>(width);
		uvstepy = uvstep * ratio;
		uvstepx = uvstep * (1 - ratio);
	}
	else {
		ratio = static_cast<float>(width) / static_cast<float>(length);
		uvstepx = uvstep * ratio;
		uvstepy = uvstep * (1.f - ratio);
	}

	if (image->Height() < 2 || image->Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	
    float* pixels = image->Pixels();
	float x, z = 0.0f;
	for (unsigned int i = 0; i < image->Height(); i++) {
		x = 0.f;
		for (unsigned int j = 0; j < image->Width(); j++) {
            const float y = (1.f - (pixels[0] + pixels[1] + pixels[2]) / 3.f)*static_cast<float>(maxHeight);
			vertices.push_back(vec3(x, y, z));
			pixels += image->Channels();
			x += width;
		}
		z += length;
	}
	unsigned int r = 0;
	unsigned int w = image->Width();
	for (unsigned int i = 0; i < image->Height() - 1; i++) {
		for (unsigned int j = 0; j < image->Width() - 1; j++) {
			triangles.push_back(Triangle((r + j), (r + j + 1), (r + j + w)));
			triangles.push_back(Triangle((r + j + 1), (r + j + w), (r + j + w + 1)));
		}
		r += w;
	}

	float y = 0.0f;
	for (unsigned int i = 0; i < image->Height(); i++) {
		x = 0.0f;
		for (unsigned int j = 0; j < image->Width(); j++) {
			uvs.push_back(glm::vec2(x, y));
			x += uvstepx;
		}
		y += uvstepy;
	}
	delete image;
}

float HeightMap::GetHeight(glm::vec3 coords) {
	return 1;
}

vector<glm::vec3> HeightMap::Vertices() {
	return vertices;
}

vector<Triangle> HeightMap::Triangles() {
	return triangles;
}

vector<glm::vec2> HeightMap::UVS() {
	return uvs;
}

Mesh* HeightMap::GetMesh() {
    if (!mesh) CreateMesh();
    return mesh;
}

void HeightMap::PrintVertices() {
	for (int i = 0; i < vertices.size(); i ++) {
		cout << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << std::endl;
	}
	cout << std::endl << std::endl;
}

void HeightMap::PrintElements() {
	for (int i = 0; i < triangles.size(); i ++) {
		cout << triangles[i].vertexIndex0 << ", " << triangles[i].vertexIndex1 << ", " << triangles[i].vertexIndex2 << std::endl;
	}
	cout << std::endl << std::endl;
}

void HeightMap::CreateMesh() {
    mesh = new Mesh(triangles.size(), vertices.size(), triangles.data(), vertices.data(), uvs.data());
}