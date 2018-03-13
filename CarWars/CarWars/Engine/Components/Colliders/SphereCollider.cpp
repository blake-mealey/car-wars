#include "SphereCollider.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

SphereCollider::~SphereCollider() {}

SphereCollider::SphereCollider(std::string _collisionGroup, physx::PxMaterial* _material, physx::PxFilterData _queryFilterData, bool _isTrigger, glm::vec3 _scale) : 
Collider(_collisionGroup, _material, _queryFilterData, _isTrigger) {
	transform.SetScale(_scale);
	InitializeGeometry();
}

SphereCollider::SphereCollider(nlohmann::json data) : Collider(data) {
	InitializeGeometry();
}

ColliderType SphereCollider::GetType() const {
	return Collider_Sphere;
}

Mesh* SphereCollider::GetRenderMesh() {
	return ContentManager::GetMesh("Sphere.obj");
}

Transform SphereCollider::GetLocalTransform() const {
	Transform transform = Collider::GetLocalTransform();
	transform.Scale(0.5f);
	return transform;
}

Transform SphereCollider::GetGlobalTransform() const {
	Transform transform = Collider::GetGlobalTransform();
	transform.Scale(0.5f);
	return transform;
}

void SphereCollider::InitializeGeometry() {
	if (geometry != nullptr) delete geometry;
	geometry = new PxSphereGeometry(transform.GetLocalScale().x);
}