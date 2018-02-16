#include "CollisionFilterShader.h"

using namespace physx;

void CollisionGroups::AddCollisionGroup(std::string name, std::vector<std::string> collidesWith) {
    CollisionGroup group;
    group.flag = 1 << nextFlagOffset++;
    group.mask = 0;
    group.collidesWith = collidesWith;
    groups[name] = group;
}

void CollisionGroups::InitializeMasks() {
    for (auto& kv : groups) {
        CollisionGroup& group = kv.second;
        for (std::string other : group.collidesWith) {
            group.mask |= groups[other].flag;
        }
    }
}

physx::PxU32 CollisionGroups::GetFlag(std::string name) {
    return groups[name].flag;
}

physx::PxU32 CollisionGroups::GetMask(std::string name) {
    return groups[name].mask;
}

physx::PxFilterData CollisionGroups::GetFilterData(std::string name) {
    CollisionGroup &group = groups[name];
    return PxFilterData(group.flag, group.mask, 0, 0);
}

PxFilterFlags CollisionGroups::FilterShader(PxFilterObjectAttributes attributes0,
    PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
    
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(constantBlock);
    PX_UNUSED(constantBlockSize);

    if ((0 == (filterData0.word0 & filterData1.word1)) && (0 == (filterData1.word0 & filterData0.word1)))
        return PxFilterFlag::eSUPPRESS;

    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    pairFlags |= PxPairFlags(PxU16(filterData0.word2 | filterData1.word2));

    return PxFilterFlags();
}
