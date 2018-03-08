#pragma once

#include <string>
#include "Transform.h"
#include "../Components/Component.h"
#include <vector>

class EntityManager;
class Event;

class Entity {
friend class EntityManager;
public:
	~Entity();

	Transform transform;

	template <class T>
	T* GetComponent() {
		for (Component* component : components) {
			T* typedComponent = dynamic_cast<T*>(component);
			if (typedComponent) {
				return typedComponent;
			}
		}
		return nullptr;
	}

	template <class T>
	std::vector<T*> GetComponents() {
		std::vector<T*> found;
		for (Component* component : components) {
			T* typedComponent = dynamic_cast<T*>(component);
			if (typedComponent) {
				found.push_back(typedComponent);
			}
		}
		return found;
	}

	void HandleEvent(Event *event);

    void RenderDebugGui();

	size_t GetId() const;
	std::string GetTag() const;
	bool HasTag(std::string _tag) const;

	bool IsMarkedForDeletion() const;
	void MarkForDeletion();
private:
	bool markedForDeletion;

	Entity(size_t _id);
	void AddComponent(Component *component);
	void RemoveComponent(Component *component);
	void SetTag(std::string _tag);

	size_t id;
	std::string tag;

    Entity *parent;
    std::vector<Entity*> children;
	std::vector<Component*> components;
};
