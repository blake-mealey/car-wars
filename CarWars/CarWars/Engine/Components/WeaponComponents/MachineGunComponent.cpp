#include "MachineGunComponent.h"

MachineGunComponent::MachineGunComponent() {}

void MachineGunComponent::InternalShoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Bullet Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/machine_gun_shot.mp3");

		Entity* vehicle = &this->GetEntity();
		PxScene* scene = &Physics::Instance().GetScene();
		Entity* mgTurret = EntityManager::FindChildren(&this->GetEntity(), "GunTurret")[0];
		PxRaycastBuffer hit;
		if (scene->raycast(Transform::ToPx(mgTurret->GetTransform().GetGlobalPosition() - mgTurret->GetTransform().GetForward() * 5.0f), -Transform::ToPx(mgTurret->GetTransform().GetForward()), 400.0f, hit)) {
			if (hit.hasAnyHits()) {
				//Cube at Hit Location
				Entity* cube = EntityManager::CreateStaticEntity();
				EntityManager::AddComponent(*cube, MeshComponent{ "Cube.obj", "Basic.json" });
				cube->GetTransform().SetPosition(Transform::FromPx(hit.block.position));
				cube->GetTransform().SetScale(glm::vec3(0.1f, 0.1f, 0.1f));

				Entity* thingHit = EntityManager::FindEntity(hit.block.actor);
				std::vector<VehicleComponent>& comps = EntityManager::Components<VehicleComponent>();
				for (size_t i = 0; i < comps.size(); i++) {
					if (thingHit != NULL && (comps[i].GetEntity().GetId() == thingHit->GetId())) {
						std::cout << "Entered Here" << std::endl;
						comps[i].TakeDamage(damage);
					}
				}
			}
		}
	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void MachineGunComponent::InternalCharge() {
	Shoot();
}

void MachineGunComponent::InternalRenderDebugGui() {
    //WeaponComponent::RenderDebugGui();
}