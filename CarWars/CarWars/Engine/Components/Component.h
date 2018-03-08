#pragma once

#include "ComponentType.h"
#include "../Events/Event.h"
#include "imgui/imgui.h"
#include <foundation/PxTransform.h>
#include<iostream>
#include "../Entities/EntityManager.h"

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
	bool enabled = true;

	short entityID = SHRT_MAX;

	void RenderDebugGui() { ImGui::Checkbox("Enabled", &enabled); static_cast<T*>(this)->InternalRenderDebugGui(); }

	void UpdateFromPhysics(physx::PxTransform t) { 
		EntityManager::GetEntityTransform(entityID).SetPosition(Transform::FromPx(t.p));
		EntityManager::GetEntityTransform(entityID).SetRotation(Transform::FromPx(t.q));
		static_cast<T*>(this)->InternalUpdateFromPhysics(t); 
	}

	void SetEntity(Entity& _entity) { entityID = _entity.GetId(); static_cast<T*>(this)->InternalSetEntity(_entity); }

	Entity& GetEntity() const { return EntityManager::GetEntity(entityID); }
	Transform& GetEntityTransform() const { return EntityManager::GetEntityTransform(entityID); }

	//Getters
	static constexpr ComponentType GetType() { return T::InternalGetType(); }
	//Ensures that derived classes must implement their own InternalGetType() method
	//template<class E>
	//void HandleEvent(E *e) { return static_cast<T>(this)->InternalHandleEvent(E *e); }
	static constexpr int GetListeners() { return listeners; }

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
		case ComponentType_RailGun: return "RailGun";
		case ComponentType_RocketLauncher: return "RocketLauncher";
		case ComponentType_AI: return "AI";
		case ComponentType_GUI: return "GUI";
		default: return std::to_string(type);
		}
	}


protected:
	void InternalSetEntity(Entity& _entity) {};
	void InternalUpdateFromPhysics(physx::PxTransform t) {
		//Transform& trans = EntityManager::GetEntityTransform(GetEntity());
		//trans.SetPosition(Transform::FromPx(t.p));
		//trans.SetRotation(Transform::FromPx(t.q));
	}
};
