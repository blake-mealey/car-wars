#include "MachineGunComponent.h"

MachineGunComponent::MachineGunComponent() {}

void MachineGunComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Bullet Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();
	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void MachineGunComponent::Charge() {
	Shoot();
}

ComponentType MachineGunComponent::GetType() {
	return ComponentType_MachineGun;
}

void MachineGunComponent::HandleEvent(Event *event) {
	return;
}

void MachineGunComponent::RenderDebugGui() {
    WeaponComponent::RenderDebugGui();
}