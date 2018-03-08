#include "Entity.h"
#include "../Events/Event.h"
#include "imgui/imgui.h"
#include "EntityManager.h"

Entity::Entity(short _id) : id(_id), components(std::unordered_map<std::type_index, vector<unsigned short>>()), children(std::vector<short>()),
        parentID(SHRT_MAX), transformID(SHRT_MAX), tag(std::string()) { }

Entity::~Entity() {
	/*while (!components.empty()) {
		//EntityManager::DestroyComponent(components.back());
	}

	while (!children.empty()) {
		EntityManager::DestroyEntity(children.back());
	}*/
}

short Entity::GetId() const {
	return id;
}

unsigned short Entity::GetIndex() {
	return id & ~(infoMask << idEnd);
}

unsigned short Entity::GetIndex(short entityID) {
	return entityID & ~(infoMask << idEnd);
}

bool Entity::isDynamic() {
	return id & (1 << typeBit);
}

bool Entity::isDynamic(short entityID) {
	return entityID & (1 << typeBit);
}

Transform& Entity::GetTransform() {
	return EntityManager::GetTransform(transformID);
}

vector<short>& Entity::GetChildren() {
	return children;
}

/*void Entity::RenderDebugGui() {
	if (ImGui::TreeNode((void*)(intptr_t)GetId(), "Entity %d (%s)", GetId(), GetTag().c_str())) {
		if (ImGui::TreeNode("Properties")) {
			if (ImGui::TreeNode("Transform")) {
				GetTransform().RenderDebugGui();
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
			for (unsigned short childID : children) {
				Entity child = EntityManager::GetEntity(childID);
				child.RenderDebugGui();
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}*/
