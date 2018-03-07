#include "Entity.h"
#include "../Events/Event.h"
#include "imgui/imgui.h"
#include "EntityManager.h"

Entity::Entity(size_t _id) : id(_id), components(std::vector<Component*>()), children(std::vector<Entity*>()),
        parent(nullptr), transform(Transform()), tag(std::string()), markedForDeletion(false) { }

Entity::~Entity() {
    while (!components.empty()) {
        EntityManager::DestroyComponent(components.back());
    }

    while (!children.empty()) {
        EntityManager::DestroyEntity(children.back());
    }
}

void Entity::HandleEvent(Event* event) {
	for (size_t i = 0; i < components.size(); i++) {
		components[i]->HandleEvent(event);		// TODO: Not really correct, use Colton's code
	}
}

void Entity::RenderDebugGui() {
    if (ImGui::TreeNode((void*)(intptr_t)GetId(), "Entity %d (%s)", GetId(), GetTag().c_str())) {
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
    }
}

void Entity::AddComponent(Component* component) {
	components.push_back(component);
}

void Entity::RemoveComponent(Component* component) {
	auto it = std::find(components.begin(), components.end(), component);
	if (it != components.end())
		components.erase(it);
}

size_t Entity::GetId() const {
	return id;
}

std::string Entity::GetTag() const {
	return tag;
}

bool Entity::HasTag(std::string _tag) const {
	return tag.compare(_tag) == 0;
}

void Entity::SetTag(std::string _tag) {
	tag = _tag;
}

bool Entity::IsMarkedForDeletion() const {
	return markedForDeletion;
}

void Entity::MarkForDeletion() {
	markedForDeletion = true;
}
