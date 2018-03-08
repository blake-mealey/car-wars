#pragma once

#include "../Component.h"

class MissileComponent : public Component<MissileComponent> {
	friend class Component<MissileComponent>;
public:
	inline MissileComponent(Entity* _owner, float _damage) : owner(_owner), damage(_damage) {};

	static constexpr ComponentType InternalGetType() { return ComponentType_Missile; }

	void InternalRenderDebugGui();

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