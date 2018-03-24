#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"
#include "../../Systems/Physics.h"
#include "../../Entities/EntityManager.h"

#include "../RigidbodyComponents/VehicleComponent.h"

#include "WeaponComponent.h"

class RailGunComponent : public WeaponComponent {
public:
	RailGunComponent();
	void Shoot(glm::vec3 position) override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;

	void RenderDebugGui() override;

	void ChargeRelease();

private:
	Time timeBetweenShots = 1.0f;
	Time chargeTime = 2.0f;
	Time nextChargeTime = 0.0f;
    bool playingChargeSound = false;
    int soundIndex;
};