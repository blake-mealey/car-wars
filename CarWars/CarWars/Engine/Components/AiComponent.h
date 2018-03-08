#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/detail/type_vec3.hpp>
#include <deque>
#include "../Systems/Time.h"
#include <GL/glew.h>

enum AiMode {
	AiMode_Reverse,
	AiMode_Stuck,
	AiMode_DriveTo,
	AiMode_Attack,
	AiMode_Evade,

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
	void SetMode();

    void UpdatePath();
    void NextNodeInPath();
    glm::vec3 NodeInPath() const;
    bool FinishedPath() const;

	Time GetModeDuration();
	void StartStuckTime();
	Time GetStuckDuration();

	
private:

    Time lastPathUpdate;

    Time startedStuck;

    std::vector<glm::vec3> path;

    AiMode mode;
	AiMode previousMode;
	Time modeStart;

    Entity *targetEntity;
    size_t waypointIndex;

    void InitializeRenderBuffers();
    void UpdateRenderBuffers();
};