#include "RailGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/ContentManager.h"

RailGunComponent::RailGunComponent() : WeaponComponent(1150.0f) {}

void RailGunComponent::Shoot(glm::vec3 position) {
	if (StateManager::gameTime.GetTimeSeconds() >= nextShotTime.GetTimeSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();
		Entity* rgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);
		std::cout << "Rail Gun Shot" << std::endl;
		//Reset Next Charing Time
		nextChargeTime = StateManager::gameTime + timeBetweenShots;

		//Play Shot Sound

		//Variables Needed
		glm::vec3 gunPosition = rgTurret->transform.GetGlobalPosition();
		glm::vec3 gunDirection = position - gunPosition;

		//Load Scene
		PxScene* scene = &Physics::Instance().GetScene();
		float rayLength = 100.0f;
		//Cast Gun Ray
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit)) {
			if (gunHit.hasAnyHits()) {
				Entity* hitMarker = ContentManager::LoadEntity("Marker.json");
				hitMarker->transform.SetPosition(Transform::FromPx(gunHit.block.position));

				Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
				if (thingHit)
					if (thingHit->HasTag("Vehicle") || thingHit->HasTag("AiVehicle")) {
						thingHit->TakeDamage(damage);
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
		//Play Charging Sound
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