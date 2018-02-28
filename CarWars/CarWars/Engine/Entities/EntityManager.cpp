#include "EntityManager.h"
#include "Entity.h"
#include "Transform.h"
#include "../Components/CameraComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/DirectionLightComponent.h"
#include "../Components/PointLightComponent.h"
#include "../Components/SpotLightComponent.h"
#include "../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Components/WeaponComponents/MachineGunComponent.h"
#include "../Components/WeaponComponents/RailGunComponent.h"
#include "../Components/WeaponComponents/RocketLauncherComponent.h"

Entity* EntityManager::root = nullptr;
vector<Entity> EntityManager::dynamicEntities;
vector<Entity> EntityManager::staticEntities;
vector<Transform> EntityManager::transforms;
//vector<Entity> EntityManager::entities;

ComponentTuple EntityManager::components;

unsigned short EntityManager::GetEntityIndex(short id) {
	return id & ~(infoMask << idEnd);
}

Entity* EntityManager::FindEntity(short id) {
	unsigned short i = id & ~(infoMask << idEnd);
	return (id & (1 << typeBit)) ? &dynamicEntities[i] : &staticEntities[i];
}

vector<Entity*> EntityManager::FindEntities(std::string tag) {
	vector<Entity*> taggedEntities;
	for (Entity &e : dynamicEntities) {
		if (tag == e.GetTag())
			taggedEntities.push_back(&e);
	}
	for (Entity &e : staticEntities) {
		if (tag == e.GetTag())
			taggedEntities.push_back(&e);
	}
	return taggedEntities;
}

void EntityManager::Initialize(int numContestants) {
	dynamicEntities.reserve(numContestants);
	staticEntities.reserve(128);
}

Entity& EntityManager::CreateEntity(std::vector<Entity> &entities, unsigned short i) {
	entities.push_back(Entity((entities.size() | (i << typeBit) | (1 << activeBit))));
	Entity& entity = entities.back();
	entity.transformID = transforms.size();
	transforms.push_back(Transform());
	return entity;
}

Entity& EntityManager::CreateDynamicEntity() {
	return CreateEntity(dynamicEntities, 1);
}

Entity& EntityManager::CreateStaticEntity() {
	return CreateEntity(staticEntities, 0);
}

Transform& EntityManager::GetTransform(short entityID) {
	return transforms[GetEntityIndex(entityID)];
}

void EntityManager::SetParent(Entity* child, Entity& parent) {
	if (child->parentID == parent.GetId()) return;
	if (child->parentID != SHRT_MAX) {
		Entity* previousParent = FindEntity(child->parentID);
		const auto it = std::find(previousParent->children.begin(), previousParent->children.end(), child->GetId());
		if (it != previousParent->children.end())
			previousParent->children.erase(it);
	}

	// Check if there is a new parent
	// Update this child's and its transform's parent pointers
	child->parentID = parent.GetId();
	GetTransform(child->GetId()).parent = &GetTransform(parent.GetId());
	
	// Add this child to the new parent's children vector
	parent.children.push_back(child->GetId());
}

void EntityManager::SetTag(Entity& entity, std::string tag) {
	if (entity.HasTag(tag)) return;

	// Set this entity's tag and add this entity to the list of entities with this tag
	entity.SetTag(tag);
}