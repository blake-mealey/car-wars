#include "EntityManager.h"
#include "../Components/CameraComponent.h"

Entity* EntityManager::root = new Entity(0);
std::vector<Entity*> EntityManager::staticEntities;
std::vector<Entity*> EntityManager::dynamicEntities;
std::map<size_t, Entity*> EntityManager::idToEntity;
std::map<std::string, std::vector<Entity*>> EntityManager::tagToEntities;

std::map<ComponentType, std::vector<Component*>> EntityManager::components;

size_t EntityManager::nextEntityId = 1;

Entity* EntityManager::GetRoot() {
    return root;
}

Entity* EntityManager::FindEntity(size_t id) {
	return idToEntity[id];
}

std::vector<Entity*> EntityManager::FindEntities(std::string tag) {
	std::vector<Entity*> ret = tagToEntities[tag];
	for (size_t i = 0; i < ret.size(); i++) ret[i]->transform.Update();
	return ret;
}

/*Entity* EntityManager::CreateGuiEntity(CameraComponent *camera) {
	CreateEntity(camera->GetGuiEntities(), camera->GetGuiRoot());
}*/

Entity* EntityManager::CreateDynamicEntity() {
	return CreateEntity(dynamicEntities, root);
}

Entity* EntityManager::CreateStaticEntity() {
	return CreateEntity(staticEntities, root);
}

Entity* EntityManager::CreateEntity(std::vector<Entity*> &entities, Entity *parent) {
	size_t id = nextEntityId++;
	Entity* entity = new Entity(id);
	entities.push_back(entity);
	idToEntity[id] = entity;
    SetParent(entity, parent);
	return entity;
}

void EntityManager::DestroyDynamicEntity(size_t id) {
	DestroyEntity(id, dynamicEntities);
}

void EntityManager::SetTag(size_t entityId, std::string tag) {
	return SetTag(FindEntity(entityId), tag);
}

void EntityManager::SetTag(Entity* entity, std::string tag) {
    if (entity->HasTag(tag)) return;
	
    // Find the list of entities with this entity's tag
    auto it = tagToEntities.find(entity->GetTag());
	if (it != tagToEntities.end()) {
		std::vector<Entity*> &entities = it->second;
        // Remove this entity from that list
		auto it2 = std::find(entities.begin(), entities.end(), entity);
		if (it2 != entities.end())
			entities.erase(it2);
	}

    // Set this entity's tag and add this entity to the list of entities with this tag
	entity->SetTag(tag);
	tagToEntities[tag].push_back(entity);
}

void EntityManager::SetParent(Entity* child, Entity* parent) {
    // Double check we aren't changing it to the same parent
    Entity *previousParent = child->parent;
    if (previousParent == parent) return;

    // Check if there was a previous parent
    if (previousParent) {
        // Remove this child from its old parent's children vector
        const auto it = std::find(previousParent->children.begin(), previousParent->children.end(), child);
        if (it != previousParent->children.end())
            previousParent->children.erase(it);
    }

    // Check if there is a new parent
    if (parent) {
        // Update this child's and its transform's parent pointers
        child->parent = parent;
        child->transform.parent = &parent->transform;

        // Add this child to the new parent's children vector
        parent->children.push_back(child);
    }
}

Entity* EntityManager::GetParent(Entity* entity) {
    return entity->parent;
}

std::vector<Entity*> EntityManager::FindChildren(Entity* entity, std::string tag, size_t maxCount) {
    std::vector<Entity*> children;
    if (maxCount == 0) return children;
    
    for (Entity *child : entity->children) {
        if (child->HasTag(tag)) {
            children.push_back(child);
            if (maxCount == children.size()) break;
        }
    }
    
    return children;
}

std::vector<Entity*> EntityManager::FindChildren(Entity* entity, std::string tag) {
    return FindChildren(entity, tag, entity->children.size());
}

Entity* EntityManager::FindFirstChild(Entity* entity, std::string tag) {
    return FindChildren(entity, tag, 1)[0];
}

std::vector<Entity*> EntityManager::GetChildren(Entity* entity) {
    return entity->children;
}

void EntityManager::DestroyStaticEntity(size_t id) {
	DestroyEntity(id, staticEntities);
}

void EntityManager::DestroyEntity(size_t id, std::vector<Entity*> &entities) {
	Entity* entity = FindEntity(id);
    SetParent(entity, nullptr);
	if (entity != nullptr) {
		idToEntity.erase(id);
		auto it = std::find(entities.begin(), entities.end(), entity);
		entities.erase(it);
	}
	delete entity;
}

void EntityManager::AddComponent(size_t entityId, Component* component) {
	AddComponent(FindEntity(entityId), component);
}

void EntityManager::AddComponent(Entity* entity, Component* component) {
	entity->AddComponent(component);
	components[component->GetType()].push_back(component);
	component->SetEntity(entity);
}

void EntityManager::DestroyComponent(Component* component) {
	auto list = components[component->GetType()];
	auto it = std::find(list.begin(), list.end(), component);
	if (it != list.end())
		list.erase(it);
	component->GetEntity()->RemoveComponent(component);
	delete component;
}

std::vector<Component*> EntityManager::GetComponents(ComponentType type) {
	return components[type];
}

std::vector<Component*> EntityManager::GetComponents(std::vector<ComponentType> types) {
    std::vector<Component*> all;
    for (ComponentType type : types) {
        std::vector<Component*> components = GetComponents(type);
        all.insert(all.end(), components.begin(), components.end());
    }
    return all;
}

void EntityManager::BroadcastEvent(Event* event) {
	for (size_t i = 0; i < staticEntities.size(); i++) {
		staticEntities[i]->HandleEvent(event);
	}
	for (size_t i = 0; i < dynamicEntities.size(); i++) {
		dynamicEntities[i]->HandleEvent(event);
	}
}
