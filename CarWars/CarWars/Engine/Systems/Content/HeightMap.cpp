#include "HeightMap.h"
#include "Picture.h"
#include "Mesh.h"
#include "../Engine/Systems/Content/ContentManager.h"

using namespace glm;

HeightMap::HeightMap(nlohmann::json data) {
	maxHeight = ContentManager::GetFromJson<float>(data["MaxHeight"], 25.f);
	maxWidth = ContentManager::GetFromJson<float>(data["MaxWidth"], 20.f);
	maxLength = ContentManager::GetFromJson<float>(data["MaxLength"], 20.f);
	
    const std::string filePath = ContentManager::HEIGHT_MAP_DIR_PATH + data["Map"].get<std::string>();
	Picture* image = new Picture(filePath);
	
    zSpacing = maxLength / static_cast<float>(image->Height());
	xSpacing = maxWidth / static_cast<float>(image->Width());

	if (image->Height() < 2 || image->Width() < 2) {
		std::cerr << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	
    const vec3 offset = -vec3(maxWidth, 0.f, maxLength) * 0.5f;
    float* pixels = image->Pixels();
	float z = 0.f;
	for (unsigned int i = 0; i < image->Height(); i++) {
		float x = 0.f;
		for (unsigned int j = 0; j < image->Width(); j++) {
            const float y = (1.f - (pixels[0] + pixels[1] + pixels[2]) / 3.f) * maxHeight;
			vertices.push_back(vec3(x, y, z) + offset);
            uvs.push_back(vec2(x / static_cast<float>(image->Width()), z / static_cast<float>(image->Height())));
			pixels += image->Channels();
			x += xSpacing;
		}
		z += zSpacing;
	}
	
    unsigned int r = 0;
	unsigned int w = image->Width();
	for (unsigned int i = 0; i < image->Height() - 1; i++) {
		for (unsigned int j = 0; j < image->Width() - 1; j++) {
			triangles.push_back(Triangle((r + j), (r + j + w), (r + j + 1)));
			triangles.push_back(Triangle((r + j + 1), (r + j + w), (r + j + w + 1)));
		}
		r += w;
	}

	delete image;
}

float HeightMap::GetHeight(vec3 coords) const {

	return 1;
}

float HeightMap::GetWidth() const {
    return maxWidth;
}

float HeightMap::GetLength() const {
    return maxLength;
}

float HeightMap::GetXSpacing() const {
    return xSpacing;
}

float HeightMap::GetZSpacing() const {
    return zSpacing;
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
