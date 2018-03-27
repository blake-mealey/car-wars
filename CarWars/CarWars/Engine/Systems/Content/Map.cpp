#include "Map.h"
#include "ContentManager.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "../../Components/Colliders/MeshCollider.h"
#include "../../Components/MeshComponent.h"
#include <deque>
#include "../../Components/RigidbodyComponents/PowerUpSpawnerComponent.h"
#include "../../Components/RigidbodyComponents/RigidDynamicComponent.h"

using namespace nlohmann;

void SetPosition(Entity* entity, glm::vec3 position) {
    RigidbodyComponent* rigid = entity->GetComponent<RigidStaticComponent>();
    if (!rigid) rigid = entity->GetComponent<PowerUpSpawnerComponent>();
    if (!rigid) rigid = entity->GetComponent<RigidDynamicComponent>();
    if (rigid) {
        physx::PxTransform t = rigid->pxRigid->getGlobalPose();
        rigid->pxRigid->setGlobalPose(physx::PxTransform(Transform::ToPx(position), t.q));
    }
    entity->transform.SetPosition(position);
}

Map::Map(std::string dirPath) {
    // Load the map data
    json data = ContentManager::LoadJson(ContentManager::MAP_DIR_PATH + dirPath + "Data.json");
    mapWidth = ContentManager::GetFromJson<float>(data["MaxWidth"], 100.f);
    mapLength = ContentManager::GetFromJson<float>(data["MaxLength"], 100.f);
    
    heightMap = ContentManager::GetHeightMap(dirPath);
    navigationMesh = ContentManager::GetNavigationMesh(dirPath);

    // Load the map's scene
    ContentManager::DestroySceneAndLoadScene(data["Scene"]);

    // TODO: Initialize powerups and spawners from image
    Picture* objectsMap = new Picture(ContentManager::MAP_DIR_PATH + dirPath + "Objects.png");
    if (objectsMap->Pixels()) {
        LoadObjects(objectsMap);
    }

    if (heightMap) {
        // Initialize entity heights based on height map
        for (Entity* entity : EntityManager::GetChildren(EntityManager::GetRoot())) {
            const glm::vec3 position = entity->transform.GetLocalPosition();
            const float height = heightMap->GetHeight(position);
            SetPosition(entity, position + glm::vec3(0.f, height, 0.f));
        }

        // Initialize height map collider
        Entity* floor = ContentManager::LoadEntity("Game/Floor.json");
        floor->GetComponent<RigidStaticComponent>()->AddCollider(new MeshCollider({
            { "CollisionGroup", "Ground" },
            { "HeightMap", dirPath }
        }));

        // Initialize height map mesh
        EntityManager::AddComponent(floor, new MeshComponent({
            { "HeightMap", dirPath },
            { "Material", "Basic.json" },
            { "Texture", "Boulder.jpg" },
            { "UvScale",{ 10, 10 } }
        }));
    }

    navigationMesh->UpdateMesh();
}

void Map::LoadObjects(Picture* objectsMap) {
    const glm::vec3 generalPowerUpColor = glm::vec3(1.f, 0.f, 1.f);
    const glm::vec3 speedPowerUpColor = glm::vec3(1.f, 1.f, 0.f);
    const glm::vec3 damagePowerUpColor = glm::vec3(1.f, 0.f, 0.f);
    const glm::vec3 defencePowerUpColor = glm::vec3(0.f, 0.f, 1.f);
    
    const glm::vec3 spawnColor = glm::vec3(0.f, 1.f, 0.f);

    const glm::vec3 offset = -glm::vec3(mapWidth, 0.f, mapLength) * 0.5f;
    float* pixels = objectsMap->Pixels();
    for (int row = 0; row < objectsMap->Height(); ++row) {
        for (int col = 0; col < objectsMap->Width(); ++col) {
            const glm::vec3 color = glm::vec3(pixels[0], pixels[1], pixels[2]);
            Entity* object = nullptr;
            if (color == spawnColor) {
                object = ContentManager::LoadEntity("Game/SpawnLocation.json");
            } else if (color == generalPowerUpColor) {
                object = ContentManager::LoadEntity("Game/PowerUpSpawner.json");
            } else if (color == speedPowerUpColor) {
                object = ContentManager::LoadEntity("Game/PowerUpSpawner.json");
                object->GetComponent<PowerUpSpawnerComponent>()->SetPowerUpType(Speed);
            } else if (color == damagePowerUpColor) {
                object = ContentManager::LoadEntity("Game/PowerUpSpawner.json");
                object->GetComponent<PowerUpSpawnerComponent>()->SetPowerUpType(Damage);
            } else if (color == defencePowerUpColor) {
                object = ContentManager::LoadEntity("Game/PowerUpSpawner.json");
                object->GetComponent<PowerUpSpawnerComponent>()->SetPowerUpType(Defence);
            }
            
            if (object) {
                const glm::vec3 position = offset + glm::vec3(
                    static_cast<float>(col) / static_cast<float>(objectsMap->Width()) * mapLength,
                    2.f,
                    static_cast<float>(row) / static_cast<float>(objectsMap->Height()) * mapWidth);
                SetPosition(object, position);
            }
            
            pixels += objectsMap->Channels();
        }
    }
}
