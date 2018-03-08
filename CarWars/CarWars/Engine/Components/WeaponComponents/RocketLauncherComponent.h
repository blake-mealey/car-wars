#pragma once
#include <iostream>

#include "../../Systems/Audio.h"
#include "../../Systems/StateManager.h"
#include "../../Entities/EntityManager.h"

#include "../MeshComponent.h"
#include "MissileComponent.h"
#include "../RigidbodyComponents/RigidDynamicComponent.h"
#include "../../Systems/Physics.h"
#include "../Colliders/BoxCollider.h"

#include "../../Systems/Content/ContentManager.h"
#include "PxSimulationEventCallback.h"

#include "WeaponComponent.h"

class RocketLauncherComponent : public WeaponComponent<RocketLauncherComponent> {
	friend class WeaponComponent<RocketLauncherComponent>;
public:
	RocketLauncherComponent();
	void InternalShoot();
	void InternalCharge();

	static constexpr ComponentType InternalGetType() { return ComponentType_RailGun; }

	void InternalRenderDebugGui();
private:
	Time timeBetweenShots = 1.0f / 0.75f;

	float damage = 500.0f;
};