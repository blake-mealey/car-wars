#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() {}

void RocketLauncherComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Rocket Shot, Dealt : " << damage << std::endl;
		
		Entity* missile = EntityManager::CreateDynamicEntity();
		MeshComponent* missileMesh = new MeshComponent("Missile.obj", "Basic.json", "Boulder.jpg");
		EntityManager::AddComponent(missile, missileMesh);

		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];
		missile->transform.SetPosition(EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetGlobalPosition());
		missile->transform.SetScale(glm::vec3(0.05, 0.05, 0.05));
		missile->transform.SetRotation(EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetLocalRotation());

		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void RocketLauncherComponent::Charge() {
	Shoot();
}

ComponentType RocketLauncherComponent::GetType() {
	return ComponentType_RocketLauncher;
}

void RocketLauncherComponent::HandleEvent(Event *event) {
	return;
}

void RocketLauncherComponent::RenderDebugGui() {
	WeaponComponent::RenderDebugGui();
}