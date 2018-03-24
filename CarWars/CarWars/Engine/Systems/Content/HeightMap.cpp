#include "HeightMap.h"
#include "Picture.h"
#include "Mesh.h"
#include "../Engine/Systems/Content/ContentManager.h"

using glm::vec3;

/*HeightMap::HeightMap(Picture& image, const int height, const int width, const int length) {
	if (image.Height() < 2 || image.Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	float* pixels = image.Pixels();
	float x = 0.0f, z = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0;
		for (unsigned int j = 0; j < image.Width(); j++) {
			vertices.push_back(glm::vec3(x, (1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*height, z));
			//vertices.push_back(x);
			//vertices.push_back((1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*height);
			//vertices.push_back(z);
			pixels += image.Channels();
			x += width;
		}
		z += length;
	}
	unsigned int r = 0;
	unsigned int w = image.Width();
	for (unsigned int i = 0; i < image.Height() - 1; i++) {
		for (unsigned int j = 0; j < image.Width() - 1; j++) {
			triangles.push_back(Triangle((r + j), (r + j + 1), (r + j + w)));
			triangles.push_back(Triangle((r + j + 1), (r + j + w), (r + j + w + 1)));
			//triangles.push_back(r + j);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + w + 1);
		}
		r += w;
	}
}*/

HeightMap::HeightMap(nlohmann::json data) {
	const unsigned int maxHeight = ContentManager::GetFromJson<unsigned int>(data["MaxHeight"], 20);
	const unsigned int maxWidth = ContentManager::GetFromJson<unsigned int>(data["MaxWidth"], 20);
	const unsigned int maxLength = ContentManager::GetFromJson<unsigned int>(data["MaxLength"], 20);
	//const unsigned int maxHeight = 20;
	//const unsigned int maxWidth = 20;
	//const unsigned int maxLength = 20;
	const float uvstep = ContentManager::GetFromJson<float>(data["UVStep"], 0.5f);
	//const float uvstep = 20.0f;
	std::string fileString = data["Map"];
	//std::string fileString = "spike.png";
	fileString = ContentManager::SCENE_DIR_PATH + "Maps/" + fileString;
	//char file[fileString.length()];
	//strcpy(file, fileString.c_str());
	const char* file = fileString.c_str();

	/*Picture* image = new Picture(file);
	const int length = maxLength / image->Height();
	const int width = maxWidth / image->Width();

	float uvstepx;
	float uvstepy;

	float ratio;

	if (width > length) {
		ratio = (float)length / (float)width;
		uvstepy = uvstep * ratio;
		uvstepx = uvstep * (1 - ratio);
	}
	else {
		ratio = (float)width / (float)length;
		uvstepx = uvstep * ratio;
		uvstepy = uvstep * (1 - ratio);
	}

	if (image->Height() < 2 || image->Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	float* pixels = image->Pixels();
	float x = 0.0f, z = 0.0f;
	for (unsigned int i = 0; i < image->Height(); i++) {
		x = 0;
		for (unsigned int j = 0; j < image->Width(); j++) {
			vertices.push_back(glm::vec3(x, (1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*maxHeight, z));
			//vertices.push_back(x);
			//vertices.push_back((1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*maxHeight);
			//vertices.push_back(z);
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
			//triangles.push_back(r + j);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + w + 1);
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
	delete image;*/
}

/*HeightMap::HeightMap(const char* file, const int height, const int width, const int length) : HeightMap(Picture(file), height, width, length) { }

//This is the one we will be using
HeightMap::HeightMap(const char* file, const int maxHeight, const int maxWidth, const int maxLength, const float uvstep) {
	Picture image = Picture(file);
	const int length = maxLength / image.Height();
	const int width = maxWidth / image.Width();

	float uvstepx;
	float uvstepy;

	float ratio;

	if (width > length) {
		ratio = (float)length / (float)width;
		uvstepy = uvstep * ratio;
		uvstepx = uvstep * (1 - ratio);
	}
	else {
		ratio = (float)width / (float)length;
		uvstepx = uvstep * ratio;
		uvstepy = uvstep * (1 - ratio);
	}

	if (image.Height() < 2 || image.Width() < 2) {
		cout << "Cannot create a 3D map from your 1D Drawing. Draw a Proper Map, Not A Trail. I'm Not That Kind of Terrain Generator.";
	}
	float* pixels = image.Pixels();
	float x = 0.0f, z = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0;
		for (unsigned int j = 0; j < image.Width(); j++) {
			vertices.push_back(glm::vec3(x, (1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*maxHeight, z));
			//vertices.push_back(x);
			//vertices.push_back((1 - (pixels[0] + pixels[1] + pixels[2]) / 3)*maxHeight);
			//vertices.push_back(z);
			pixels += image.Channels();
			x += width;
		}
		z += length;
	}
	unsigned int r = 0;
	unsigned int w = image.Width();
	for (unsigned int i = 0; i < image.Height() - 1; i++) {
		for (unsigned int j = 0; j < image.Width() - 1; j++) {
			triangles.push_back(Triangle((r + j), (r + j + 1), (r + j + w)));
			triangles.push_back(Triangle((r + j + 1), (r + j + w), (r + j + w + 1)));
			//triangles.push_back(r + j);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + 1);
			//triangles.push_back(r + j + w);
			//triangles.push_back(r + j + w + 1);
		}
		r += w;
	}

	float y = 0.0f;
	for (unsigned int i = 0; i < image.Height(); i++) {
		x = 0.0f;
		for (unsigned int j = 0; j < image.Width(); j++) {
			uvs.push_back(glm::vec2(x, y));
			x += uvstepx;
		}
		y += uvstepy;
	}
	
}*/

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
    mesh = ContentManager::GetMesh("CircleMap.obj");
//	mesh = new Mesh(triangles.size(), vertices.size(), &triangles[0], &vertices[0], &uvs[0]);
}

/*Mesh* HeightMap::CreateMesh(nlohmann::json data) {
	const unsigned int maxHeight = ContentManager::GetFromJson<unsigned int>(data["MaxHeight"], 20);
	const unsigned int maxWidth = ContentManager::GetFromJson<unsigned int>(data["MaxWidth"], 20);
	const unsigned int maxLength = ContentManager::GetFromJson<unsigned int>(data["MaxLength"], 20);
	//const unsigned int maxHeight = 20;
	//const unsigned int maxWidth = 20;
	//const unsigned int maxLength = 20;
	const float uvstep = ContentManager::GetFromJson<float>(data["UVStep"], 0.5f);
	//const float uvstep = 20.0f;
	std::string fileString = data["Map"];
	//std::string fileString = "spike.png";
	fileString = ContentManager::SCENE_DIR_PATH + "Maps/" + fileString;
	//char file[fileString.length()];
	//strcpy(file, fileString.c_str());
	HeightMap hm = HeightMap(&fileString[0], maxHeight, maxWidth, maxLength, uvstep);
	vector<Triangle>& triangles = hm.Triangles();
	vector<glm::vec3>& vertices = hm.Vertices();
	vector<glm::vec2>& uvs = hm.UVS();
	Mesh* m = new Mesh(triangles.size(), vertices.size(), &triangles[0], &vertices[0], &uvs[0]);
	//ContentManager::StoreMesh(fileString, m);
	return m;
}*/

/*vector<Triangle>& HeightMap::Triangles() {
	vector<Triangle> triangles;
	for (unsigned int i = 0; i < triangles.size(); i += 3) {
		triangles.push_back(Triangle(triangles[i], triangles[i+1], triangles[i+2]));
	}
	return triangles;
}

vector<glm::vec3>& HeightMap::Vec3Vertices() {
	vector<glm::vec3> vecs;
	for (unsigned int i = 0; i < vertices.size(); i += 3) {
		vecs.push_back(glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]));
	}
	return vecs;
}

vector<glm::vec2>& HeightMap::Vec2UVS() {
	vector<glm::vec2> UVs;
	for (unsigned int i = 0; i < uvs.size(); i += 2) {
		UVs.push_back(glm::vec2(uvs[i], uvs[i + 1]));
	}
	return UVs;
}*/