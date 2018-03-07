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

class MachineGunComponent : public WeaponComponent {
public:
	MachineGunComponent();
	void Shoot() override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;
private:
	Time timeBetweenShots = 0.05f;

	float damage = 20.0f;
};