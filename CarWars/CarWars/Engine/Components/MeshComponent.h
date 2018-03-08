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

class MeshComponent : public Component<MeshComponent> {
	friend class Component<MeshComponent>;
public:
	//Transform transform;
	unsigned short transformID;
	static constexpr ComponentType InternalGetType() { return ComponentType_Mesh; }
	//void HandleEvent(Event* event) override;

	MeshComponent(nlohmann::json data);
	MeshComponent(std::string meshPath, std::string materialPath);
	MeshComponent(std::string meshPath, Material *_material);
	MeshComponent(std::string meshPath, std::string materialPath, std::string texturePath);
	MeshComponent(MeshComponent* component);

	void MakeCylinder(Mesh* mesh);

	Mesh* GetMesh() const;
	Material* GetMaterial() const;
	Texture* GetTexture() const;
	glm::vec2 GetUvScale() const;

	void InternalRenderDebugGui();
private:
	Mesh *mesh;
	Material *material;
	Texture *texture;
	glm::vec2 uvScale;
protected:
	void InternalSetEntity(Entity& _entity);
};
