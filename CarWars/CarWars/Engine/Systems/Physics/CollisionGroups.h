#pragma once

#include "PxPhysicsAPI.h"
#include <vector>
#include <unordered_map>
#include <json/json.hpp>

struct CollisionGroup {
    physx::PxU32 flag;
    physx::PxU32 mask;
    std::vector<std::string> collidesWith;
};

class CollisionGroups {
public:
    static void AddCollisionGroup(std::string name, std::vector<std::string> collidesWith);
    static void InitializeMasks();

    static physx::PxU32 GetFlag(std::string name);
    static physx::PxU32 GetMask(std::string name);

    static physx::PxFilterData GetFilterData(std::string name);

    static physx::PxFilterFlags FilterShader
    (physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

private:
    static physx::PxU32 nextFlagOffset;
    static std::unordered_map<std::string, CollisionGroup> groups;
};
