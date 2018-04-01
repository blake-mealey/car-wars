#include "BillboardComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

BillboardComponent::BillboardComponent(nlohmann::json data) {
    transform = Transform(data);

    const std::string texturePath = ContentManager::GetFromJson<std::string>(data["Texture"], "DiamondPlate.jpg");
    texture = ContentManager::GetTexture(texturePath);

    uvScale = ContentManager::JsonToVec2(data["UvScale"], glm::vec2(1.f));
}

ComponentType BillboardComponent::GetType() {
    return ComponentType_Billboard;
}

void BillboardComponent::HandleEvent(Event* event) { }

void BillboardComponent::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    transform.parent = &_entity->transform;
}

void BillboardComponent::RenderDebugGui() {
    Component::RenderDebugGui();
    if (ImGui::TreeNode("Transform")) {
        transform.RenderDebugGui();
        ImGui::TreePop();
    }
    ImGui::DragFloat2("UV Scale", glm::value_ptr(uvScale), 0.1f);
}

Texture* BillboardComponent::GetTexture() const {
    return texture;
}

glm::vec2 BillboardComponent::GetUvScale() const {
    return uvScale;
}


