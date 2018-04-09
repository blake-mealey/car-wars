#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../Component.h"
#include "../Engine/Systems/Time.h"

class WeaponComponent : public Component {
	friend class MachineGunComponent;
	friend class RocketLauncherComponent;
	friend class RailGunComponent;
public:
	WeaponComponent(float _damage);

    void TweenChargeIndicator();

	virtual void Shoot(glm::vec3 position) = 0;
	virtual void Charge() = 0;

	virtual float GetDamage();

	void SetTargetRotation(float _horizontalAngle, float _verticalAngle);

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;

	float horizontalAngle;
	float verticalAngle;

    float damageMultiplier = 1.f;
	Time timeBetweenShots;
	void turnTurret(glm::vec3 location);
protected:

	float targetHorizontalAngle;
	float targetVerticalAngle;

	Time nextShotTime = 0;

	float damage;
};