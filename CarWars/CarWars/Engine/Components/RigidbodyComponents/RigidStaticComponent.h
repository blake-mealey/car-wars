#pragma once

#include "RigidbodyComponent.h"

class RigidStaticComponent : public RigidbodyComponent {
public:
    RigidStaticComponent(nlohmann::json data, Mesh* heightmap = nullptr);

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;

private:
    void InitializeRigidbody() override;
};