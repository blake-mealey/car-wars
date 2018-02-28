#include "Entity.h"
#include "../Events/Event.h"
#include "imgui/imgui.h"

#include "Entity.h"
Entity::Entity(short i) : id(i) {}

short Entity::GetId() const {
	return id;
}

unsigned short Entity::GetIndex() {
	return id & ~(infoMask << idEnd);
}
unsigned short Entity::GetComponentIndex(ComponentType t) const {
	return components[t];
}

void Entity::RenderDebugGui() {
    /*if (ImGui::TreeNode((void*)(intptr_t)GetId(), "Entity (%s)", GetTag().c_str())) {
        if (ImGui::TreeNode("Properties")) {
            if (ImGui::TreeNode("Transform")) {
                transform.RenderDebugGui();
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        if (!components.empty() && ImGui::TreeNode("Components")) {
            size_t i = 0;
            for (Component *component : components) {
                if (ImGui::TreeNode((void*)(intptr_t)i, "Component (%s)", Component::GetTypeName(component->GetType()))) {
                    component->RenderDebugGui();
                    ImGui::TreePop();
                }
                i++;
            }
            ImGui::TreePop();
        }

        if (!children.empty() && ImGui::TreeNode("Children")) {
            for (Entity *child : children) {
                child->RenderDebugGui();
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }*/
}


