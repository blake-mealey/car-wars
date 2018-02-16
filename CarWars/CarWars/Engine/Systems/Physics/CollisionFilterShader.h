#pragma once

#include "PxPhysicsAPI.h"
#include <vector>
#include <unordered_map>
#include <json/json.hpp>

namespace CollisionGroups {
    struct CollisionGroup {
        physx::PxU32 flag;
        physx::PxU32 mask;
        std::vector<std::string> collidesWith;
    };

    static physx::PxU32 nextFlagOffset = 0;
    static auto groups = std::unordered_map<std::string, CollisionGroup>();

    void AddCollisionGroup(std::string name, std::vector<std::string> collidesWith);
    void InitializeMasks();

    physx::PxU32 GetFlag(std::string name);
    physx::PxU32 GetMask(std::string name);

    physx::PxFilterData GetFilterData(std::string name);

    physx::PxFilterFlags FilterShader
    (physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

}
