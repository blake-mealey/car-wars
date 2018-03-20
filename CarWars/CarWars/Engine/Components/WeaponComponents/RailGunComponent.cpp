#include "RailGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/RaycastGroups.h"
#include "../LineComponent.h"
#include "PennerEasing/Linear.h"
#include "../../Systems/Effects.h"

RailGunComponent::RailGunComponent() : WeaponComponent(1150.0f) {}

void RailGunComponent::Shoot(glm::vec3 position) {
	if (StateManager::gameTime.GetSeconds() >= nextShotTime.GetSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();
		Entity* rgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);
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
		PxQueryFilterData filterData;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			if (gunHit.hasAnyHits()) {
                Entity* bullet = ContentManager::LoadEntity("Bullet.json");
                LineComponent* line = bullet->GetComponent<LineComponent>();
                line->SetPoint0(gunPosition);
                line->SetPoint1(Transform::FromPx(gunHit.block.position));
                auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeNone>(1.f, 0.f, 0.1);
                tween->SetUpdateCallback([line, rgTurret](float& value) mutable {
                    if (StateManager::GetState() != GameState_Playing) return;
                    line->SetColor(glm::vec4(1.f, 0.f, 0.f, value));
                    line->SetPoint0(rgTurret->transform.GetGlobalPosition());
                });
                tween->SetFinishedCallback([bullet](float& value) mutable {
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
        TweenChargeIndicator();
	} else if (StateManager::gameTime.GetSeconds() < nextChargeTime.GetSeconds()) {
	} else {
	}
}

void RailGunComponent::Charge() {
	if (StateManager::gameTime.GetSeconds() >= nextChargeTime.GetSeconds()) { // charging
		nextShotTime = StateManager::gameTime + chargeTime;
		//Play Charging Sound
	} else { // on cooldown
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