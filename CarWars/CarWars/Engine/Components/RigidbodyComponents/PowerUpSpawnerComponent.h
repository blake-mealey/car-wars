#pragma once

#include "RigidStaticComponent.h"
#include "../PowerUpComponents/PowerUp.h"

class VehicleComponent;

enum PowerUpType {
    Health=0,
    Damage,
    Defence,
    Random,
    Count
};

class PowerUpSpawnerComponent : public RigidStaticComponent {
public:
    float oscillation = 1.0f;
    explicit PowerUpSpawnerComponent(nlohmann::json data);
    ~PowerUpSpawnerComponent() override;
    
    ComponentType GetType() override;
    
    void OnTrigger(RigidbodyComponent* body) override;
    
    void RenderDebugGui() override;

    void Respawn();
    void Collect(VehicleComponent* vehicle);

    void SetPowerUpType(PowerUpType type);

	bool HasActivePowerup() const;

private:
    double baseDuration = 15.0;
    Time respawnDuration;
    Time lastPickupTime;

    PowerUp* activePowerUp;
    MeshComponent* powerUpMesh;

    PowerUpType powerUpType;
};
