#pragma once

class Event;
class Entity;

enum ComponentType {
	ComponentType_Mesh,
	ComponentType_Camera,
	ComponentType_PointLight,
	ComponentType_DirectionLight,
	ComponentType_SpotLight,
	ComponentType_Rigidbody,
    ComponentType_Vehicle
};

class Component {
public:
	Component();
	
	bool enabled;
	
	virtual ComponentType GetType() = 0;
	virtual void HandleEvent(Event *event) = 0;

	virtual void SetEntity(Entity *_entity);
	Entity* GetEntity() const;
protected:
	Entity *entity;
};
