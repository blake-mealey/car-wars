#pragma once

#include "Entity.h"
#include <map>

class EntityManager {
public:
	// Access entities
	static Entity* FindEntity(size_t id);
	static std::vector<Entity*> FindEntities(std::string tag);

	// Manage entities
	static Entity* CreateStaticEntity();
	static Entity* CreateDynamicEntity();
	static void DestroyStaticEntity(size_t id);
	static void DestroyDynamicEntity(size_t id);

	static void SetTag(size_t entityId, std::string tag);
	static void SetTag(Entity *entity, std::string tag);

	// Manage components
	static void AddComponent(size_t entityId, Component* component);
	static void AddComponent(Entity *entity, Component* component);
	static void DestroyComponent(Component* component);
	static std::vector<Component*> GetComponents(ComponentType type);
	
	// Contact entities
	static void BroadcastEvent(Event *event);
private:
	static Entity* CreateEntity(std::vector<Entity*> &entities);
	static void DestroyEntity(size_t id, std::vector<Entity*> &entities);

	// Store entities
	static std::vector<Entity*> staticEntities;
	static std::vector<Entity*> dynamicEntities;
	static std::map<size_t, Entity*> idToEntity;
	static std::map<std::string, std::vector<Entity*>> tagToEntities;

	// Store components
	static std::map<ComponentType, std::vector<Component*>> components;

	static size_t nextEntityId;

	// TODO (if necessary): Object pools (under-the-hood, won't change interface of Create/Destroy)
};
