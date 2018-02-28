#pragma once

#include "RigidbodyComponent.h"

class RigidStaticComponent : public RigidbodyComponent<RigidStaticComponent> {
	friend class RigidbodyComponent<RigidStaticComponent>;
public:
	RigidStaticComponent();
    RigidStaticComponent(nlohmann::json data);

	static constexpr ComponentType InternalGetType() { return ComponentType_RigidStatic; }

	void InternalRenderDebugGui();

private:
	void InternalInitializeRigidbody();
};