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

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <glm/gtc/type_ptr.hpp>

const float SPRAY = 1.1f;

MachineGunComponent::MachineGunComponent() : WeaponComponent(20.0f) {}

void MachineGunComponent::Shoot(glm::vec3 position) {
	if (StateManager::gameTime.GetSeconds() > nextShotTime.GetSeconds()) {
		Entity* vehicle = GetEntity();
		Entity* mgTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");
		const glm::vec3 gunPosition = mgTurret->transform.GetGlobalPosition();

		glm::vec3 gunDirection = position - gunPosition;
		float distanceToTarget = glm::length(gunDirection);
		gunDirection = glm::normalize(gunDirection);

		//work for clamping guns
		glm::vec3 up = vehicle->transform.GetUp();
		glm::vec3 front = -vehicle->transform.GetForward();
		glm::vec3 right = vehicle->transform.GetRight();

		glm::vec3 directionHorizontalPlane = glm::normalize(gunDirection - (glm::dot(gunDirection, up) * up)); //project to RF plane

		float horizontalAngle = acos(glm::dot(directionHorizontalPlane, front)) * (glm::dot(right, directionHorizontalPlane) > 0 ? 1 : -1);

		if (horizontalAngle) {
			mgTurret->transform.SetRotation(glm::quat());
			mgTurret->transform.Rotate(Transform::UP, horizontalAngle);
		}

		// pick a random point on a sphere for spray
		float randomHorizontalAngle = (float)rand() / (float)RAND_MAX * M_PI * 2.f;
		float randomVerticalAngle = (float)rand() / (float)RAND_MAX * M_PI;
		
		glm::vec3 randomOffset = glm::vec3(cos(randomHorizontalAngle) * sin(randomVerticalAngle),
			cos(randomVerticalAngle),
			sin(randomHorizontalAngle) * sin(randomVerticalAngle)) * SPRAY;

		glm::vec3 shotPosition = glm::normalize(gunDirection)*100.f + randomOffset + gunPosition;


		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime + timeBetweenShots;

		//Play Shooting Sound
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/machine_gun_shot.mp3");

		//Variables Needed
		const glm::vec3 shotDirection = glm::normalize(shotPosition - gunPosition);

		//Cast Gun Ray
	    PxScene* scene = &Physics::Instance().GetScene();
		const float rayLength = 1000.0f;
		PxRaycastBuffer cameraHit;
		PxQueryFilterData filterData;
		glm::vec3 hitPosition;
		filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
		PxRaycastBuffer gunHit;
		if (scene->raycast(Transform::ToPx(gunPosition), Transform::ToPx(shotDirection), rayLength, gunHit, PxHitFlag::eDEFAULT, filterData)) {
			hitPosition = Transform::FromPx(gunHit.block.position);
			Entity* thingHit = EntityManager::FindEntity(gunHit.block.actor);
            if (thingHit) {
				thingHit->TakeDamage(this, this->damage);
            }
		} else {
			hitPosition = gunPosition + (shotDirection * rayLength);
		}

		Entity* bullet = ContentManager::LoadEntity("Bullet.json");
		LineComponent* line = bullet->GetComponent<LineComponent>();
		line->SetPoint0(gunPosition);
		line->SetPoint1(hitPosition);
		auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeNone>(1.f, 0.f, timeBetweenShots*0.5, StateManager::gameTime);
		tween->SetUpdateCallback([line, mgTurret](float& value) mutable {
			line->SetColor(glm::vec4(1.0f, .9f, .4f, .3f));
			line->SetPoint0(mgTurret->transform.GetGlobalPosition());
		});
		tween->SetFinishedCallback([bullet](float& value) mutable {
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