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

	void HandleEvent(Event *event);

    void RenderDebugGui();

	size_t GetId() const;
	std::string GetTag() const;
	bool HasTag(std::string _tag) const;

	bool connectedToCylinder;

private:
	Entity(size_t _id);
	void AddComponent(Component *component);
	void RemoveComponent(Component *component);
	void SetTag(std::string _tag);

	size_t id;
	std::string tag;

    Entity *parent;
    std::vector<Entity*> children;
public:
	std::vector<Component*> components;
};
