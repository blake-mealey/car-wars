#include "HeightMap.h"
#include "Picture.h"
#include "Mesh.h"
#include "../Engine/Systems/Content/ContentManager.h"
#include <cstdlib>
#include <algorithm>

using namespace glm;

HeightMap::HeightMap(std::string dirPath) {
    nlohmann::json data = ContentManager::LoadJson(ContentManager::MAP_DIR_PATH + dirPath + "Data.json");
    maxHeight = ContentManager::GetFromJson<float>(data["MaxHeight"], 25.f);
    maxWidth = ContentManager::GetFromJson<float>(data["MaxWidth"], 20.f);
    maxLength = ContentManager::GetFromJson<float>(data["MaxLength"], 20.f);
	wallVertices = ContentManager::GetFromJson<unsigned int>(data["WallVertices"], 0);
	wallHeight = ContentManager::GetFromJson<float>(data["WallHeight"], 0.0f);
	wallInclineRate = ContentManager::GetFromJson<float>(data["WallIncline"], 1.0f);
	variation = ContentManager::GetFromJson<float>(data["WallVariation"], 0.0f);
	wallVerTween = ContentManager::GetFromJson<float>(data["WallVerTween"], 1);
	wallMoundMaxVertices = ContentManager::GetFromJson<float>(data["WallMoundMaxVertices"], 0.0);
	wallMoundMinVertices = ContentManager::GetFromJson<float>(data["WallMoundMinVertices"], 0.0);
	wallMoundVariation = ContentManager::GetFromJson<float>(data["WallMoundVariation"], 0.0);


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
	unsigned int  totalRowCount = rowCount + wallVertices * 2;
	unsigned int totalColCount = colCount + wallVertices * 2;

	zSpacing = maxLength / static_cast<float>(rowCount);
	xSpacing = maxWidth / static_cast<float>(colCount);

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
		/*for (unsigned long j = 0; j < totalColCount; j++) {
			heights[i][j] = wallHeight + maxHeight;
			vertices[v] = glm::vec3(xSpacing*j, wallHeight + maxHeight, zSpacing*i) + offset;
			uvs[v] = vec2(xSpacing*j / static_cast<float>(totalColCount), zSpacing*i / static_cast<float>(totalRowCount));
			v++;
		}*/
	}

	float* pixels = image->Pixels();
	float z = 0.f;
	v = 0;

	float inclineRate = 0.0f;

	if (wallVertices != 0)
		inclineRate = wallHeight / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
	float currIncline;

	z = 0.0;
	v = wallVertices*(totalColCount)+wallVertices;
	for (unsigned long i = wallVertices; i < rowCount + wallVertices; i++) {
		float x = 0.0;

		for (unsigned long j = wallVertices; j < colCount + wallVertices; j++) {
			const float y = (1.f - (pixels[0] + pixels[1] + pixels[2]) / 3.f) * maxHeight;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			v++;

			pixels += image->Channels();
			x += xSpacing;
		}
		v += wallVertices * 2;

		z += zSpacing;
	}

	//Add walls to the left side based on the heights closest to the wall
	int mound = 0;
	int moundMax = (int)(((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (1 - wallMoundVariation)) * (wallMoundMaxVertices - wallMoundMinVertices) + wallMoundMinVertices);
	unsigned int tweened = 0;
	float tweenAmount = 0.0f;
	
	v = wallVertices*(totalColCount)+wallVertices-1;
	z = 0.0;
	//float temp = std::min(std::max(-xSpacing + xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation), -xSpacing*3), xSpacing);
	float var = (xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation)) / wallVerTween;
	float temp = -xSpacing;

	for (unsigned long i = wallVertices; i < rowCount + wallVertices; i++) {
		currIncline = inclineRate;
		//float x = -xSpacing + xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation);
		float x;
		if (mound < wallVerTween) {
			temp += var;
			x = temp;
		}
		if (mound > moundMax + wallVerTween) {
			var = (xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation)) / wallVerTween;
			x = temp + var;
			temp = x;
			mound = 0;
			moundMax = (int)(((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (1 - wallMoundVariation)) * (wallMoundMaxVertices - wallMoundMinVertices) + wallMoundMinVertices);
		}
		else {
			x = temp;
		}
		//const float yoffset = heights[i][wallVertices];
		for (int j = wallVertices - 1; j >= 0; j--) {
			const float y = heights[i][j + 1] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v--;
			currIncline *= wallInclineRate;
			x -= xSpacing;
		}
		mound++;
		z += zSpacing;
		v += colCount + wallVertices * 3;
	}

	//Add walls to the right side based on the heights closest to the wall
	v = wallVertices*(totalColCount)+wallVertices + colCount;
	z = 0.0;
	mound = 0;
	moundMax = (int)(((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (1 - wallMoundVariation)) * (wallMoundMaxVertices - wallMoundMinVertices) + wallMoundMinVertices);
	var = (xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation)) / wallVerTween;
	temp = xSpacing*(colCount) + xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation);
	//temp = xSpacing*(colCount)+ xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation);
	for (unsigned long i = wallVertices; i < rowCount + wallVertices; i++) {
		currIncline = inclineRate;
		float x;
		if (mound < wallVerTween) {
			temp += var;
			x = temp;
		}
		if (mound > moundMax + wallVerTween) {
			var = (xSpacing*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (variation * 2) - variation)) / wallVerTween;
			x = temp + var;
			temp = x;
			mound = 0;
			moundMax = (int)(((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (1 - wallMoundVariation)) * (wallMoundMaxVertices - wallMoundMinVertices) + wallMoundMinVertices);
		}
		else {
			x = temp;
		}
		for (unsigned long j = colCount + wallVertices; j < totalColCount; j++) {
			const float y = heights[i][j - 1] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;
			currIncline *= wallInclineRate;
			x += xSpacing;
		}
		mound++;
		z += zSpacing;
		v += wallVertices + colCount;
	}

	//Add walls to the Top based on the heights closest to the wall
	currIncline = inclineRate;
	z = -zSpacing;
	v = (wallVertices - 1)*(totalColCount) + wallVertices;
	for (int i = wallVertices - 1; i >= 0; i--) {
		//float x = -xSpacing*(wallVertices);
		float x = 0;
		for (unsigned long j = wallVertices; j < wallVertices + colCount; j++) {
			const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		//v -= totalColCount * 2;
		v -= (totalColCount + colCount);
		z -= zSpacing;
	}

	//Add walls to the Top Left Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = -zSpacing;
	v = (wallVertices - 1)*(totalColCount);
	for (int i = wallVertices - 1; i >= 0; i--) {
		float x = vertices[(wallVertices - 1)*(totalColCount)+totalColCount].x - offset.x;
		//float x = -xSpacing*wallVertices;
		for (unsigned long j = 0; j < wallVertices; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices][j] - heights[wallVertices][wallVertices])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, (wallVertices - 1) - i);
			const float y = heights[i + 1][j] + tempCurr;
			//const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		v -= (totalColCount + wallVertices);
		z -= zSpacing;
	}

	//Add walls to the Top Right Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = -zSpacing;
	v = (wallVertices - 1)*(totalColCount) + wallVertices + colCount;
	for (int i = wallVertices - 1; i >= 0; i--) {
		float x = vertices[(wallVertices - 1)*(totalColCount)+ wallVertices + colCount + totalColCount].x - offset.x;
		//float x = -xSpacing*wallVertices;
		for (unsigned long j = wallVertices + colCount; j < totalColCount; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices][j] - heights[wallVertices][wallVertices + colCount])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, (wallVertices - 1) - i);
			const float y = heights[i + 1][j] + tempCurr;
			//const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		//v -= totalColCount * 2;
		v -= (totalColCount + colCount);
		z -= zSpacing;
	}

	//Add walls to the Top Left Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = -zSpacing;
	v = (wallVertices - 1)*(totalColCount);
	for (int i = wallVertices - 1; i >= 0; i--) {
		float x = vertices[(wallVertices - 1)*(totalColCount)+totalColCount].x - offset.x;
		//float x = -xSpacing*wallVertices;
		for (unsigned long j = 0; j < wallVertices; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices][j] - heights[wallVertices][wallVertices])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, (wallVertices - 1) - i);
			const float y = heights[i + 1][j] + tempCurr;
			//const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		v -= (totalColCount + wallVertices);
		z -= zSpacing;
	}

	//Add walls to the Top Right Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = -zSpacing;
	v = (wallVertices - 1)*(totalColCount) + wallVertices + colCount;
	for (int i = wallVertices - 1; i >= 0; i--) {
		float x = vertices[(wallVertices - 1)*(totalColCount)+ wallVertices + colCount + totalColCount].x - offset.x;
		//float x = -xSpacing*wallVertices;
		for (unsigned long j = wallVertices + colCount; j < totalColCount; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices][j] - heights[wallVertices][wallVertices + colCount])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, (wallVertices - 1) - i);
			const float y = heights[i + 1][j] + tempCurr;
			//const float y = heights[i + 1][j] + currIncline;
			heights[i][j] = y;

			cout << std::endl;
			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		v -= (totalColCount + wallVertices);
		z -= zSpacing;
	}


	//Add walls to the Bottom based on the heights closest to the wall
	currIncline = inclineRate;
	z = zSpacing * (rowCount);
	v = (wallVertices + rowCount)*(totalColCount) + wallVertices;
	for (unsigned long i = wallVertices + rowCount; i < totalRowCount; i++) {
		float x = 0;
		for (unsigned long j = wallVertices; j < wallVertices + colCount; j++) {
			const float y = heights[i - 1][j] + currIncline;
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(1.f - (static_cast<float>(j - wallVertices) / static_cast<float>(colCount)), static_cast<float>(i - wallVertices) / static_cast<float>(rowCount));
			//uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		z += zSpacing;
		v += wallVertices*2;
	}

	//Add walls to the Bottom Left Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = zSpacing * (rowCount);
	v = (wallVertices + rowCount)*(totalColCount);
	for (unsigned long i = wallVertices + rowCount; i < totalRowCount; i++) {
		float x = vertices[(wallVertices + rowCount)*(totalColCount) - totalColCount].x - offset.x;
		for (unsigned long j = 0; j < wallVertices; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices + rowCount - 1][j] - heights[wallVertices + rowCount - 1][wallVertices])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, i - (wallVertices + rowCount));
			const float y = heights[i - 1][j] + tempCurr;
			//const float y = heights[i - 1][j];
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		z += zSpacing;
		v += colCount + wallVertices;
	}

	//Add walls to the Bottom Right Corner based on the heights closest to the wall
	currIncline = inclineRate;
	z = zSpacing * (rowCount);
	v = (wallVertices + rowCount)*(totalColCount) + (wallVertices + colCount);
	for (unsigned long i = wallVertices + rowCount; i < totalRowCount; i++) {
		float x = vertices[(wallVertices + rowCount)*(totalColCount)+(wallVertices + colCount) - totalColCount].x - offset.x;
		for (unsigned long j = wallVertices + colCount; j < totalColCount; j++) {
			const float tempInclineRate = (wallHeight - (heights[wallVertices + rowCount - 1][j] - heights[wallVertices + rowCount - 1][wallVertices + colCount])) / (1 + (pow(wallInclineRate, wallVertices) - wallInclineRate)*(1 / (wallInclineRate - 1)));
			const float tempCurr = tempInclineRate * pow(wallInclineRate, i - (wallVertices + rowCount));
			const float y = heights[i - 1][j] + tempCurr;
			//const float y = heights[i - 1][j];
			heights[i][j] = y;

			vertices[v] = vec3(x, y, z) + offset;
			uvs[v] = vec2(x / static_cast<float>(totalColCount), z / static_cast<float>(totalRowCount));
			v++;

			x += xSpacing;
		}
		currIncline *= wallInclineRate;
		z += zSpacing;
		v += wallVertices + colCount;
	}


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
	//rowCount = totalRowCount;
	//colCount = totalColCount;
}


float HeightMap::GetHeight(vec3 coords) const {
    const int row = (coords.z + GetLength()*0.5f) / zSpacing;
    const int col = (coords.x + GetWidth()*0.5f) / xSpacing;

    if (row < 0 || row > rowCount - 1 || col < 0 || col > colCount - 1) return -5.f;

    return heights[row+wallVertices][col+wallVertices];
}

float HeightMap::GetWidth() const {
    return maxWidth;
}

float HeightMap::GetLength() const {
    return maxLength;
}

float HeightMap::GetMaxHeight() const {
	return maxHeight;
}

float HeightMap::GetWallHeight() const {
	return wallHeight;
}

unsigned int HeightMap::GetWallVertices() const {
	return wallVertices;
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
