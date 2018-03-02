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
	virtual void Shoot() = 0;
	virtual void Charge() = 0;

	void SetTargetRotation(float _horizontalAngle, float _verticalAngle);

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;

	float horizontalAngle;
	float verticalAngle;

    float damageMutliplier = 1.f;
private:

	float targetHorizontalAngle;
	float targetVerticalAngle;

	Time nextShotTime = 0;
	Time timeBetweenShots;

	float damage;
};