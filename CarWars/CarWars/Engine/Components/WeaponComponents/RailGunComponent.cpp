#include "RailGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/ContentManager.h"

RailGunComponent::RailGunComponent() : WeaponComponent(1150.0f) {}

void RailGunComponent::Shoot() {
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

		//Determine Player and Get Camera
		CameraComponent* vehicleCamera = nullptr;
		glm::vec3 cameraDirection;
		Game& gameInstance = Game::Instance();
		for (int i = 0; i < gameInstance.gameData.playerCount; ++i) {
			PlayerData& player = gameInstance.players[i];
			if (player.vehicleEntity->GetId() == vehicle->GetId()) {
				if (player.camera) {
					vehicleCamera = player.camera;
					cameraDirection = player.camera->GetTarget() - player.camera->GetPosition();
				}
			}
		}

		//Load Scene
		PxScene* scene = &Physics::Instance().GetScene();
		float rayLength = 100.0f;
		//Cast Camera Ray
		PxRaycastBuffer cameraHit;
		glm::vec3 cameraHitPosition;
		if (scene->raycast(Transform::ToPx(vehicleCamera->GetTarget()), Transform::ToPx(cameraDirection), rayLength, cameraHit)) {
			//cameraHit has hit something
			if (cameraHit.hasAnyHits()) {
				cameraHitPosition = Transform::FromPx(cameraHit.block.position);
			} else {
				//cameraHit has not hit anything
				cameraHitPosition = vehicleCamera->GetTarget() + (cameraDirection * rayLength);
			}
		}

		//Variables Needed
		glm::vec3 gunPosition = rgTurret->transform.GetGlobalPosition();
		glm::vec3 gunDirection = cameraHitPosition - gunPosition;

		//Cast Gun Ray
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit)) {
			if (gunHit.hasAnyHits()) {
				Entity* hitMarker = ContentManager::LoadEntity("Marker.json");
				hitMarker->transform.SetPosition(Transform::FromPx(gunHit.block.position));

				Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
				if (thingHit)
					if (thingHit->HasTag("Vehicle") || thingHit->HasTag("AiVehicle")) {
						std::cout << "Dealt : " << damage << std::endl;
						VehicleComponent* thingHitVehicleComponent = thingHit->GetComponent<VehicleComponent>();
						thingHitVehicleComponent->TakeDamage(damage);
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