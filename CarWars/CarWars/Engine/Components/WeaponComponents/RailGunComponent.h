#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"
#include "../../Systems/Physics.h"
#include "../../Entities/EntityManager.h"

#include "../RigidbodyComponents/VehicleComponent.h"

#include "WeaponComponent.h"

class RailGunComponent : public WeaponComponent<RailGunComponent> {
	friend class WeaponComponent<RailGunComponent>;
public:
	RailGunComponent();
	void InternalShoot();
	void InternalCharge();

	static constexpr ComponentType InternalGetType() { return ComponentType_RailGun; }

	void InternalRenderDebugGui();
private:
	Time timeBetweenShots = 1.0f;
	Time chargeTime = 2.0f;
	Time nextChargeTime = 0.0f;
	float damage = 1150.0f;
};