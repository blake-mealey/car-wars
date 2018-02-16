#include "Component.h"

Component::Component() : entity(nullptr), enabled(true) { }

std::string Component::GetTypeName(ComponentType type) {
    switch(type) {
    case ComponentType_Mesh: return "Mesh";
    case ComponentType_Camera: return "Camera";
    case ComponentType_PointLight: return "PointLight";
    case ComponentType_DirectionLight: return "DirectionLight";
    case ComponentType_SpotLight: return "SpotLight";
    case ComponentType_Rigidbody: return "Rigidbody";
    case ComponentType_Vehicle: return "Vehicle";
    default: return std::to_string(type);
    }
}

void Component::SetEntity(Entity* _entity) {
	entity = _entity;
}

Entity* Component::GetEntity() const {
	return entity;
}
