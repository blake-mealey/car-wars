#include "Collider.h"
#include <extensions/PxRigidActorExt.h>
#include "../../Systems/Physics/CollisionGroups.h"
#include "../../Systems/Physics/RaycastGroups.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/VehicleSceneQuery.h"
#include "imgui/imgui.h"

using namespace physx;

Collider::Collider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, bool _isTrigger)
    : collisionGroup(_collisionGroup), material(_material), queryFilterData(_queryFilterData), shape(nullptr), geometry(nullptr), isTrigger(_isTrigger) {}

Collider::Collider(nlohmann::json data) : shape(nullptr), geometry(nullptr) {
    collisionGroup = ContentManager::GetFromJson<std::string>(data["CollisionGroup"], "Default");
    material = ContentManager::GetPxMaterial(ContentManager::GetFromJson<std::string>(data["Material"], "Default.json"));
	queryFilterData.word0 = RaycastGroups::GetDefaultGroup();
    transform = Transform(data);
	isTrigger = ContentManager::GetFromJson<bool>(data["IsTrigger"], false);
    const std::string queryFilterType = ContentManager::GetFromJson<std::string>(data["QueryFilterType"], isTrigger ? "NonDrivableSurface" : "DrivableSurface");
    if (queryFilterType == "DrivableSurface") {
        setupDrivableSurface(queryFilterData);
    }
    else {
        setupNonDrivableSurface(queryFilterData);
    }
}

Collider::~Collider() {
    if (shape->getActor()) {
        shape->getActor()->detachShape(*shape);
    } else {
        shape->release();
    }
    delete geometry;
}

physx::PxShape* Collider::GetShape() const {
    return shape;
}

void Collider::CreateShape(PxRigidActor *actor) {
    shape = physx::PxRigidActorExt::createExclusiveShape(*actor, *geometry, *material);

	if (isTrigger) {
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}

    shape->setQueryFilterData(queryFilterData);                                         // For raycasts
    shape->setSimulationFilterData(CollisionGroups::GetFilterData(collisionGroup));     // For collisions
    shape->setLocalPose(Transform::ToPx(transform));
}

void Collider::RenderDebugGui() {
    if (ImGui::TreeNode("Transform")) {
		if (transform.RenderDebugGui()) {
			shape->setLocalPose(Transform::ToPx(transform));
			UpdateScale(transform.GetGlobalScale());
		}
        ImGui::TreePop();
    }
    ImGui::LabelText("Collision Group", "%s", collisionGroup);
    ImGui::Text("Static Friction: %f", material->getStaticFriction());
    ImGui::Text("Dynamic Friction: %f", material->getDynamicFriction());
    ImGui::Text("Restitution Friction: %f", material->getRestitution());
}

std::string Collider::GetTypeName(ColliderType type) {
    switch(type) {
        case Collider_Box: return "Box";
        case Collider_ConvexMesh: return "ConvexMesh";
        case Collider_TriangleMesh: return "TriangleMesh";
        default: return std::to_string(type);
    }
}

Transform Collider::GetLocalTransform() const {
    Transform pose = shape->getLocalPose();
	pose.SetScale(transform.GetLocalScale());
	return pose;
}

Transform Collider::GetGlobalTransform() const {
	Transform pose = PxShapeExt::getGlobalPose(*shape, *shape->getActor());
	pose.SetScale(transform.GetLocalScale());
	return pose;
}

void Collider::Scale(glm::vec3 scaleFactor) {
	UpdateScale(transform.GetLocalScale() * scaleFactor);
}

void Collider::UpdateScale(glm::vec3 scale) {
	transform.SetScale(scale);
	if (shape != nullptr) {
		InitializeGeometry();
		shape->setGeometry(*geometry);
	}
}