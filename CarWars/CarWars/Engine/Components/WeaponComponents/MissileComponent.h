#pragma once

#include "../Component.h"

class MissileComponent : public Component {
public:
	inline MissileComponent(Entity* _owner) : owner(_owner) {};

	ComponentType GetType() override {
		return ComponentType_Missile;
	}

	void HandleEvent(Event *event) override {
		return;
	}

	float GetSpeed() {
		return missileSpeed;
	}

private:
	float missileSpeed = 2000.0f;
	float explosionRadius = 2.0f;

	Entity* owner;
};