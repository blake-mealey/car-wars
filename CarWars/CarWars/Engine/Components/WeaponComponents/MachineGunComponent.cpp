#include "MachineGunComponent.h"

#include "../Component.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/RaycastGroups.h"
#include "../../Systems/Audio.h"
#include "../../Systems/StateManager.h"
#include "../../Systems/Physics.h"
#include "../RigidbodyComponents/VehicleComponent.h"

MachineGunComponent::MachineGunComponent() : WeaponComponent(100.0f) {}

void MachineGunComponent::Shoot(glm::vec3 position) {
	if (StateManager::gameTime.GetSeconds() > nextShotTime.GetSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();
		Entity* mgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + timeBetweenShots;

		//Play Shooting Sound
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/machine_gun_shot.mp3");

		//Variables Needed
		glm::vec3 gunPosition = mgTurret->transform.GetGlobalPosition();
		glm::vec3 gunDirection = position - gunPosition;

		//Cast Gun Ray
	    PxScene* scene = &Physics::Instance().GetScene();
		float rayLength = 100.0f;
		PxRaycastBuffer cameraHit;
		PxQueryFilterData filterData;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			if (gunHit.hasAnyHits()) {
				Entity* hitMarker = ContentManager::LoadEntity("Marker.json");
				hitMarker->transform.SetPosition(Transform::FromPx(gunHit.block.position));

				Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
				if (thingHit)
				if (thingHit->HasTag("Vehicle") || thingHit->HasTag("AiVehicle")) {
					thingHit->TakeDamage(this);
				}
			}
		}
	} else { // betweeen shots
	}
}

void MachineGunComponent::Charge() {
	return;
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