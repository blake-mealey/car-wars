#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/detail/type_vec3.hpp>
#include <deque>

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

    void UpdatePath();
    void NextNodeInPath();
    glm::vec3 NodeInPath() const;
    bool FinishedPath() const;
private:

    Entity *marker;
    std::deque<glm::vec3> path;

    AiMode mode;
    Entity *targetEntity;
    size_t waypointIndex;
};

