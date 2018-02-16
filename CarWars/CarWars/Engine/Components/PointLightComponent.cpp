#include "PointLightComponent.h"
#include "../Systems/Content/ContentManager.h"

PointLightComponent::PointLightComponent(nlohmann::json data) {
	color = ContentManager::JsonToVec3(data["Color"], glm::vec3(1.f));
	power = ContentManager::GetFromJson<float>(data["Power"], 10);
}

PointLightComponent::PointLightComponent(glm::vec3 _color, float _power) : color(_color), power(_power) {}

glm::vec3 PointLightComponent::GetColor() const {
	return color;
}

float PointLightComponent::GetPower() const {
	return power;
}

PointLight PointLightComponent::GetData() const {
	return PointLight(color, power, GetEntity()->transform.GetGlobalPosition());
}

ComponentType PointLightComponent::GetType() {
	return ComponentType_PointLight;
}

void PointLightComponent::HandleEvent(Event* event) {}
