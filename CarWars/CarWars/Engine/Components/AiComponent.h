#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/detail/type_vec3.hpp>
#include <deque>
#include "../Systems/Time.h"
#include <GL/glew.h>

enum AiMode {
	AiMode_GetPowerup,
	AiMode_Attack
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

    Entity* GetTargetEntity() const;
    AiMode GetMode() const;
	void SetMode();

    void UpdatePath(glm::vec3 _position);
    void NextNodeInPath();
    glm::vec3 NodeInPath() const;
    bool FinishedPath() const;

	Time GetModeDuration();

	void StartStuckTime();
	Time GetStuckDuration();

	Time GetSearchDuration();

	Time GetPreppingTime();

	void StartCharge();
	Time GetChargeDuration();
	void StopCharge();

	void Update();

	void UpdateMode(AiMode _mode);

	void TakeDamage(WeaponComponent* damager, float _damage) override;

	static float MAX_DIFFICULTY;	 // this is the max AI difficulty

private:
	static float STUCK_TIME;		// how long before the AI is stuck
	static float UPDATE_TIME;		// how frequent the AI Updates its mode
	static float TARGETING_RANGE;	// the range that AI searches for targeting
	static float LOCKON_RANGE;		// the range that the target will shoot
	static float LOST_TIME;			// how long until the target is lost 
	static float SPRAY;				// how accurate the AI is (higher means more accurate)
	static float STOPING_DISTANCE;	// how close to the target the AI will get (better AI is more accurate doesn't need to be as close)
	static float STUCK_CONTROL;		// controls the cycles of reverse and accelerate for the AI (better AI has smaller cycles)
	static float ACCELERATION;		// bottom end of vehicle acceleration

	void Act();
	void Drive();
	void FindTargets();

	void LostTargetTime();
	Time LostTargetDuration();

	bool GetLineOfSight(glm::vec3 _position);

    std::vector<glm::vec3> path;

    AiMode mode;
	AiMode previousMode;

    Entity* vehicleEntity;
	Entity* powerupEntity;

	Time modeStartTime;
	Time lostTargetTime;
	Time lastPathUpdate;
	Time startedStuckTime;
	Time lastSearchTime;
	Time chargeStartTime;

	bool cooldown = false;

	bool stuck = false;

	float distanceToTarget;
	bool lineOfSight;

    void InitializeRenderBuffers();
    void UpdateRenderBuffers();
};