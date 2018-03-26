#pragma once

#include "WeaponComponent.h"

class SuicideWeaponComponent : public WeaponComponent {
public:
	SuicideWeaponComponent();
	void Shoot(glm::vec3 position) override;
	void Charge() override;

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;
private:
};