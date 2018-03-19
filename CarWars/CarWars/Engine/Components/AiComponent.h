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

	static const float MAX_DIFFUCULTY;	 // this is the max AI diffuculty

private:
	static const float STUCK_TIME;
	static const float UPDATE_TIME;
	static const float TARGETING_RANGE;	// the range that AI searches for targeting
	static const float LOCKON_RANGE;	// the range that the target will shoot
	static const float LOST_TIME;		// how long until the target is lost 
	static const float SPRAY;			// how accurate the AI is (higher means more accurate)
	static const float STOPING_DISTANCE;// how close to the target the AI will get (better AI is more accurate doesn't need to be as close)
	static const float STUCK_CONTROL;	// controls the cycles of reverse and accelerate for the AI (better AI has smaller cycles)
	static const float ACCELERATION;	// bottom end of vehicle acceleration

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
	bool stuck;

    void InitializeRenderBuffers();
    void UpdateRenderBuffers();
};