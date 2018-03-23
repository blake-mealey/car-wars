#pragma once

#include "RigidStaticComponent.h"
#include "../PowerUpComponents/PowerUp.h"

class VehicleComponent;

enum PowerUpType {
    Speed=0,
    Damage,
    Defence,
    Count
};

class PowerUpSpawnerComponent : public RigidStaticComponent {
public:
    explicit PowerUpSpawnerComponent(nlohmann::json data);
    ~PowerUpSpawnerComponent() override;
    
    ComponentType GetType() override;
    
    void OnTrigger(RigidbodyComponent* body) override;
    
    void RenderDebugGui() override;

    void Respawn();
    void Collect(VehicleComponent* vehicle);
private:
    Time respawnDuration;
    Time lastPickupTime;

    PowerUp* activePowerUp;
    MeshComponent* powerUpMesh;

    int powerUpType;
};
