#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/detail/type_vec3.hpp>
#include <deque>
#include "../Systems/Time.h"
#include <GL/glew.h>

enum AiMode {
    AiMode_Waypoints,
    AiMode_Chase
};

class AiComponent : public Component {
public:
    ~AiComponent() override;
    AiComponent(nlohmann::json data);
    size_t GetPathLength() const;

    GLuint pathVbo;
    GLuint pathVao;

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

    void StartReversing();
    void StopReversing();
    Time GetReversingDuration() const;
    bool IsReversing() const;

    void SetStuck(bool _stuck);
    Time GetStuckDuration() const;
    bool IsStuck() const;
private:
    Time lastPathUpdate;

    Time startedStuck;
    bool stuck;

    Time startedReversing;
    bool reversing;

    std::vector<glm::vec3> path;

    AiMode mode;
    Entity *targetEntity;
    size_t waypointIndex;

    void InitializeRenderBuffers();
    void UpdateRenderBuffers();
};

