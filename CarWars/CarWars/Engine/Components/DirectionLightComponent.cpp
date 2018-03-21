#include "DirectionLightComponent.h"
#include "../Systems/Content/ContentManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"

DirectionLightComponent::DirectionLightComponent(nlohmann::json data) {
    color = ContentManager::GetColorFromJson(data["Color"], glm::vec4(1.f));
	direction = normalize(ContentManager::JsonToVec3(data["Direction"], glm::vec3(0.f, 0.f, 1.f)));
	castsShadows = ContentManager::GetFromJson<bool>(data["CastsShadows"], false);
}

DirectionLightComponent::DirectionLightComponent(glm::vec3 _color, glm::vec3 _direction)
	: color(_color), direction(normalize(_direction)) {}

glm::vec3 DirectionLightComponent::GetColor() const {
	return color;
}

glm::vec3 DirectionLightComponent::GetDirection() const {
	return direction;
}

DirectionLight DirectionLightComponent::GetData() const {
	return DirectionLight(color, direction);
}

bool DirectionLightComponent::IsShadowCaster() const {
	return castsShadows;
}

void DirectionLightComponent::SetShadowCaster(bool _castsShadows) {
	castsShadows = _castsShadows;
}

void DirectionLightComponent::SetDirection(glm::vec3 _direction) {
    direction = glm::normalize(_direction);
}

ComponentType DirectionLightComponent::GetType() {
	return ComponentType_DirectionLight;
}

void DirectionLightComponent::HandleEvent(Event* event) {}
void DirectionLightComponent::RenderDebugGui() {
    Component::RenderDebugGui();
    ImGui::Checkbox("Casts Shadows", &castsShadows);
    ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.01f);
    ImGui::ColorEdit3("Colour", glm::value_ptr(color));
}
