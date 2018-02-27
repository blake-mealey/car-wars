#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"

#include "WeaponComponent.h"

class RailGunComponent : public WeaponComponent {
public:
	RailGunComponent();
	void Shoot() override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;

	void RenderDebugGui() override;
private:
	Time timeBetweenShots = 1.0f;
	Time chargeTime = 2.0f;
	Time nextChargeTime = 0.0f;
	float damage = 1150.0f;
};