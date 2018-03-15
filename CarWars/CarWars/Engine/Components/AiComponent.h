#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/detail/type_vec3.hpp>
#include <deque>
#include "../Systems/Time.h"
#include <GL/glew.h>

enum AiMode {
	AiMode_Stuck,
	AiMode_GetPowerup,
	AiMode_Attack,
	AiMode_Damaged,
	AiMode_Hide,

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

	void Update();

	void UpdateMode(AiMode _mode);

	void TakeDamage(WeaponComponent* damager) override;

private:
	void LostTargetTime();
	Time LostTargetDuration();

    Time lastPathUpdate;

    Time startedStuck;

    std::vector<glm::vec3> path;

    AiMode mode;
	AiMode previousMode;
	Time modeStart;
	Time lostTarget;

    Entity *targetEntity;
    size_t waypointIndex;


	bool charged = false;

    void InitializeRenderBuffers();
    void UpdateRenderBuffers();
};