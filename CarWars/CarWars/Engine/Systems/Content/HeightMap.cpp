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

    rowCount = image->Height();
    colCount = image->Width();

    if (rowCount < 2 || colCount < 2) {
        std::cerr << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
    }
	
    zSpacing = maxLength / static_cast<float>(rowCount);
	xSpacing = maxWidth / static_cast<float>(colCount);
    
    heights = new float*[rowCount];

    const size_t vertexCount = rowCount * colCount;
    const size_t triangleCount = (rowCount - 1) * (colCount - 1) * 2;
    vec3* vertices = new vec3[vertexCount];
    vec2* uvs = new vec2[vertexCount];
    Triangle* triangles = new Triangle[triangleCount];
	
    const vec3 offset = -vec3(maxWidth, 0.f, maxLength) * 0.5f;
    float* pixels = image->Pixels();
	float z = 0.f;
    int v = 0;
	for (unsigned int i = 0; i < rowCount; i++) {
		float x = 0.f;
        heights[i] = new float[colCount];
		
	    for (unsigned int j = 0; j < colCount; j++) {
            const float y = (1.f - (pixels[0] + pixels[1] + pixels[2]) / 3.f) * maxHeight;
            heights[i][j] = y;
            
		    vertices[v] = vec3(x, y, z) + offset;
            uvs[v] = vec2(x / static_cast<float>(colCount), z / static_cast<float>(rowCount));
            v++;
			
		    pixels += image->Channels();
			x += xSpacing;
		}
		z += zSpacing;
	}
	
    unsigned int r = 0;
	const unsigned int w = colCount;
    int t = 0;
	for (unsigned int i = 0; i < rowCount - 1; i++) {
		for (unsigned int j = 0; j < colCount - 1; j++) {
            triangles[t++] = Triangle((r + j), (r + j + w), (r + j + 1));
            triangles[t++] = Triangle((r + j + 1), (r + j + w), (r + j + w + 1));
		}
		r += w;
	}

    mesh = new Mesh(triangleCount, vertexCount, triangles, vertices, uvs);

	delete image;
}

float HeightMap::GetHeight(vec3 coords) const {
    const int row = (coords.z + GetLength()*0.5f) / zSpacing;
    const int col = (coords.x + GetWidth()*0.5f) / xSpacing;

    if (row < 0 || row > rowCount - 1 || col < 0 || col > colCount - 1) return -5.f;

    return heights[row][col];
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
    return mesh;
}
