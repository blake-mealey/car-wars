#include "MachineGunComponent.h"

MachineGunComponent::MachineGunComponent() {}

void MachineGunComponent::InternalShoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Bullet Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();
	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void MachineGunComponent::InternalRenderDebugGui() {
}