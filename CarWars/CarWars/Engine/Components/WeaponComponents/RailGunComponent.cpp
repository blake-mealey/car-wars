#include "RailGunComponent.h"

RailGunComponent::RailGunComponent() {}

void RailGunComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextShotTime.GetTimeSeconds()) {
		nextChargeTime = StateManager::gameTime + timeBetweenShots;
		std::cout << "Shoot Rail Gun" << std::endl;
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);

		Entity* vehicle = this->GetEntity();
		PxScene* scene = &Physics::Instance().GetScene();
		Entity* rgTurret = EntityManager::FindChildren(vehicle, "GunTurret")[0];
		PxRaycastBuffer hit;
		if (scene->raycast(Transform::ToPx(rgTurret->transform.GetGlobalPosition() - rgTurret->transform.GetForward() * 5.0f), -Transform::ToPx(rgTurret->transform.GetForward()), 400.0f, hit)) {
			if (hit.hasAnyHits()) {
				//Cube at Hit Location
				Entity* cube = EntityManager::CreateStaticEntity();
				EntityManager::AddComponent(cube, new MeshComponent("Cube.obj", "Basic.json"));
				cube->transform.SetPosition(Transform::FromPx(hit.block.position));
				cube->transform.SetScale(glm::vec3(0.1f, 0.1f, 0.1f));

				Entity* thingHit = EntityManager::FindEntity(hit.block.actor);
				std::vector<Component*> comps = EntityManager::GetComponents(ComponentType_Vehicle);
				for (size_t i = 0; i < comps.size(); i++) {
					if (thingHit != NULL && (comps[i]->GetEntity()->GetId() == thingHit->GetId())) {
						std::cout << "Entered Here" << std::endl;
						static_cast<VehicleComponent*>(comps[i])->TakeDamage(damage);
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

void RailGunComponent::Charge() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextChargeTime.GetTimeSeconds()) {
		nextShotTime = StateManager::gameTime + chargeTime;
		std::cout << "Charging" << std::endl;
	} else {
		std::cout << "Rail Gun on Cooldown" << std::endl;
	}
}

ComponentType RailGunComponent::GetType() {
	return ComponentType_RailGun;
}

void RailGunComponent::HandleEvent(Event* event) {
	return;
}

void RailGunComponent::RenderDebugGui() {
	RailGunComponent::RenderDebugGui();
}