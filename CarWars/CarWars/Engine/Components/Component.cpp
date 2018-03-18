#include "Component.h"
#include "../Entities/Entity.h"
#include "imgui/imgui.h"

Component::Component() : entity(nullptr), enabled(true) { }

std::string Component::GetTypeName(ComponentType type) {
    switch(type) {
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
        case ComponentType_SpeedPowerUp: return "SpeedPowerUp";
        case ComponentType_DefencePowerUp: return "DefencePowerUp";
        case ComponentType_DamagePowerUp: return "DamagePowerUp";
		case ComponentType_GUI: return "GUI";
        default: return std::to_string(type);
    }
}

void Component::RenderDebugGui() {
    ImGui::Checkbox("Enabled", &enabled);
}

void Component::UpdateFromPhysics(physx::PxTransform t) {
    GetEntity()->transform.SetPosition(Transform::FromPx(t.p));
    GetEntity()->transform.SetRotation(Transform::FromPx(t.q));
}

void Component::TakeDamage(WeaponComponent* damager) { }

void Component::SetEntity(Entity* _entity) {
	entity = _entity;
}

Entity* Component::GetEntity() const {
	return entity;
}

