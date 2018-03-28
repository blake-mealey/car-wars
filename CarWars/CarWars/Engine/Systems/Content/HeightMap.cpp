#include "HeightMap.h"
#include "Picture.h"
#include "Mesh.h"
#include "../Engine/Systems/Content/ContentManager.h"
#include <math.h>

using namespace glm;

#define wallVertexThick 9
#define wallHeight 30.0f
#define wallThick 2.0f
#define inclineExp 1.9f
#define heightVar 0.0f
#define overhangVar 0.0f
HeightMap::HeightMap(std::string dirPath) {
    nlohmann::json data = ContentManager::LoadJson(ContentManager::MAP_DIR_PATH + dirPath + "Data.json");
    maxHeight = ContentManager::GetFromJson<float>(data["MaxHeight"], 25.f);
    maxWidth = ContentManager::GetFromJson<float>(data["MaxWidth"], 20.f);
    maxLength = ContentManager::GetFromJson<float>(data["MaxLength"], 20.f);
    const std::string filePath = ContentManager::MAP_DIR_PATH + dirPath + "Map.png";

    Initialize(filePath);
}

void HeightMap::Initialize(std::string filePath) {
    Picture* image = new Picture(filePath);

    rowCount = image->Height();
    colCount = image->Width();

    if (rowCount < 2 || colCount < 2) {
        std::cerr << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
    }

	float totalRowCount = rowCount + wallVertexThick * 2;
	float totalColCount = colCount + wallVertexThick * 2;

    zSpacing = maxLength / static_cast<float>(totalRowCount);
    xSpacing = maxWidth / static_cast<float>(totalColCount);

	int v = 0;
    heights = new float*[totalRowCount];

    const size_t vertexCount = (totalRowCount) * (totalColCount);
    const size_t triangleCount = (totalRowCount - 1) * (totalColCount - 1) * 2;
    vec3* vertices = new vec3[vertexCount];
    vec2* uvs = new vec2[vertexCount];
    Triangle* triangles = new Triangle[triangleCount];

	const vec3 offset = -vec3(maxWidth, 0.f, maxLength) * 0.5f;

	for (unsigned long i = 0; i < totalRowCount; i++) {
		heights[i] = new float[totalColCount];
		for (unsigned long j = 0; j < totalColCount; j++) {
			heights[i][j] = wallHeight + maxHeight;
			vertices[v] = glm::vec3(xSpacing*j, wallHeight + maxHeight, zSpacing*i) + offset;
			uvs[v] = vec2(xSpacing*j / static_cast<float>(totalColCount), zSpacing*i / static_cast<float>(totalRowCount));
			v++;
		}
	}

    float* pixels = image->Pixels();
    float z = 0.f;
    v = 0;

	float wallHPV = wallThick / wallVertexThick;
	float wallZspace = 0.0f;
	float wallXspace = 0.0f;
	float inclineRate = wallHeight / (1 + (pow(inclineExp, wallVertexThick) - inclineExp)*(1 / (inclineExp - 1)));
	float currIncline;

	z = zSpacing * wallVertexThick;
	v = wallVertexThick*(totalColCount)+wallVertexThick;
    for (unsigned long i = wallVertexThick; i < rowCount + wallVertexThick; i++) {
        float x = xSpacing*wallVertexThick;

        for (unsigned long j = wallVertexThick; j < colCount + wallVertexThick; j++) {
            const float y = (1.f - (pixels[0] + pixels[1] + pixels[2]) / 3.f) * maxHeight;
            heights[i][j] = y;

            vertices[v] = vec3(x, y, z) + offset;
            uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
            v++;

            pixels += image->Channels();
            x += xSpacing;
        }
		v += wallVertexThick * 2;

        z += zSpacing;
    }

	//Add walls to the left side based on the heights closest to the wall
	v = wallVertexThick*(totalColCount) + wallVertexThick;
	z = zSpacing*wallVertexThick;
	for (unsigned long i = wallVertexThick; i < rowCount + wallVertexThick; i++) {
		currIncline = inclineRate;
		float x = xSpacing*wallVertexThick;
		//const float yoffset = heights[i][wallVertexThick];
		for (int j = wallVertexThick - 1; j >= 0; j--) {
			const float y = heights[i][j+1] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v--;
			currIncline *= inclineExp;
			x -= xSpacing;
		}
		z += zSpacing;
		v += colCount + wallVertexThick*3;
	}

	//Add walls to the right side based on the heights closest to the wall
	v = wallVertexThick*(totalColCount) + wallVertexThick + colCount;
	z = zSpacing*wallVertexThick;
	for (unsigned long i = wallVertexThick; i < rowCount + wallVertexThick; i++) {
		currIncline = inclineRate;
		float x = xSpacing*(wallVertexThick + colCount);
		for (unsigned long j = colCount + wallVertexThick; j < totalColCount; j++) {
			const float y = heights[i][j - 1] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;
			currIncline *= inclineExp;
			x += xSpacing;
		}
		z += zSpacing;
		v += wallVertexThick + colCount;
	}

	//Add walls to the Top based on the heights closest to the wall
	currIncline = inclineRate;
	z = zSpacing*wallVertexThick;
	v = (wallVertexThick-1)*(totalColCount);
	for (int i = wallVertexThick - 1; i >= 0 ; i--) {
		float x = 0.0f;
		for (unsigned long j = 0; j < totalColCount; j++) {
			const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= inclineExp;
		v -= totalColCount * 2;
		z -= zSpacing;
	}

	//Add walls to the Bottom based on the heights closest to the wall
	currIncline = inclineRate;
	z = zSpacing * (rowCount + wallVertexThick);
	v = (wallVertexThick + rowCount)*(totalColCount);
	for (unsigned long i = rowCount; i < rowCount + wallVertexThick; i++) {
		float x = 0.f;
		for (unsigned long j = 0; j < totalColCount; j++) {
			const float y = heights[i - 1][j] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= inclineExp;
		z += zSpacing;
	}

	//Fix the Corners
	/*v = 0;
	unsigned long i = 0;
	unsigned long j = 0;
	heights[i][j] = heights[i + 1][j];
	vertices[v].y *= 0.5;*/


    unsigned long r = 0;
    const unsigned long w = totalColCount;
    int t = 0;
    for (unsigned long i = 0; i < totalRowCount - 1; i++) {
        for (unsigned long j = 0; j < totalColCount - 1; j++) {
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
