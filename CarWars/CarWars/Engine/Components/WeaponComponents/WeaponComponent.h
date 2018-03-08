#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../Component.h"
#include "../Engine/Systems/Time.h"

template<class T, EventType... events>
class WeaponComponent : public Component<T, events...> {
	friend class Component<T, events...>;
	friend class MachineGunComponent;
	friend class RocketLauncherComponent;
	friend class RailGunComponent;
public:
	void Shoot() { static_cast<T*>(this)->InternalShoot(); }
	void Charge() { static_cast<T*>(this)->InternalCharge(); }

	void SetTargetRotation(float _horizontalAngle, float _verticalAngle) {
		targetHorizontalAngle = _horizontalAngle * -1.0f;
		targetVerticalAngle = _verticalAngle * -1.0f;

		horizontalAngle = glm::mix(horizontalAngle, targetHorizontalAngle, 0.05f);
		verticalAngle = glm::mix(verticalAngle, targetVerticalAngle, 0.05f);
	}

	float horizontalAngle;
	float verticalAngle;

	Time timeBetweenShots;

private:

	float targetHorizontalAngle;
	float targetVerticalAngle;

	Time nextShotTime = 0;

	float damage;
};