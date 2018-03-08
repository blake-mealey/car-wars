#include "RailGunComponent.h"

RailGunComponent::RailGunComponent() {}

void RailGunComponent::InternalShoot() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextShotTime.GetTimeSeconds()) {
		nextChargeTime = StateManager::gameTime + timeBetweenShots;
		std::cout << "Shoot Rail Gun" << std::endl;
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);

		Entity* vehicle = &this->GetEntity();
		PxScene* scene = &Physics::Instance().GetScene();
		Entity* rgTurret = EntityManager::FindChildren(vehicle, "GunTurret")[0];
		PxRaycastBuffer hit;
		if (scene->raycast(Transform::ToPx(rgTurret->GetTransform().GetGlobalPosition() - rgTurret->GetTransform().GetForward() * 5.0f), -Transform::ToPx(rgTurret->GetTransform().GetForward()), 400.0f, hit)) {
			if (hit.hasAnyHits()) {
				//Cube at Hit Location
				Entity* cube = EntityManager::CreateStaticEntity();
				EntityManager::AddComponent(*cube, MeshComponent("Cube.obj", "Basic.json"));
				cube->GetTransform().SetPosition(Transform::FromPx(hit.block.position));
				cube->GetTransform().SetScale(glm::vec3(0.1f, 0.1f, 0.1f));

				Entity* thingHit = EntityManager::FindEntity(hit.block.actor);
				std::vector<VehicleComponent> comps = EntityManager::Components<VehicleComponent>();
				for (size_t i = 0; i < comps.size(); i++) {
					if (thingHit != NULL && (comps[i].GetEntity().GetId() == thingHit->GetId())) {
						std::cout << "Entered Here" << std::endl;
						comps[i].TakeDamage(damage);
					}
				}
			}
		}

	} else if (StateManager::gameTime.GetTimeSeconds() < nextChargeTime.GetTimeSeconds()) {
		std::cout << "Rail Gun on Cooldown" << std::endl;
	} else {
		std::cout << "Still Charging..." << std::endl;
	}
}

void RailGunComponent::InternalCharge() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextChargeTime.GetTimeSeconds()) {
		nextShotTime = StateManager::gameTime + chargeTime;
		std::cout << "Charging" << std::endl;
	} else {
		std::cout << "Rail Gun on Cooldown" << std::endl;
	}
}

void RailGunComponent::InternalRenderDebugGui() {
	//RailGunComponent::RenderDebugGui();
}