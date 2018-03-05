#pragma once

#include "../Component.h"

class MissileComponent : public Component {
public:
	inline MissileComponent(Entity* _owner, float _damage) : owner(_owner), damage(_damage) {};

	ComponentType GetType() override {
		return ComponentType_Missile;
	}

	void HandleEvent(Event *event) override {
		return;
	}

	float GetSpeed() {
		return missileSpeed;
	}

	Entity* GetOwner() {
		return owner;
	}

	float GetDamage() {
		return damage;
	}

private:
	float missileSpeed = 2000.0f;
	float explosionRadius = 2.0f;
	float damage;
	Entity* owner;
};