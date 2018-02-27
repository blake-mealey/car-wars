#pragma once

#include "Component.h"
#include <json/json.hpp>

enum AiMode {
    AiMode_Waypoints,
    AiMode_Chase
};

class AiComponent : public Component {
public:
    AiComponent(nlohmann::json data);

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void RenderDebugGui() override;

    void SetTargetEntity(Entity* target);
    Entity* GetTargetEntity() const;
    AiMode GetMode() const;

    size_t NextWaypoint(size_t waypointCount);
    size_t GetWaypoint() const;
private:
    AiMode mode;
    Entity *targetEntity;
    size_t waypointIndex;
};

