#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class SphereCollider : public Collider {
public:
	~SphereCollider() override;
	SphereCollider(std::string _collisionGroup, physx::PxMaterial* _material, physx::PxFilterData _queryFilterData, bool _isTrigger, glm::vec3 _scale);
	SphereCollider(nlohmann::json data);

	ColliderType GetType() const override;

	Mesh* GetRenderMesh() override;

	Transform GetLocalTransform() const override;
	Transform GetGlobalTransform() const override;
	
protected:
	void InitializeGeometry() override;
};