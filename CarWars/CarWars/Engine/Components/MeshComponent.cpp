#include "MeshComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Entities/Entity.h"

#include <iostream>

ComponentType MeshComponent::GetType() {
	return ComponentType_Mesh;
}

void MeshComponent::HandleEvent(Event* event) {}

MeshComponent::MeshComponent(nlohmann::json data) {
	mesh = ContentManager::GetMesh(data["Mesh"]);
	material = ContentManager::GetMaterial(data["Material"]);
	if (!data["Texture"].is_null()) texture = ContentManager::GetTexture(data["Texture"]);
	else texture = nullptr;
	uvScale = ContentManager::JsonToVec2(data["UvScale"], glm::vec2(1.f));
    if (ContentManager::GetFromJson<bool>(data["CylinderMesh"], false)) MakeCylinder(mesh);
	glm::vec3 pos = ContentManager::JsonToVec3(data["Position"], glm::vec3());
	glm::vec3 rot = ContentManager::JsonToVec3(data["Rotate"], glm::vec3());
	glm::vec3 scale = ContentManager::JsonToVec3(data["Scale"], glm::vec3(1.0f));
	glm::vec3 rotRad = glm::vec3(glm::radians(rot.x), glm::radians(rot.y), glm::radians(rot.z));
	transform = Transform(nullptr, pos, scale, rotRad, false);
}

MeshComponent::MeshComponent(MeshComponent* component) {
	mesh = component->GetMesh();
	material = component->GetMaterial();
	texture = component->GetTexture();
	uvScale = component->GetUvScale();
}

MeshComponent::MeshComponent(std::string meshPath, std::string materialPath) : texture(nullptr) {
	mesh = ContentManager::GetMesh(meshPath);
	material = ContentManager::GetMaterial(materialPath);
}

MeshComponent::MeshComponent(const std::string meshPath, const std::string materialPath, const std::string texturePath) : uvScale(glm::vec2(1.f)), transform(Transform()) {
	mesh = ContentManager::GetMesh(meshPath);
    material = ContentManager::GetMaterial(materialPath);
    texture = ContentManager::GetTexture(texturePath);
}

MeshComponent::MeshComponent(std::string meshPath, Material *_material) : material(_material), uvScale(glm::vec2(1.f)), texture(nullptr) {
	mesh = ContentManager::GetMesh(meshPath);
}

Mesh* MeshComponent::GetMesh() const {
	return mesh;
}

Material* MeshComponent::GetMaterial() const {
	return material;
}

Texture* MeshComponent::GetTexture() const {
	return texture;
}

glm::vec2 MeshComponent::GetUvScale() const {
	return uvScale;
}

void MeshComponent::SetEntity(Entity* _entity) {
	Component::SetEntity(_entity);
	transform.parent = &_entity->transform;
}

void MeshComponent::MakeCylinder(Mesh* mesh) {
	if (Transform::radius<=0) {
		float maxX = mesh->vertices[0].x;
		float minX = mesh->vertices[0].x;
		for (size_t i = 1; i < mesh->vertexCount; ++i) {
			maxX = std::max(maxX, mesh->vertices[i].x);
			minX = std::min(minX, mesh->vertices[i].x);
		}
		float circumfrence = (maxX - minX);
		Transform::radius = circumfrence / 2.f / (float)M_PI;
	}

	for (size_t i = 0; i < mesh->vertexCount; ++i) {
		glm::vec3 point = { mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z };
		point = Transform().ToCylinder(point);
		mesh->vertices[i] = point;
	}
	mesh->GenerateNormals();
}