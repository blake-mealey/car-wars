#include "PowerUpSpawnerComponent.h"
#include "VehicleComponent.h"
#include "../../Entities/Entity.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/StateManager.h"
#include "../PowerUpComponents/SpeedPowerUp.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Game.h"
#include "../PowerUpComponents/DefencePowerUp.h"
#include "../PowerUpComponents/DamagePowerUp.h"

PowerUpSpawnerComponent::PowerUpSpawnerComponent(nlohmann::json data): RigidStaticComponent(data) {
    respawnDuration = ContentManager::GetFromJson<double>(data["RespawnDuration"], 5.0);
    powerUpType = ContentManager::GetFromJson<PowerUpType>(data["PowerUpType"], Random);
    activePowerUp = nullptr;
    powerUpMesh = nullptr;
    lastPickupTime = -respawnDuration;
}

PowerUpSpawnerComponent::~PowerUpSpawnerComponent() { }

ComponentType PowerUpSpawnerComponent::GetType() {
    return ComponentType_PowerUpSpawner;
}

void PowerUpSpawnerComponent::OnTrigger(RigidbodyComponent* body) {
    VehicleComponent* vehicle = body->GetEntity()->GetComponent<VehicleComponent>();
    if (!vehicle) return;

    Collect(vehicle);
}

void PowerUpSpawnerComponent::RenderDebugGui() {
    RigidStaticComponent::RenderDebugGui();
}

void PowerUpSpawnerComponent::Respawn() {
    if (activePowerUp) return;
    if (StateManager::gameTime < lastPickupTime + respawnDuration) return;

    int type = powerUpType;
    if (type == Random) type = rand() % Count;
    switch (type) {
    case Speed:
        activePowerUp = new SpeedPowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("SpeedPowerUpMesh.json");
        GetEntity()->GetComponent<PointLightComponent>()->SetColor(glm::vec4(1.f, 1.f, 0.f, 1.f));
        break;
    case Damage:
        activePowerUp = new DamagePowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("DamagePowerUpMesh.json");
        GetEntity()->GetComponent<PointLightComponent>()->SetColor(glm::vec4(1.f, 0.f, 0.f, 1.f));
        break;
    case Defence:
        activePowerUp = new DefencePowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("DefencePowerUpMesh.json");
        GetEntity()->GetComponent<PointLightComponent>()->SetColor(glm::vec4(0.f, 1.f, 1.f, 1.f));
        break;
    default:
        return;
    }

    EntityManager::AddComponent(GetEntity(), powerUpMesh);
}

void PowerUpSpawnerComponent::Collect(VehicleComponent* vehicle) {
    if (!activePowerUp) return;

    PlayerData* player = Game::GetPlayerFromEntity(vehicle->GetEntity());
    if (!player || player->activePowerUp) return;

    lastPickupTime = StateManager::gameTime;
    activePowerUp->Collect(player);
    activePowerUp = nullptr;
    EntityManager::DestroyComponent(powerUpMesh);

    GetEntity()->GetComponent<PointLightComponent>()->SetColor(glm::vec4(1.f));
}

void PowerUpSpawnerComponent::SetPowerUpType(PowerUpType type) {
    powerUpType = type;
}
