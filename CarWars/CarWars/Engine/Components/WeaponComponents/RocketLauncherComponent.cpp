#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() {}

void RocketLauncherComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Rocket Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket_launcher_shot.mp3");


		Entity* missile = EntityManager::CreateDynamicEntity();
		MeshComponent* missileMesh = new MeshComponent("Missile.obj", "Basic.json", "Boulder.jpg");
		EntityManager::AddComponent(missile, missileMesh);

		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];
		missile->transform.SetPosition(EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetGlobalPosition());
		missile->transform.SetScale(glm::vec3(0.05, 0.05, 0.05));
		missile->transform.SetRotation(vehicle->transform.GetLocalRotation() * EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetLocalRotation());

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