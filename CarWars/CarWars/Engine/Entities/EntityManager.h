#pragma once
#include <vector>
#include <tuple>
#include <PxRigidActor.h>

using std::vector;

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
class AiComponent;
class GuiComponent;
class MissileComponent;

//Used as a workaround to prevent unecessary defines
struct Empty {

};

//Update when new components are added
#define COMPONENTS X(MeshComponent) X(CameraComponent) X(PointLightComponent) X(DirectionLightComponent) X(SpotLightComponent) X(RigidStaticComponent) X(RigidDynamicComponent) \
X(VehicleComponent) X(MachineGunComponent) X(RailGunComponent) X(RocketLauncherComponent) X(AiComponent) X(GuiComponent) X(MissileComponent)
#define X(ARG) std::vector<ARG>,
using ComponentTuple = std::tuple<COMPONENTS Empty>;
#undef X

class EntityManager {
private:
	//Attributes
	// Store entities
	static vector<Transform> transforms;
	//Singleton vectors to gaurantee ordered instantiation
	static vector<Entity>& getDynamicEntities();
	static vector<Entity>& getStaticEntities();
	static unsigned short root;

	//Store Components
	static ComponentTuple components;

	//Functions
	static Entity* CreateEntity(std::vector<Entity>& entities, Entity* parent, unsigned short i);
	static void CreateRoot();

	//Helper Functions
	static unsigned short GetEntityIndex(short id);

	// TODO (if necessary): Object pools (under-the-hood, won't change interface of Create/Destroy)
public:
	// Access entities
    static Entity* GetRoot();
	static Entity* FindEntity(short id);
	static Entity* FindEntity(physx::PxRigidActor* _actor);
	static Entity& GetEntity(short id);
	static std::vector<Entity*> FindEntities(std::string tag);

	//Get Transform
	static Transform& GetEntityTransform(short entityID);
	static Transform& GetTransform(unsigned short transformID);
	static unsigned short AddTransform(Transform& trans);
	static void SetTransform(unsigned short index, Transform& transform);

	// Manage entities
	static Entity* CreateStaticEntity(Entity *parent=nullptr);
	static Entity* CreateDynamicEntity(Entity *parent = nullptr);

	static void SetTag(short id, std::string tag);
	static void SetTag(Entity *entity, std::string tag);
    static void ClearTag(Entity *entity);

    // Manage entity parenting
    static void SetParent(Entity* child, Entity *parent);
    static Entity* GetParent(Entity* entity);
	static std::vector<Entity*> FindChildren(Entity* entity, std::string tag, size_t maxCount);
	static std::vector<Entity*> FindChildren(Entity* entity, std::string tag);

	// Manage components
	//Get Component Type Specific Vector
	template<class T>
	static auto& Components();
	template<class T>
	static void AddComponent(Entity&, T&&);
	template<class T>
	static auto& GetComponentFromEntity(const Entity* entity);

	//Render Info
	static void EntityRenderDebug(Entity& e);

	static constexpr size_t ComponentTupleSize() { return std::tuple_size<ComponentTuple>::value; }

	//Destruction
	static void DestroyEntity(short entityID);
	static void DestroyDynamicEntity(short entityID);
	static void DestroyStaticEntity(short entityID);
	template<class T>
	static void DestroyComponent(unsigned short comID);
	static void DestroyScene();
	static void DestroyEntities();
	static void DestroyComponents();
	static void DestroyTransforms();
};

template<class T>
auto& EntityManager::Components()
{
	return std::get<std::vector<T>>(components);
}

template<class T>
auto& EntityManager::GetComponentFromEntity(const Entity* entity)
{
	return Components<T>()[entity->GetComponentIndex<T>()];
}

template<class T>
void EntityManager::AddComponent(Entity& e, T&& c) {
	auto& comp = Components<std::decay_t<T>>();
	c.SetEntity(e);
	comp.push_back(c);
	e.AddComponent<std::decay_t<T>>(comp.size() - 1);
}

template<class T>
void EntityManager::DestroyComponent(unsigned short compID) {

}