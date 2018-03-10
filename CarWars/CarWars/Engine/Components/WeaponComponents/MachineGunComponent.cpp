#include "MachineGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/CollisionGroups.h"
#include "../../Systems/Physics/RaycastGroups.h"

MachineGunComponent::MachineGunComponent() : WeaponComponent(20.0f) {}

void MachineGunComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();
		Entity* mgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + timeBetweenShots;
		std::cout << "Bullet Shot" << damage << std::endl;

		//Play Shooting Sound
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/machine_gun_shot.mp3");

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
		PxQueryFilterData filterData;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		glm::vec3 cameraHitPosition;
		if (scene->raycast(Transform::ToPx(vehicleCamera->GetTarget()), Transform::ToPx(cameraDirection), rayLength, cameraHit, PxHitFlag::eDEFAULT, filterData)) {
			//cameraHit has hit something
			if (cameraHit.hasAnyHits()) {
				cameraHitPosition = Transform::FromPx(cameraHit.block.position);
				EntityManager::FindEntity(cameraHit.block.actor);
			} else {
				//cameraHit has not hit anything
				cameraHitPosition = vehicleCamera->GetTarget() + (cameraDirection * rayLength);
			}
		}
		
		//Variables Needed
		glm::vec3 gunPosition = mgTurret->transform.GetGlobalPosition();
		glm::vec3 gunDirection = cameraHitPosition - gunPosition;

		//Cast Gun Ray
		PxRaycastBuffer gunHit;

		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
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
	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void MachineGunComponent::Charge() {
	Shoot();
}

ComponentType MachineGunComponent::GetType() {
	return ComponentType_MachineGun;
}

void MachineGunComponent::HandleEvent(Event *event) {
	return;
}

void MachineGunComponent::RenderDebugGui() {
    WeaponComponent::RenderDebugGui();
}