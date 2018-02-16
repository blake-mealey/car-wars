#pragma once

#include "Component.h"
#include "../Systems/Content/Mesh.h"
#include <string>
#include "../Entities/Transform.h"
#include "../Systems/Content/Material.h"
#include "../Systems/Content/Texture.h"
#include <json/json.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

class MeshComponent : public Component {
public:
	Transform transform;		// Temporary?

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;
	
	MeshComponent(nlohmann::json data);
	MeshComponent(std::string meshPath, std::string materialPath);
	MeshComponent(std::string meshPath, Material *_material);
	MeshComponent(std::string meshPath, std::string materialPath, std::string texturePath);
	MeshComponent(MeshComponent* component);

	void MakeCylinder(Mesh* mesh);

	void SetEntity(Entity* _entity) override;

	Mesh* GetMesh() const;
	Material* GetMaterial() const;
	Texture* GetTexture() const;
	glm::vec2 GetUvScale() const;
private:
	Mesh *mesh;
	Material *material;
	Texture *texture;
	glm::vec2 uvScale;
};
