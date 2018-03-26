#include "Map.h"
#include "ContentManager.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "../../Components/Colliders/MeshCollider.h"
#include "../../Components/MeshComponent.h"

using namespace nlohmann;

Map::Map(std::string dirPath) {
    // Load the map data
    json data = ContentManager::LoadJson(ContentManager::MAP_DIR_PATH + dirPath + "Data.json");
    heightMap = ContentManager::GetHeightMap(dirPath);
    navigationMesh = ContentManager::GetNavigationMesh(dirPath);

    // Load the map's scene
    ContentManager::DestroySceneAndLoadScene(data["Scene"]);

    navigationMesh->UpdateMesh(EntityManager::GetComponents(ComponentType_RigidStatic));

    // TODO: Initialize powerups and spawners from image

    if (heightMap) {
        // Initialize entity heights based on height map
        for (Entity* entity : EntityManager::GetChildren(EntityManager::GetRoot())) {
            const float height = heightMap->GetHeight(entity->transform.GetLocalPosition());
            entity->transform.Translate(glm::vec3(0.f, height, 0.f));
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
}
