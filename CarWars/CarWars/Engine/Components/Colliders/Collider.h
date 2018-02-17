#pragma once

#include <string>
#include <PxShape.h>
#include <PxActor.h>
#include <PxMaterial.h>
#include <PxFiltering.h>
#include <json/json.hpp>

class Collider {
public:
    Collider(std::string _collisionGroup, physx::PxMaterial *_material);
    Collider(nlohmann::json data);
    ~Collider();

    physx::PxShape* GetShape() const;
    void CreateShape(physx::PxRigidActor *actor);

protected:
    virtual void InitializeGeometry() = 0;

    std::string collisionGroup;
    physx::PxShape *shape;
    physx::PxGeometry *geometry;
    physx::PxMaterial *material;
    physx::PxFilterData queryFilterData;
};
