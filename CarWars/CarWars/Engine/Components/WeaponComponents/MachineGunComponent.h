#pragma once

#include <glm/gtx/string_cast.hpp>

#include "WeaponComponent.h"

class MachineGunComponent : public WeaponComponent {
public:
	MachineGunComponent();
	void Shoot(glm::vec3 position) override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;
private:
	Time timeBetweenShots = 0.05f;
};