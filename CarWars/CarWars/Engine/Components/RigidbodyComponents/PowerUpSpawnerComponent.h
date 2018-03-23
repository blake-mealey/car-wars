#pragma once

#include "RigidStaticComponent.h"

class PowerUpSpawnerComponent : public RigidStaticComponent {
public:
    explicit PowerUpSpawnerComponent(const nlohmann::json data);
    ~PowerUpSpawnerComponent() override;
    
    ComponentType GetType() override;
    
    void OnTrigger(RigidbodyComponent* body) override;
    
    void RenderDebugGui() override;
};
