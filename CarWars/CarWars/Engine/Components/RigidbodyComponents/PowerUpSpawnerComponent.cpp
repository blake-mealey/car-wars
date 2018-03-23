#include "PowerUpSpawnerComponent.h"
#include "VehicleComponent.h"
#include "../../Entities/Entity.h"
#include <iostream>

PowerUpSpawnerComponent::PowerUpSpawnerComponent(const nlohmann::json data): RigidStaticComponent(data) { }

PowerUpSpawnerComponent::~PowerUpSpawnerComponent() { }

ComponentType PowerUpSpawnerComponent::GetType() {
    return ComponentType_PowerUpSpawner;
}

void PowerUpSpawnerComponent::OnTrigger(RigidbodyComponent* body) {
    VehicleComponent* vehicle = body->GetEntity()->GetComponent<VehicleComponent>();
    if (!vehicle) return;

    std::cout << "Powerup!" << std::endl;
}

void PowerUpSpawnerComponent::RenderDebugGui() {
    RigidStaticComponent::RenderDebugGui();
}
