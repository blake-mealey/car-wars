#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"

#include "WeaponComponent.h"

class MachineGunComponent : public WeaponComponent<MachineGunComponent> {
	friend class WeaponComponent<MachineGunComponent>;
public:
	MachineGunComponent();
	void InternalShoot();

	static constexpr ComponentType InternalGetType() { return ComponentType_MachineGun; }

	void InternalRenderDebugGui();
private:
	Time timeBetweenShots = 0.05f;

	float damage = 20.0f;
};