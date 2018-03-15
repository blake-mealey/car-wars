#include "HeightMapComponent.h"
#include "../Systems/Content/HeightMap.h"

#include "../Systems/Content/ContentManager.h"
#include "../Entities/Entity.h"

#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

HeightMapComponent::HeightMapComponent(nlohmann::json data) {
	mesh = CreateMesh(data);
	material = ContentManager::GetMaterial(data["Material"]);
	if (!data["Texture"].is_null()) texture = ContentManager::GetTexture(data["Texture"]);
	else texture = nullptr;
	uvScale = ContentManager::JsonToVec2(data["UvScale"], glm::vec2(1.f));
	if (ContentManager::GetFromJson<bool>(data["CylinderMesh"], false)) MakeCylinder(mesh);
	transform = Transform(data);
}

Mesh* HeightMapComponent::CreateMesh(nlohmann::json data) {
	const unsigned int maxHeight = ContentManager::GetFromJson<unsigned int>(data["MaxHeight"], 20);
	const unsigned int maxWidth = ContentManager::GetFromJson<unsigned int>(data["MaxWidth"], 20);
	const unsigned int maxLength = ContentManager::GetFromJson<unsigned int>(data["MaxLength"], 20);
	const float uvstep = ContentManager::GetFromJson<float>(data["UVStep"], 0.5f);
	std::string fileString = data["Map"];
	fileString = ContentManager::SCENE_DIR_PATH + "Maps/" + fileString;
	//char file[fileString.length()];
	//strcpy(file, fileString.c_str());
	HeightMap hm = HeightMap(&fileString[0], maxHeight, maxWidth, maxLength, uvstep);
	vector<Triangle>& triangles = hm.Triangles();
	vector<glm::vec3>& vertices = hm.Vec3Vertices();
	vector<glm::vec2>& uvs = hm.Vec2UVS();
	Mesh m = Mesh(triangles.size(), vertices.size(), &triangles[0], &vertices[0], &uvs[0]);
	return &m;
}