#pragma once

#include "Entity.h"
#include <map>
#include <unordered_map>
#include <typeindex>

class CameraComponent;

class EntityManager {
public:
	// Access entities
    static Entity* GetRoot();
	static Entity* FindEntity(size_t id);
	static Entity* FindEntity(physx::PxRigidActor* _actor);
	static std::vector<Entity*> FindEntities(std::string tag);

	// Manage entities
	static Entity* CreateStaticEntity(Entity *parent=nullptr);
	static Entity* CreateDynamicEntity(Entity *parent = nullptr);
	static void DestroyStaticEntity(Entity *entity);
	static void DestroyDynamicEntity(Entity *entity);
    static void DestroyEntity(Entity *entity);
	static void DestroyChildren(Entity *entity);
    static size_t GetEntityCount();

    static void DestroyScene();

	static void SetTag(size_t entityId, std::string tag);
	static void SetTag(Entity *entity, std::string tag);
    static void ClearTag(Entity *entity);

    // Manage entity parenting
    static void SetParent(Entity* child, Entity *parent);
    static Entity* GetParent(Entity* entity);
    static std::vector<Entity*> FindChildren(Entity* entity, std::string tag, size_t maxCount);
    static std::vector<Entity*> FindChildren(Entity* entity, std::string tag);
    static Entity* FindFirstChild(Entity* entity, std::string tag);
    static std::vector<Entity*> GetChildren(Entity* entity);

	// Manage components
	static void AddComponent(size_t entityId, Component* component);
	static void AddComponent(Entity *entity, Component* component);
	static void DestroyComponent(Component* component);
	static std::vector<Component*> GetComponents(ComponentType type);
    static std::vector<Component*> GetComponents(std::vector<ComponentType> types);
    static size_t GetComponentCount(ComponentType type);
    static size_t GetComponentCount();
	
	template <class T>
	static std::vector<T*> GetComponents(ComponentType type) {
		std::vector<T*> result;
		std::vector<Component*> temp = components[type];
		for (Component* component : temp) {
			result.push_back(static_cast<T*>(component));
		}
		return result;
	}

	// Contact entities
	static void BroadcastEvent(Event *event);
private:
	static Entity* CreateEntity(std::vector<Entity*> &entities, Entity* parent);
	static void DestroyEntity(Entity *entity, std::vector<Entity*> &entities);

	// Store entities
    static Entity* root;
	static std::vector<Entity*> staticEntities;
	static std::vector<Entity*> dynamicEntities;
	static std::map<size_t, Entity*> idToEntity;
	static std::map<std::string, std::vector<Entity*>> tagToEntities;

	// Store components
	static std::map<ComponentType, std::vector<Component*>> components;
	static std::unordered_map<std::type_index, std::vector<Component*>> _components;

	static size_t nextEntityId;

	// TODO (if necessary): Object pools (under-the-hood, won't change interface of Create/Destroy)
};
