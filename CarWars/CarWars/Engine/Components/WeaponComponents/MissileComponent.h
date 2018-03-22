#pragma once

#include "../Component.h"


#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MissileComponent : public Component {
public:
	MissileComponent();

	void Initialize(Entity* _entity, glm::vec3 positon);

	float GetSpeed();
	float GetDamage();
	float GetExplosionRadius();
	Entity* GetOwner();

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

private:
	float missileSpeed = 200.0f;
	float explosionRadius = 15.0f;
	float damage;
	Entity* owner;
};