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
#include "../LineComponent.h"
#include "../../Systems/Effects.h"
#include "PennerEasing/Linear.h"

MachineGunComponent::MachineGunComponent() : WeaponComponent(20.0f) {}

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
		const glm::vec3 gunPosition = mgTurret->transform.GetGlobalPosition();
		const glm::vec3 gunDirection = glm::normalize(position - gunPosition);

		//Cast Gun Ray
	    PxScene* scene = &Physics::Instance().GetScene();
		const float rayLength = 1000.0f;
		PxRaycastBuffer cameraHit;
		PxQueryFilterData filterData;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			if (gunHit.hasAnyHits()) {
                Entity* bullet = ContentManager::LoadEntity("Bullet.json");
                LineComponent* line = bullet->GetComponent<LineComponent>();
                const glm::vec3 start = gunPosition;
                const glm::vec3 end = Transform::FromPx(gunHit.block.position);
                auto tween = Effects::Instance().CreateTween<glm::vec3, easing::Linear::easeNone>(start, end, 0.1, StateManager::gameTime);
                tween->SetUpdateCallback([line, gunDirection](glm::vec3& value) mutable {
                    if (StateManager::GetState() != GameState_Playing) return;
                    line->SetPoint0(value);
                    line->SetPoint1(value + gunDirection * 5.f);
                });
                tween->SetFinishedCallback([bullet](glm::vec3& value) mutable {
                    if (StateManager::GetState() != GameState_Playing) return;
                    EntityManager::DestroyEntity(bullet);
                });
                tween->Start();

				Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
                if (thingHit) {
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