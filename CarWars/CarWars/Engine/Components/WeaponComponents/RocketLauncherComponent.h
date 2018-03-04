#pragma once
#include <iostream>

#include "../../Systems/Audio.h"
#include "../../Systems/StateManager.h"
#include "../../Entities/EntityManager.h"

#include "../MeshComponent.h"

#include "WeaponComponent.h"

class RocketLauncherComponent : public WeaponComponent {
public:
	RocketLauncherComponent();
	void Shoot() override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;
private:
	Time timeBetweenShots = 1.0f / 0.75f;

	float damage = 500.0f;
};