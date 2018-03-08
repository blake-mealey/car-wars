#pragma once

#include "RigidbodyComponent.h"

class RigidStaticComponent : public RigidbodyComponent {
public:
	RigidStaticComponent(nlohmann::json data);

	static constexpr ComponentType GetType() { return ComponentType_RigidStatic; }

	void RenderDebugGui();

private:
	void InitializeRigidbody();
};