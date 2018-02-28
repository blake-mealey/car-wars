#pragma once

#include "ComponentType.h"
#include "../Events/Event.h"
#include "imgui/imgui.h"
#include <foundation/PxTransform.h>
#include<iostream>
#include "../Entities/EntityManager.h"

using std::cout;
using std::endl;

class Entity;

template<class T, EventType... events>
class Component {
private:

	//Recursive template helpers
	template <EventType Last>
	static constexpr unsigned short BitCalc() {
		return 1 << Last;
	}

	template<EventType ...Rest>
	static constexpr unsigned short BitCalc();

	template <EventType First, EventType Second, EventType ...Rest>
	static constexpr unsigned short BitCalc() {
		return BitCalc<First>() | BitCalc<Second, Rest...>();
	}

	template<>
	static constexpr unsigned short BitCalc<>() {
		return 0;
	}



	//Actual bitmask we will use
	static constexpr unsigned short listeners = BitCalc<events...>();

public:
	bool enabled;

	short entityID;

	static std::string GetTypeName(ComponentType type) {
		switch (type) {
		case ComponentType_Mesh: return "Mesh";
		case ComponentType_Camera: return "Camera";
		case ComponentType_PointLight: return "PointLight";
		case ComponentType_DirectionLight: return "DirectionLight";
		case ComponentType_SpotLight: return "SpotLight";
		case ComponentType_Rigidbody: return "Rigidbody";
		case ComponentType_RigidStatic: return "RigidStatic";
		case ComponentType_RigidDynamic: return "RigidDynamic";
		case ComponentType_Vehicle: return "Vehicle";
		case ComponentType_Weapons: return "Weapons";
		case ComponentType_MachineGun: return "MachineGun";
		default: return std::to_string(type);
		}
	};

	void RenderDebugGui() { ImGui::Checkbox("Enabled", &enabled); static_cast<T*>(this)->InternalRenderDebugGui(); }

	void UpdateFromPhysics(physx::PxTransform t) {	static_cast<T*>(this)->InternalUpdateFromPhysics(t); }

	void SetEntity(Entity& _entity) { entityID = _entity.GetId(); static_cast<T*>(this)->InternalSetEntity(_entity); }

	short GetEntity() { return entityID; }

	//Getters
	static constexpr ComponentType GetType() { return T::InternalGetType(); }
	//Ensures that derived classes must implement their own InternalGetType() method
	//template<class E>
	//void HandleEvent(E *e) { return static_cast<T>(this)->InternalHandleEvent(E *e); }
	static constexpr int GetListeners() { return listeners; }
protected:
	void InternalSetEntity(Entity& _entity) {};
	void InternalUpdateFromPhysics(physx::PxTransform t) {
		Transform& trans = EntityManager::GetTransform(GetEntity());
		trans.SetPosition(Transform::FromPx(t.p));
		trans.SetRotation(Transform::FromPx(t.q));
	}
};

/*class BodyComponent : public Component<BodyComponent, EventType_Accelerate, EventType_Collision> {
	friend class Component<BodyComponent, EventType_Accelerate, EventType_Collision>;
protected:
	static constexpr ComponentType InternalGetType() { return ComponentType_Body; }
public:
	void HandleEvent(CollisionEvent *e) {
		cout << "(BodyComponent) Hello World! I have arrived!" << endl;
	}
};*/
