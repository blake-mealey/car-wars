#pragma once

#include "../Component.h"

#include "../../Systems/Audio.h"
#include "../../Systems/StateManager.h"
#include "../../Entities/EntityManager.h"

#include "../MeshComponent.h"
#include "../RigidbodyComponents/RigidDynamicComponent.h"
#include "../../Systems/Physics.h"
#include "../Colliders/BoxCollider.h"

#include "../../Systems/Content/ContentManager.h"
#include "PxSimulationEventCallback.h"

class PowerUp : public Component {
public:
    virtual void Collect() = 0;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;

private:
    float multiplier;
};