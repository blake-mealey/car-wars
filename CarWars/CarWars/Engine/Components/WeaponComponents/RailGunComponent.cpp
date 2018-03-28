#include "RailGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Systems/Audio.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Components/GuiComponents/GuiHelper.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/RaycastGroups.h"
#include "../LineComponent.h"
#include "PennerEasing/Linear.h"
#include "PennerEasing/Sine.h"
#include "../../Systems/Effects.h"

#include <string>

RailGunComponent::RailGunComponent() : WeaponComponent(1150.0f) {}

void RailGunComponent::Shoot(glm::vec3 position) {
	turnTurret(position);
	if (StateManager::gameTime.GetSeconds() >= nextShotTime.GetSeconds()) {
        playingChargeSound = false;
		Audio::Instance().StopSound(soundIndex);
		//Audio::Instance().StopSound3D(soundIndex);
		Audio::Instance().PlayAudio2D("Content/Sounds/railgun-shoot.mp3");

		//Get Vehicle
		Entity* vehicle = GetEntity();
		//Audio::Instance().PlayAudio3D("Content/Sounds/railgun-shoot.mp3", vehicle->transform.GetGlobalPosition(), glm::vec3(0.f, 0.f, 0.f));

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
		glm::vec3 hitPosition;
		PxRaycastBuffer gunHit;
		PxQueryFilterData filterData;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			hitPosition = Transform::FromPx(gunHit.block.position);
			Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
            if (thingHit) {
				thingHit->TakeDamage(this, GetDamage());
            }
		} else {
			hitPosition = gunPosition + (gunDirection * rayLength);
		}
		PlayerData* player = Game::Instance().GetPlayerFromEntity(GetEntity());

		Entity* bullet = ContentManager::LoadEntity("Bullet.json");
		LineComponent* line = bullet->GetComponent<LineComponent>();
		line->SetPoint0(gunPosition);
		line->SetPoint1(hitPosition);
		auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeNone>(1.f, 0.f, 0.1, StateManager::gameTime);
		tween->SetUpdateCallback([line, rgTurret, player, tween](float& value) mutable {
			if (!player->alive) return;
			line->SetColor(glm::vec4(1.f, 0.f, 0.f, value));
			line->SetPoint0(rgTurret->transform.GetGlobalPosition());
		});
		tween->SetFinishedCallback([bullet](float& value) mutable {
			EntityManager::DestroyEntity(bullet);
		});
		tween->Start();

		HumanData* human = Game::Instance().GetHumanFromEntity(GetEntity());
		if (human) {
			GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(human->camera->GetGuiRoot(), "ChargeIndicator"));
			Transform& mask = gui->GetMask();

			auto tweenOut = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeIn>(
				glm::vec3(134.f, 0.f, 0.f), glm::vec3(134.f, 134.f, 0.f), timeBetweenShots, StateManager::gameTime);
			tweenOut->SetUpdateCallback([&mask](glm::vec3& value) {
				mask.SetScale(value);
			});
			tweenOut->SetTag("RailGunChargeOut" + std::to_string(player->id));
			tweenOut->Start();
		}
	} else if (StateManager::gameTime > nextChargeTime && StateManager::gameTime < nextShotTime) {
        //Play Charging Sound
        if (!playingChargeSound) {
			soundIndex = Audio::Instance().PlaySound("Content/Sounds/railgun-charge.mp3");
			//soundIndex = Audio::Instance().PlaySound3D("Content/Sounds/railgun-charge.mp3", GetEntity()->transform.GetGlobalPosition(), glm::vec3(0.f, 0.f, 0.f), 0.35f);
            playingChargeSound = true;
        }

		HumanData* player = Game::Instance().GetHumanFromEntity(GetEntity());
		if (player) {
			Tween* chargeTween = Effects::Instance().FindTween("RailGunChargeIn" + std::to_string(player->id));
			if (!chargeTween) {
				Charge();
			}
		}
	} else{
	}
}

void RailGunComponent::Charge() {
	if (StateManager::gameTime.GetSeconds() >= nextChargeTime.GetSeconds()) { // charging
		nextShotTime = StateManager::gameTime + chargeTime;
		

		HumanData* player = Game::Instance().GetHumanFromEntity(GetEntity());
		if (player) {
			Tween* oldTween = Effects::Instance().FindTween("RailGunChargeOut" + std::to_string(player->id));
			if (oldTween) Effects::Instance().DestroyTween(oldTween);

			GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "ChargeIndicator"));
			Transform& mask = gui->GetMask();
			
			auto tweenIn = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeOut>(
				glm::vec3(134.f, 134.f, 0.f), glm::vec3(134.f, 0.f, 0.f), chargeTime, StateManager::gameTime);
			tweenIn->SetUpdateCallback([&mask](glm::vec3& value) {
				mask.SetScale(value);
			});
			tweenIn->SetTag("RailGunChargeIn" + std::to_string(player->id));
			tweenIn->Start();
		}
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

void RailGunComponent::ChargeRelease() {
	HumanData* player = Game::Instance().GetHumanFromEntity(GetEntity());
    playingChargeSound = false;
	Audio::Instance().StopSound(soundIndex);
	//Audio::Instance().StopSound3D(soundIndex);
	if (player) {
		Tween* outTween = Effects::Instance().FindTween("RailGunChargeOut" + std::to_string(player->id));
		if (outTween) return;

		Tween* oldTween = Effects::Instance().FindTween("RailGunChargeIn" + std::to_string(player->id));
		if (oldTween) Effects::Instance().DestroyTween(oldTween);

		GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "ChargeIndicator"));
		Transform& mask = gui->GetMask();

		auto tweenOut = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeIn>(
			mask.GetLocalScale(), glm::vec3(134.f, 134.f, 0.f), 0.05, StateManager::gameTime);
		tweenOut->SetUpdateCallback([&mask](glm::vec3& value) {
			mask.SetScale(value);
		});
		tweenOut->SetTag("RailGunChargeOut" + std::to_string(player->id));
		tweenOut->Start();
	}
}
