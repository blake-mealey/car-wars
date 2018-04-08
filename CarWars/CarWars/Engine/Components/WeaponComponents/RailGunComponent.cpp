#include "RailGunComponent.h"

#include "../Component.h"
#include "../../Systems/Game.h"
#include "../../Systems/Audio.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Components/GuiComponents/GuiHelper.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/ParticleEmitterComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Physics/RaycastGroups.h"
#include "../LineComponent.h"
#include "PennerEasing/Linear.h"
#include "PennerEasing/Sine.h"
#include "../../Systems/Effects.h"

#include <string>
#include "PennerEasing/Quint.h"

RailGunComponent::~RailGunComponent() {
    if (beam) EntityManager::DestroyEntity(beam);
}

RailGunComponent::RailGunComponent() : WeaponComponent(1150.0f), beam(nullptr) {}

void RailGunComponent::Shoot(glm::vec3 position) {
	turnTurret(position);

    //Get Vehicle
    Entity* vehicle = GetEntity();
    Entity* rgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");

    //Variables Needed
    const glm::vec3 gunPosition = rgTurret->transform.GetGlobalPosition();
    const glm::vec3 gunDirection = position - gunPosition;

    //Load Scene
    PxScene* scene = &Physics::Instance().GetScene();
    float rayLength = 100.0f;
    //Cast Gun Ray
    glm::vec3 hitPosition;
    PxRaycastBuffer gunHit;
    PxQueryFilterData filterData;
    filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
    const bool didHit = scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(gunDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData);
    if (didHit) {
        hitPosition = Transform::FromPx(gunHit.block.position);
    } else {
        hitPosition = gunPosition + (gunDirection * rayLength);
    }

	if (StateManager::gameTime.GetSeconds() >= nextShotTime.GetSeconds()) {
        playingChargeSound = false;
		//Audio::Instance().StopSound(soundIndex);
		Audio::Instance().StopSound3D(soundIndex);

		//Audio::Instance().PlayAudio2D("Content/Sounds/railgun-shoot.mp3");
		Audio::Instance().PlayAudio3D("Content/Sounds/railgun-shoot.mp3", vehicle->transform.GetGlobalPosition(), glm::vec3(0.f, 0.f, 0.f));

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);
		//Reset Next Charing Time
		nextChargeTime = StateManager::gameTime + timeBetweenShots;

        if (didHit) {
            Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
			if (thingHit && vehicle) {
				if(thingHit->GetComponent<VehicleComponent>())
					thingHit->GetComponent<VehicleComponent>()->pxVehicle->getRigidDynamicActor()->addForce(Transform::ToPx(glm::normalize(thingHit->transform.GetGlobalPosition() - vehicle->transform.GetGlobalPosition()) * 40000.f), PxForceMode::eIMPULSE, true);
				thingHit->TakeDamage(this, GetDamage());
			}
        }
		
		PlayerData* player = Game::Instance().GetPlayerFromEntity(GetEntity());

        Transform& beamTransform = GetBeam()->transform;
        Transform& beamMeshTransform = beam->GetComponent<MeshComponent>()->transform;
        ParticleEmitterComponent* emitter = beam->GetComponent<ParticleEmitterComponent>();

        auto tweenIn = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.2, StateManager::gameTime);
        tweenIn->SetUpdateCallback([emitter, &beamTransform, &beamMeshTransform, hitPosition, rgTurret, player](float& value) mutable {
            if (!player->alive) return;
            beamTransform.SetPosition(0.5f * (rgTurret->transform.GetGlobalPosition() + hitPosition));
            float radius = glm::mix(0.1f, 1.f, value);
            beamMeshTransform.SetScale(glm::vec3(radius, radius, length(rgTurret->transform.GetGlobalPosition() - hitPosition)));
            beamTransform.LookAt(hitPosition);
            emitter->SetInitialScale(glm::vec2(radius*4.f));
            emitter->SetFinalScale(glm::vec2(radius*4.f));
        });

        float startRadius = beamMeshTransform.GetLocalScale().x;
		auto tweenOut = Effects::Instance().CreateTween<float, easing::Quint::easeIn>(0.f, 1.f, 0.2, StateManager::gameTime);
        tweenOut->SetUpdateCallback([emitter, startRadius, &beamTransform, &beamMeshTransform, hitPosition, rgTurret, player](float& value) mutable {
			if (!player->alive) return;
            beamTransform.SetPosition(0.5f * (rgTurret->transform.GetGlobalPosition() + hitPosition));
            float radius = glm::mix(startRadius, 0.f, value);
            beamMeshTransform.SetScale(glm::vec3(radius, radius, length(rgTurret->transform.GetGlobalPosition() - hitPosition)));
            beamTransform.LookAt(hitPosition);
            emitter->SetInitialScale(glm::vec2(radius*4.f));
            emitter->SetFinalScale(glm::vec2(radius*4.f));
		});
        tweenOut->SetFinishedCallback([this](float& value) mutable {
			EntityManager::DestroyEntity(beam);
            beam = nullptr;
		});


        tweenIn->SetNext(tweenOut, 0.1);
        tweenIn->Start();

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
			//soundIndex = Audio::Instance().PlaySound("Content/Sounds/railgun-charge.mp3");
			soundIndex = Audio::Instance().PlaySound3D("Content/Sounds/railgun-charge.mp3", GetEntity()->transform.GetGlobalPosition(), glm::vec3(0.f, 0.f, 0.f), 0.22f);
            playingChargeSound = true;
        }

		HumanData* player = Game::Instance().GetHumanFromEntity(GetEntity());
		if (player) {
			Tween* chargeTween = Effects::Instance().FindTween("RailGunChargeIn" + std::to_string(player->id));
			if (!chargeTween) Charge();
		}

        const float ratio = ((StateManager::gameTime - (nextShotTime - chargeTime)) / chargeTime).GetSeconds();
        const glm::vec3 direction = hitPosition - gunPosition;
        const float distance = length(direction);
        const float radius = 0.1f;// glm::mix(0.f, 1.f, ratio);
        MeshComponent* beamMesh = GetBeam()->GetComponent<MeshComponent>();
        beamMesh->transform.SetScale(glm::vec3(radius, radius, distance));
        beam->transform.SetPosition(0.5f * (gunPosition + hitPosition));
        beam->transform.LookAt(hitPosition);

        ParticleEmitterComponent* emitter = beam->GetComponent<ParticleEmitterComponent>();
        emitter->SetEmitScale(glm::vec3(0.1f, 0.1f, distance*0.5f));
        emitter->SetEmitCount(distance*0.25f);
        emitter->SetInitialScale(glm::vec2(radius*4.f));
        emitter->SetFinalScale(glm::vec2(radius*4.f));
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

            auto tweenIn = Effects::Instance().CreateTween <float, easing::Sine::easeOut>(0.f, 1.f, chargeTime, StateManager::gameTime);
            tweenIn->SetUpdateCallback([&mask](float& value) mutable {
                mask.SetScale(glm::vec3(134.f, glm::mix(134.f, 0.f, value), 0.f));
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
	//Audio::Instance().StopSound(soundIndex);
	Audio::Instance().StopSound3D(soundIndex);
	if (player) {
		Tween* outTween = Effects::Instance().FindTween("RailGunChargeOut" + std::to_string(player->id));
		if (outTween) return;

		Tween* oldTween = Effects::Instance().FindTween("RailGunChargeIn" + std::to_string(player->id));
		if (oldTween) Effects::Instance().DestroyTween(oldTween);

		GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "ChargeIndicator"));
		Transform& mask = gui->GetMask();

        Transform& transform = GetEntity()->transform;
        Transform& beamMeshTransform = GetBeam()->GetComponent<MeshComponent>()->transform;
        ParticleEmitterComponent* emitter = beam->GetComponent<ParticleEmitterComponent>();
        float startRadius = beamMeshTransform.GetLocalScale().x;

        const glm::vec3 scaleStart = mask.GetLocalScale();
        const glm::vec3 scaleEnd = glm::vec3(134.f, 134.f, 0.f);
		auto tweenOut = Effects::Instance().CreateTween<float, easing::Sine::easeIn>(0.f, 1.f, 0.01, StateManager::gameTime);
		tweenOut->SetUpdateCallback([emitter, &beamMeshTransform, &transform, &mask, startRadius, scaleStart, scaleEnd](float& value) {
			mask.SetScale(mix(scaleStart, scaleEnd, value));

            float radius = glm::mix(startRadius, 0.f, value);
            beamMeshTransform.SetScale(glm::vec3(radius, radius, beamMeshTransform.GetLocalScale().z));
		    emitter->SetInitialScale(glm::vec2(radius*3.f));
            emitter->SetFinalScale(glm::vec2(radius*3.f));
		});
        tweenOut->SetFinishedCallback([this](float& value) mutable {
            EntityManager::DestroyEntity(beam);
            beam = nullptr;
        });
		tweenOut->SetTag("RailGunChargeOut" + std::to_string(player->id));
		tweenOut->Start();
	}
}

Entity* RailGunComponent::GetBeam() {
    if (!beam) beam = ContentManager::LoadEntity("Beam.json");
    return beam;
}

Time RailGunComponent::GetChargeTime() {
	RailGunComponent railgun;
	return railgun.chargeTime;
}
