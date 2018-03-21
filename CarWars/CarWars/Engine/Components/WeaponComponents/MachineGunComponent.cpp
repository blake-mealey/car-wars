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
		glm::vec3 hitPosition;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			hitPosition = Transform::FromPx(gunHit.block.position);
			Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
            if (thingHit) {
				thingHit->TakeDamage(this, this->damage);
            }
		} else {
			hitPosition = gunPosition + (gunDirection * rayLength);
		}

		Entity* bullet = ContentManager::LoadEntity("Bullet.json");
		LineComponent* line = bullet->GetComponent<LineComponent>();
		line->SetPoint0(gunPosition);
		line->SetPoint1(hitPosition);
		auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeNone>(1.f, 0.f, timeBetweenShots*0.5, StateManager::gameTime);
		tween->SetUpdateCallback([line, mgTurret](float& value) mutable {
			if (StateManager::GetState() != GameState_Playing) return;
			line->SetColor(glm::vec4(1.f, 1.f, 1.f, value));
			line->SetPoint0(mgTurret->transform.GetGlobalPosition());
		});
		tween->SetFinishedCallback([bullet](float& value) mutable {
			if (StateManager::GetState() != GameState_Playing) return;
			EntityManager::DestroyEntity(bullet);
		});
		tween->Start();

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