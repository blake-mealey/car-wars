#include "PowerUpSpawnerComponent.h"
#include "VehicleComponent.h"
#include "../../Entities/Entity.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/StateManager.h"
#include "../PowerUpComponents/HealthPowerUp.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Game.h"
#include "../PowerUpComponents/DefencePowerUp.h"
#include "../PowerUpComponents/DamagePowerUp.h"
#include "../SpotLightComponent.h"
#include "PennerEasing/Quint.h"
#include "../../Systems/Effects.h"

PowerUpSpawnerComponent::PowerUpSpawnerComponent(nlohmann::json data): RigidStaticComponent(data) {
    respawnDuration = ContentManager::GetFromJson<double>(data["RespawnDuration"], baseDuration);
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
    //type = Health;
    switch (type) {
    case Health:
        activePowerUp = new HealthPowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("HealthPowerUpMesh.json");
        break;
    case Damage:
        activePowerUp = new DamagePowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("DamagePowerUpMesh.json");
        break;
    case Defence:
        activePowerUp = new DefencePowerUp();
        powerUpMesh = ContentManager::LoadComponent<MeshComponent>("DefencePowerUpMesh.json");
        break;
    default:
        return;
    }

	GetEntity()->GetComponent<PointLightComponent>()->SetColor(activePowerUp->GetColor());
    EntityManager::AddComponent(GetEntity(), powerUpMesh);
}

void PowerUpSpawnerComponent::Collect(VehicleComponent* vehicle) {
    if (!activePowerUp) return;

    PlayerData* player = Game::GetPlayerFromEntity(vehicle->GetEntity());
    if (!player || player->activePowerUp) return;

	std::vector<Entity*> headlights = EntityManager::FindChildren(vehicle->GetEntity(), "HeadLamp");
	for (Entity* entity : headlights) {
		Effects::Instance().DestroyTween("Headlight" + std::to_string(player->id));
		SpotLightComponent* light = entity->GetComponent<SpotLightComponent>();
		light->SetColor(activePowerUp->GetColor());

        MeshComponent* mesh = entity->GetComponent<MeshComponent>();
        mesh->GetMaterial()->diffuseColor = glm::mix(activePowerUp->GetColor(), glm::vec4(1.f), 0.25f);
	}

    lastPickupTime = StateManager::gameTime;
    activePowerUp->Collect(player);
    activePowerUp = nullptr;
    EntityManager::DestroyComponent(powerUpMesh);

    GetEntity()->GetComponent<PointLightComponent>()->SetColor(glm::vec4(1.f));
}

void PowerUpSpawnerComponent::SetPowerUpType(PowerUpType type) {
    powerUpType = type;
}

bool PowerUpSpawnerComponent::HasActivePowerup() const{
	return activePowerUp;
}