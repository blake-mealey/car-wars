#pragma once
#include <vector>
#include <tuple>

class Entity;
class Transform;

class MeshComponent;
class CameraComponent;
class PointLightComponent;
class DirectionLightComponent;
class SpotLightComponent;
class RigidStaticComponent;
class RigidDynamicComponent;
class VehicleComponent;
class MachineGunComponent;
class RailGunComponent;
class RocketLauncherComponent;

/*#define typeBit 15
#define activeBit 14
#define idEnd 14
#define infoMask 11*/

///As a quick note the entity manager will reserve the first #contestants indices of applicable vectors for the contestants in play, the first #numplayers of which will be human controlled

using std::vector;

enum EntityType {
	EntityType_Static,
	EntityType_Dynamic
};
struct Empty {

};

///////////////////This is the only part we have to update when new Components are Added////////////////////////////////////////

#define COMPONENTS X(MeshComponent) X(CameraComponent) X(PointLightComponent) X(DirectionLightComponent) X(SpotLightComponent) X(RigidStaticComponent) X(RigidDynamicComponent) \
X(VehicleComponent) X(MachineGunComponent) X(RailGunComponent)// X(RocketLauncherComponent)
#define X(ARG) std::vector<ARG>,
using ComponentTuple = std::tuple<COMPONENTS Empty>;
#undef X
//#undef COMPONENTS

class EntityManager {
	//Inner Workings
private:
	static Entity* root;
	static ComponentTuple components;
	//static size_t nextEntityId;
	static int staticCount;
	static Entity& CreateEntity(vector<Entity> &entities, unsigned short i);
public:
	static Entity* GetRoot() { return root; }

	//Variables
	static vector<Entity> dynamicEntities;
	static vector<Entity> staticEntities;
	static vector<Transform> transforms;

	//These are returning references to the original
	static Entity& CreateDynamicEntity();
	static Entity& CreateStaticEntity();

	//TODO We will decide whether or not we need deletes

	//Initializing
	static void Initialize(int);

	//Find Entities
	static Entity* FindEntity(short id);
	static vector<Entity*> FindEntities(std::string tag);
	static unsigned short GetEntityIndex(short id);

	//Setting tags
	static void SetTag(Entity& entity, std::string tag);

	//Get Component Type Specific Vector
	template<class T>
	static auto& Components();

	//Get Transform
	//static Transform* GetTransform(unsigned short transformID);
	static Transform& GetTransform(short entityID);


	template<class T>
	static void AddComponent(Entity&, T&&);

	static constexpr size_t ComponentTupleSize() { return std::tuple_size<ComponentTuple>::value; }



	static void SetParent(Entity* child, Entity& parent);
	static Entity* GetParent(short child);
	static std::vector<Entity*> FindChildren(short parent, std::string tag, size_t maxCount);
	static std::vector<Entity*> FindChildren(short parent, std::string tag);
	static Entity* FindFirstChild(short parent, std::string tag);
	static std::vector<Entity*> GetChildren(short parent);




	//Map functions
//public:

	// Manage entities

	// Manage entity parenting
	/*static void SetParent(Entity* child, Entity *parent);
	static Entity* GetParent(Entity* entity);
	static std::vector<Entity*> FindChildren(Entity* entity, std::string tag, size_t maxCount);
	static std::vector<Entity*> FindChildren(Entity* entity, std::string tag);
	static Entity* FindFirstChild(Entity* entity, std::string tag);
	static std::vector<Entity*> GetChildren(Entity* entity);*/

	// Manage components
	/*static void AddComponent(size_t entityId, Component* component);
	static void AddComponent(Entity *entity, Component* component);
	static void DestroyComponent(Component* component);
	static std::vector<Component*> GetComponents(ComponentType type);
	static std::vector<Component*> GetComponents(std::vector<ComponentType> types);*/
};

template<class T>
auto& EntityManager::Components()
{
	return std::get<std::vector<T>>(components);
}

template<class T>
void EntityManager::AddComponent(Entity& e, T&& c) {
	auto& comp = Components<std::decay_t<T>>();
	c.SetEntity(e);
	comp.push_back(c);
	e.SetComponent(c.GetType(), comp.size() - 1);
}
