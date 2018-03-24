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

    // Initialize mesh and collider for the height map
    if (heightMap) {
        Entity* floor = ContentManager::LoadEntity("Game/Floor.json");
        floor->GetComponent<RigidStaticComponent>()->AddCollider(new MeshCollider({
            { "CollisionGroup", "Ground" },
            { "HeightMap", dirPath }
        }));

        EntityManager::AddComponent(floor, new MeshComponent({
            { "HeightMap", dirPath },
            { "Material", "Basic.json" },
            { "Texture", "Boulder.jpg" },
            { "UvScale", {10, 10} }
        }));
    }

    navigationMesh->UpdateMesh(EntityManager::GetComponents(ComponentType_RigidStatic));

    // TODO: Initialize powerups and spawners from image
}
