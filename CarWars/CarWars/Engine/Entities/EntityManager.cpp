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
#include "../Components/WeaponComponents/MissileComponent.h"
#include "../Components/AiComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Systems/Graphics.h"
//#include <PxRigidActor.h>
#include "imgui/imgui.h"

class physx::PxRigidActor;

unsigned short EntityManager::root = 0;
vector<Transform> EntityManager::transforms;
ComponentTuple EntityManager::components;

vector<Entity>& EntityManager::getDynamicEntities() {
	static vector<Entity> entities;
	return entities;
}

vector<Entity>& EntityManager::getStaticEntities() {
	static vector<Entity> entities;
	return entities;
}

Entity* EntityManager::GetRoot() {
    return EntityManager::FindEntity(root);
}

unsigned short EntityManager::GetEntityIndex(short id) {
	return id & ~(infoMask << idEnd);
}

Entity* EntityManager::FindEntity(short id) {
	unsigned short i = id & ~(infoMask << idEnd);
	return (id & (1 << typeBit)) ? &getDynamicEntities()[i] : &getStaticEntities()[i];
}

Entity* EntityManager::FindEntity(physx::PxRigidActor* _actor) {
	for (size_t i = 0; i < getDynamicEntities().size(); i++) {
		VehicleComponent* vehicle = getDynamicEntities()[i].GetComponent<VehicleComponent>();
		if (vehicle != nullptr) {
			if (vehicle->pxRigid == _actor) {
				return &getDynamicEntities()[i];
			}
		}
	}
	return nullptr;
}

Entity& EntityManager::GetEntity(short id) {
	unsigned short i = id & ~(infoMask << idEnd);
	return (id & (1 << typeBit)) ? getDynamicEntities()[i] : getStaticEntities()[i];
}

//Might have to update all transforms before returning
vector<Entity*> EntityManager::FindEntities(std::string tag) {
	vector<Entity*> taggedEntities;
	for (Entity &e : getDynamicEntities()) {
		if (tag == e.GetTag())
			taggedEntities.push_back(&e);
	}
	for (Entity &e : getStaticEntities()) {
		if (tag == e.GetTag())
			taggedEntities.push_back(&e);
	}
	return taggedEntities;
}

void EntityManager::CreateRoot() {
	getStaticEntities().push_back(Entity((getStaticEntities().size() | (0 << typeBit) | (1 << activeBit))));
	getStaticEntities()[0].transformID = 0;
	transforms.push_back(Transform());
}

Entity* EntityManager::CreateEntity(std::vector<Entity> &entities, Entity *parent, unsigned short i) {
	short parentID = parent->GetId();
	entities.push_back(Entity((entities.size() | (i << typeBit) | (1 << activeBit))));
	Entity* entity;
	entity = &entities.back();
	if(parentID != SHRT_MAX)
		parent = &GetEntity(parentID);
	entity->transformID = transforms.size();
	transforms.push_back(Transform());
	SetParent(entity, parent);
	return entity;
}

Entity* EntityManager::CreateDynamicEntity(Entity *parent) {
    if (!parent) parent = GetRoot();
	return CreateEntity(getDynamicEntities(), parent, 1);
}

Entity* EntityManager::CreateStaticEntity(Entity *parent) {
    if (!parent) parent = GetRoot();
	return CreateEntity(getStaticEntities(), parent, 0);
}

void EntityManager::SetTag(short entityId, std::string tag) {
	return SetTag(FindEntity(entityId), tag);
}

void EntityManager::SetTag(Entity* entity, std::string tag) {
    if (entity->HasTag(tag)) return;
	
    // Clear the current tag from the entity
    ClearTag(entity);

    // Set this entity's tag and add this entity to the list of entities with this tag
	entity->SetTag(tag);
}

void EntityManager::ClearTag(Entity* entity) {
    // Find the list of entities with this entity's tag
    entity->SetTag("");
}

void EntityManager::SetParent(Entity* child, Entity* parent) {
    // Double check we aren't changing it to the same parent
    //if (child->parentID == parent->GetId()) return;

    // Check if there was a previous parent
    if (child->parentID != SHRT_MAX) {
		Entity* previousParent = FindEntity(child->parentID);
        // Remove this child from its old parent's children vector
        const auto it = std::find(previousParent->children.begin(), previousParent->children.end(), child->GetId());
        if (it != previousParent->children.end())
            previousParent->children.erase(it);
    }

    // Check if there is a new parent
    if (parent) {
        // Update this child's and its transform's parent pointers
        child->parentID = parent->GetId();
        GetTransform(child->transformID).parentID = parent->transformID;

        // Add this child to the new parent's children vector
        parent->children.push_back(child->GetId());
    }
}

Entity* EntityManager::GetParent(Entity* entity) {
    return FindEntity(entity->parentID);
}

Transform& EntityManager::GetEntityTransform(short entityID) {
	return transforms[GetEntityIndex(entityID)];
}

Transform& EntityManager::GetTransform(unsigned short transformID) {
	return transforms[transformID];
}

unsigned short EntityManager::AddTransform(Transform& trans) {
	transforms.push_back(trans);
	return transforms.size() - 1;
}

void EntityManager::SetTransform(unsigned short index, Transform& transform) {
	transforms[index] = transform;
}

void EntityManager::EntityRenderDebug(Entity& e) {
	if (ImGui::TreeNode((void*)(intptr_t)e.GetId(), "Entity %d (%s)", e.GetId(), e.GetTag().c_str())) {
		if (ImGui::TreeNode("Properties")) {
			if (ImGui::TreeNode("Transform")) {
				GetTransform(e.transformID).RenderDebugGui();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		/*if (!e.components.empty() && ImGui::TreeNode("Components")) {
			//std::unordered_map<std::type_index, vector<unsigned short>>::iterator
			for (auto itr = e.components.begin(); itr < e.components.end(); itr++) {
				for(itr->second)
				if (ImGui::TreeNode((void*)(intptr_t)i, "Component (%s)", Component::GetTypeName(component->GetType()))) {
					component->RenderDebugGui();
					ImGui::TreePop();
				}
				i++;
			}
			ImGui::TreePop();
		}*/
		size_t i = 0;
#define X(ARG) \
		{ \
			const vector<unsigned short>* ids = e.GetComponentIDs<ARG>(); \
			if (ids != nullptr && ImGui::TreeNode("Components")) { \
					for (unsigned short i = 0; i < ids->size(); i++) { \
						ARG& comp = EntityManager::Components<ARG>()[(*ids)[i]]; \
						if (ImGui::TreeNode((void*)(intptr_t)i, "Component (%s)", CameraComponent::GetTypeName(comp.GetType()))) { \
								comp.RenderDebugGui(); \
								ImGui::TreePop(); \
						} \
						i++; \
					} \
				ImGui::TreePop(); \
			} \
		}
		COMPONENTS
#undef X
/*#define X(ARG) \
		{ \
			vector<ARG*>* comps = e.GetComponents<ARG>(); \
			if (comps != nullptr && ImGui::TreeNode("Components")) { \
					for (ARG* comp : *comps) { \
							if (ImGui::TreeNode((void*)(intptr_t)i, "Component (%s)", CameraComponent::GetTypeName(comp->GetType()))) { \
									comp->RenderDebugGui(); \
									ImGui::TreePop(); \
							} \
								i++; \
					} \
				ImGui::TreePop(); \
			} \
		}
		COMPONENTS
#undef X*/

		if (!e.GetChildren().empty() && ImGui::TreeNode("Children")) {
			for (short childID : e.children) {
				EntityRenderDebug(GetEntity(childID));
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}

std::vector<Entity*> EntityManager::FindChildren(Entity* entity, std::string tag) {
	std::vector<Entity*> children;

	for (short childID : entity->children) {
		Entity* child = &EntityManager::GetEntity(childID);
		if (child->HasTag(tag)) {
			children.push_back(child);
		}
	}

	return children;
}

//Need to work on destruction
void EntityManager::DestroyEntity(short entityID) {
	Entity::isDynamic(entityID) ? DestroyDynamicEntity(entityID) : DestroyStaticEntity(entityID);
}

void EntityManager::DestroyDynamicEntity(short entityID) {

}

void EntityManager::DestroyStaticEntity(short entityID) {

}

void EntityManager::DestroyEntities() {
	getDynamicEntities().clear();
	getStaticEntities().clear();
	//root->children.clear();
}

void EntityManager::DestroyComponents() {
#define X(ARG) Components<ARG>().clear();
	COMPONENTS
#undef X
}

void EntityManager::DestroyTransforms() {
	transforms.clear();
}


void EntityManager::DestroyScene() { 
	DestroyEntities();
	DestroyComponents();
	DestroyTransforms();
	CreateRoot(); //This is for the root
}