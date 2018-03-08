#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() {}

void RocketLauncherComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Rocket Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket_launcher_shot.mp3");

		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];

		Entity* missile = EntityManager::CreateDynamicEntity();
		EntityManager::SetTag(missile, "Missile");
		MeshComponent* missileMesh = new MeshComponent("Missile.obj", "RedShiny.json");

		missile->transform.SetPosition(EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetGlobalPosition() - EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetForward() * 5.0f);
		missile->transform.SetScale(glm::vec3(0.05, 0.05, 0.05));
		missile->transform.SetRotation(vehicle->transform.GetLocalRotation() * EntityManager::FindChildren(vehicle, "GunTurret")[0]->transform.GetLocalRotation());

		EntityManager::AddComponent(missile, missileMesh);
		MissileComponent* missileComponent = new MissileComponent(vehicle, damage);
		EntityManager::AddComponent(missile, missileComponent);
		RigidDynamicComponent* missileRigidDynamic = new RigidDynamicComponent();
		EntityManager::AddComponent(missile, missileRigidDynamic);
		PxMaterial *material = ContentManager::GetPxMaterial("Default.json");
		BoxCollider* missileCollider = new BoxCollider("Missiles", material, PxFilterData(), glm::vec3(.1f, .1f, 1.f));
		missileRigidDynamic->AddCollider(missileCollider);
		missileRigidDynamic->actor->setLinearVelocity(Transform::ToPx(-missile->transform.GetForward() * missileComponent->GetSpeed()), true);
		missileRigidDynamic->actor->setLinearDamping(0.0);
		missileRigidDynamic->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

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