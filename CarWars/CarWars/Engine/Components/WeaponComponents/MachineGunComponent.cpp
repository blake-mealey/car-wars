#include "MachineGunComponent.h"

MachineGunComponent::MachineGunComponent() {}

void MachineGunComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Bullet Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

		Entity* vehicle = this->GetEntity();

		static_cast<VehicleComponent*>(vehicle->components[2])->TakeDamage(damage);

		PxScene* scene = &Physics::Instance().GetScene();
		Entity* mgTurret = EntityManager::FindChildren(this->GetEntity(), "GunTurret")[0];
		PxRaycastBuffer hit;
		PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
		if (scene->raycast(Transform::ToPx(mgTurret->transform.GetGlobalPosition()), Transform::ToPx(mgTurret->transform.GetForward()), 100000000.0f, hit, PxHitFlag::eDEFAULT, filterData)) {
			if (hit.hasAnyHits()) {
				std::cout << glm::to_string(Transform::FromPx(hit.block.position)) << std::endl;
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