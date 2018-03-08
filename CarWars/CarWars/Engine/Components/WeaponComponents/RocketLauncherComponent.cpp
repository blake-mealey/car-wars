#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() {}

void RocketLauncherComponent::InternalShoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Rocket Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket_launcher_shot.mp3");

		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];

		Entity* missile = EntityManager::CreateDynamicEntity();
		EntityManager::SetTag(missile, "Missile");
		MeshComponent missileMesh = MeshComponent("Missile.obj", "RedShiny.json");

		missile->GetTransform().SetPosition(EntityManager::FindChildren(vehicle, "GunTurret")[0]->GetTransform().GetGlobalPosition() - EntityManager::FindChildren(vehicle, "GunTurret")[0]->GetTransform().GetForward() * 5.0f);
		missile->GetTransform().SetScale(glm::vec3(0.05, 0.05, 0.05));
		missile->GetTransform().SetRotation(vehicle->GetTransform().GetLocalRotation() * EntityManager::FindChildren(vehicle, "GunTurret")[0]->GetTransform().GetLocalRotation());

		EntityManager::AddComponent(*missile, missileMesh);
		MissileComponent* missileComponent = new MissileComponent(vehicle, damage);
		EntityManager::AddComponent(*missile, *missileComponent);
		RigidDynamicComponent* missileRigidDynamic = new RigidDynamicComponent();
		EntityManager::AddComponent(*missile, *missileRigidDynamic);
		PxMaterial *material = ContentManager::GetPxMaterial("Default.json");
		BoxCollider* missileCollider = new BoxCollider("Missiles", material, PxFilterData(), glm::vec3(.1f, .1f, 1.f));
		missileRigidDynamic->AddCollider(missileCollider);
		missileRigidDynamic->actor->setLinearVelocity(Transform::ToPx(-missile->GetTransform().GetForward() * missileComponent->GetSpeed()), true);
		missileRigidDynamic->actor->setLinearDamping(0.0);
		missileRigidDynamic->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void RocketLauncherComponent::InternalCharge() {
	Shoot();
}

void RocketLauncherComponent::InternalRenderDebugGui() {
	//WeaponComponent::RenderDebugGui();
}