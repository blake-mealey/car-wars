#pragma once
#include <iostream>
#include <string>

#include <glm/gtx/string_cast.hpp>

#include "../../Systems/Audio.h"
#include "../../Systems/StateManager.h"
#include "../../Systems/Physics.h"
#include "../../Entities/EntityManager.h"

#include "../RigidbodyComponents/VehicleComponent.h"

#include "WeaponComponent.h"

class MachineGunComponent : public WeaponComponent<MachineGunComponent> {
	friend class WeaponComponent<MachineGunComponent>;
public:
	MachineGunComponent();
	void InternalShoot();
	void InternalCharge();

	static constexpr ComponentType InternalGetType() { return ComponentType_MachineGun; }

	void InternalRenderDebugGui();
private:
	Time timeBetweenShots = 0.05f;

	float damage = 20.0f;
};