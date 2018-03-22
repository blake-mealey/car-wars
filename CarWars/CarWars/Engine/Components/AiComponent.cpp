#include "AiComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Systems/Pathfinder.h"
#include "../Systems/Game.h"
#include <iostream>
#include "../Systems/StateManager.h"
#include "../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Components/WeaponComponents/WeaponComponent.h"
#include "../Systems/Physics.h"
#include "../Systems/Physics/RaycastGroups.h"

#include <iostream>



const float AiComponent::MAX_DIFFUCULTY = 10.f;	// this is the max AI diffuculty
const float AiComponent::ACCELERATION = .5f;	// bottom end of vehicle acceleration
const float AiComponent::STUCK_TIME = 2.f;		// how long until the AI realizes they are stuck
const float AiComponent::UPDATE_TIME = 2.f;		// how frequently the AI updates its mode in seconds
const float AiComponent::STUCK_CONTROL = 1.5f;	// controls the cycles of reverse and accelerate for the AI (better AI has smaller cycles)

// all these values are modified by the max because they are changed based on their diffuculty later (easier to tune the hard AI this way)
const float AiComponent::TARGETING_RANGE = 500.f	/ MAX_DIFFUCULTY;	// the range that AI searches for targeting
const float AiComponent::LOCKON_RANGE = 400.f		/ MAX_DIFFUCULTY;	// the range that the target will shoot
const float AiComponent::LOST_TIME = 1.f			/ MAX_DIFFUCULTY;	// how long until the AI no longer looks for the target, in seconds
const float AiComponent::SPRAY = 1.5f				* MAX_DIFFUCULTY;	// how accurate the AI is (lower means more accurate)
const float AiComponent::STOPING_DISTANCE = 100.f	/ MAX_DIFFUCULTY;	// how close to the target the AI will get (better AI is more accurate doesn't need to be as close)


AiComponent::~AiComponent() {
    glDeleteBuffers(1, &pathVbo);
    glDeleteVertexArrays(1, &pathVao);
}

AiComponent::AiComponent(nlohmann::json data) : targetEntity(nullptr), waypointIndex(0), lastPathUpdate(0) {
	mode = AiMode_Waypoints;
    std::string modeName = ContentManager::GetFromJson<std::string>(data["Mode"], "Waypoints");
    if (modeName == "Waypoints") {
        UpdateMode(AiMode_Waypoints);
    } else if (modeName == "Chase") {
        UpdateMode(AiMode_Chase);
    }
	startedStuck = Time(-1);
	UpdateMode(AiMode_Attack);

    InitializeRenderBuffers();
}

size_t AiComponent::GetPathLength() const {
    return path.size();
}

ComponentType AiComponent::GetType() {
    return ComponentType_AI;
}

void AiComponent::HandleEvent(Event* event) { }

void AiComponent::RenderDebugGui() {
    Component::RenderDebugGui();
}

void AiComponent::SetTargetEntity(Entity* target) {
    targetEntity = target;
}

Entity* AiComponent::GetTargetEntity() const {
    return targetEntity;
}

AiMode AiComponent::GetMode() const {
    return mode;
}

void AiComponent::UpdatePath() {
    if (!FinishedPath() && StateManager::gameTime - lastPathUpdate < 0.01) return;
    lastPathUpdate = StateManager::gameTime;

	if (GetTargetEntity() &&  Game::GetPlayerFromEntity(GetTargetEntity())->alive) {
		const glm::vec3 currentPosition = GetEntity()->transform.GetGlobalPosition();
		const glm::vec3 targetPosition = GetTargetEntity()->transform.GetGlobalPosition();
		const glm::vec3 offsetDirection = normalize(-GetEntity()->transform.GetForward() * 1.f + normalize(targetPosition - currentPosition));
		//    const glm::vec3 offsetDirection = -GetEntity()->transform.GetForward();
		auto newPath = Pathfinder::FindPath(
			Game::Instance().GetNavigationMesh(),
			currentPosition + offsetDirection * Game::Instance().GetNavigationMesh()->GetSpacing() * 2.f,
			targetPosition);

		if (!newPath.empty() || FinishedPath()) {
			path = newPath;
			UpdateRenderBuffers();
		}
	}
}

void AiComponent::NextNodeInPath() {
    if (FinishedPath()) return;
    path.pop_back();
}

glm::vec3 AiComponent::NodeInPath() const {
    if (FinishedPath()) return targetEntity->transform.GetGlobalPosition();
    return path.back();
}

bool AiComponent::FinishedPath() const {
    return path.size() == 0;
}

void AiComponent::InitializeRenderBuffers() {
    glGenBuffers(1, &pathVbo);
    UpdateRenderBuffers();

    glGenVertexArrays(1, &pathVao);
    glBindVertexArray(pathVao);
    glBindBuffer(GL_ARRAY_BUFFER, pathVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));        // position

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AiComponent::UpdateRenderBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, pathVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * path.size(), path.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Time AiComponent::GetModeDuration() {
	return StateManager::gameTime - modeStart;
}

void AiComponent::StartStuckTime() {
	if (startedStuck.GetSeconds() < 0) startedStuck = StateManager::gameTime;
}

Time AiComponent::GetStuckDuration() {
	if (startedStuck.GetSeconds() == -1) return -1;
	return StateManager::gameTime - startedStuck;
}


void AiComponent::LostTargetTime() {
	if (lostTarget.GetSeconds() < 0) lostTarget = StateManager::gameTime;
}

Time AiComponent::LostTargetDuration() {
	return StateManager::gameTime - lostTarget;
}


void AiComponent::UpdateMode(AiMode _mode) {
	previousMode = mode;
	mode = _mode;
	if (mode != previousMode) modeStart = StateManager::gameTime;
}


void AiComponent::SetMode() {
	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();
	float speed = vehicle->pxVehicle->computeForwardSpeed();

	//detect being stuck
	if (abs(speed) <= 1.f) {
		StartStuckTime();
	}

	else if (speed > 1.f) {
		startedStuck = Time(-1);
	}
	// check if stuck
	if (GetStuckDuration().GetSeconds() > STUCK_TIME) {
		stuck = true;
		return;
	}
	else {
		stuck = false;
	}

	// TODO: select mode based on stats
	if (StateManager::gameTime.GetSeconds() > modeStart.GetSeconds() + UPDATE_TIME) { // every 2 seconds select a mode TUNEABLE
		if (0) {
			UpdateMode(AiMode_GetPowerup);
			return;
		}
		if (1) {
			UpdateMode(AiMode_Attack);
			return;
		} else {
			UpdateMode(mode);
		}
	}
	return;
}

void AiComponent::Drive() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return;

	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();

	// Driving Stuff
	Transform &myTransform = GetEntity()->transform;
	const glm::vec3 position = myTransform.GetGlobalPosition();
	const glm::vec3 forward = myTransform.GetForward();
	const glm::vec3 right = myTransform.GetRight();

	UpdatePath(); // Will only update every x seconds

	glm::vec3 nodePosition = NodeInPath();
	glm::vec3 directionToNode = nodePosition - position;
	float distanceToNode = glm::length(directionToNode);
	directionToNode = glm::normalize(directionToNode);

	NavigationMesh* navigationMesh = Game::Instance().GetNavigationMesh();

	if (distanceToNode <= navigationMesh->GetSpacing() * 2.f) {
		nodePosition = NodeInPath();
	}

	const float steer = glm::dot(directionToNode, right);

	const float maxAcceleration = ACCELERATION + myData->diffuculty / MAX_DIFFUCULTY * (1 - ACCELERATION);

	float forwardPower = maxAcceleration;
	float backwardPower = 0.0f;

	//stopping rage control
	if (mode == AiMode_Attack) {
		float stoppingDistance = STOPING_DISTANCE * myData->diffuculty;
		if ((distanceToTarget < stoppingDistance) && lineOfSight) {
			backwardPower = distanceToTarget / stoppingDistance * maxAcceleration;
			forwardPower = backwardPower;
			startedStuck = Time(-1); // dont become stuck
		}
	}

	glm::vec3 boostDir = glm::vec3(0);
	// try to unstick yourself
	if (stuck) {
		backwardPower = abs(std::cos(GetStuckDuration().GetSeconds() / M_PI / STUCK_CONTROL)) * maxAcceleration;
		forwardPower = (maxAcceleration - backwardPower);
		if (GetStuckDuration().GetSeconds() > 4 * STUCK_CONTROL) {
			boostDir = -myTransform.GetUp() * ((float)rand() / RAND_MAX) + myTransform.GetRight() * ((float)rand() / RAND_MAX - .5f);
		}
	}

	bool reverse = backwardPower > forwardPower;

	vehicle->Boost(boostDir);
	vehicle->Steer(reverse ? -steer : steer);
	vehicle->HandleAcceleration(forwardPower, backwardPower);

	if (FinishedPath()) {
		UpdatePath();
	}
}



bool AiComponent::GetLineOfSight(glm::vec3 _position) {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));

	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();
	PxScene* scene = &Physics::Instance().GetScene();
	PxRaycastBuffer hit;

	bool sight = false;
	glm::vec3 directionToTarget = _position - localPosition;
	float distanceToTarget = glm::length(directionToTarget);
	directionToTarget = glm::normalize(directionToTarget);

	PxQueryFilterData filterData;
	filterData.data.word0 = -1 ^ GetEntity()->GetComponent<VehicleComponent>()->GetRaycastGroup();
	VehicleComponent* enemyVehicleComponent = targetEntity->GetComponent<VehicleComponent>();
	if (enemyVehicleComponent) filterData.data.word0 ^= enemyVehicleComponent->GetRaycastGroup();

	//naive Raycast for line of sight
	scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(directionToTarget), distanceToTarget, hit, PxHitFlag::eDEFAULT, filterData);
	if (!hit.hasAnyHits()){ // if you hit nothing then you have line or sight
		sight = true;
	}

	return sight;
}

bool AiComponent::FindTarget() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();

	bool foundTarget = false;

	if (mode == AiMode_Attack) {
		GameData gameData = Game::Instance().gameData;
		const int playerCount = gameData.humanCount + gameData.aiCount;
		std::vector<PlayerData*> players;

		if (previousMode != mode || (GetTargetEntity() && !Game::GetPlayerFromEntity(targetEntity)->alive)) {
			// get players
			for (size_t i = 0; i < gameData.humanCount; ++i) {
				players.push_back(&Game::humanPlayers[i]);
			}
			for (size_t i = 0; i < gameData.aiCount; ++i) {
				players.push_back(&Game::ais[i]);
			}

			//find attack target
			float bestRating = INFINITY;
			Entity* bestTarget = nullptr;

			for (PlayerData* enemyPlayer : players) {
				if ((enemyPlayer->teamIndex != myData->teamIndex) // if they are not on my team
					&& enemyPlayer->vehicleEntity						 // have a vehicle
					&& enemyPlayer->alive) {							 // are alive
					glm::vec3 enemyPosition = enemyPlayer->vehicleEntity->transform.GetGlobalPosition();
					float distanceToEnemy = glm::length(enemyPosition - localPosition);
					if (distanceToEnemy < myData->diffuculty * TARGETING_RANGE) { //see if they are in targeting range
						//set target to entity and check line of sight
						targetEntity = enemyPlayer->vehicleEntity;
						bool sightOnTarget = GetLineOfSight(enemyPosition);
						if (sightOnTarget) {
							float rating = distanceToEnemy; // this is used to select who to attack
							if (rating <= bestRating) {
								if (rating < bestRating || distanceToEnemy < glm::length(localPosition - bestTarget->transform.GetGlobalPosition())) { // if two are same rating. attack the closer one
									bestRating = rating;
									bestTarget = enemyPlayer->vehicleEntity;
								}
							}
						}
					}
				}
			}
			if (bestRating == INFINITY) { // couldn't find a target 
				UpdateMode(AiMode_GetPowerup);
				lineOfSight = false;
			}
			else {
				targetEntity = bestTarget;
				lineOfSight = true;
				foundTarget = true;
			}
		}
		else {
			foundTarget = true;
		}
	}

	//find powerup
	if (mode == AiMode_GetPowerup) {
		std::vector<Component*> powerupComponents = EntityManager::GetComponents(ComponentType_Vehicle); // find powerup spawn locations
		float bestDistance = INFINITY;
		for (Component* powerup : powerupComponents) {
			if (powerup->GetEntity()->GetId() != GetEntity()->GetId()) {
				if (1) { /*is something there */
					float distance = glm::length(powerup->GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
					if (distance < bestDistance) {
						bestDistance = distance;
						targetEntity = powerup->GetEntity();
					}
				}
			}
		}
		if (bestDistance == INFINITY) {
			UpdateMode(AiMode_Attack);
		}
		else {
			foundTarget = true;
		}
	}
	return foundTarget;
}


void AiComponent::Shoot() {
	// Shooting stuff
	if (mode == AiMode_Attack) {
		AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
		glm::vec3 targetPosition = targetEntity->transform.GetGlobalPosition();
		WeaponComponent* weapon = GetEntity()->GetComponent<WeaponComponent>();
		if (weapon) {
			if (distanceToTarget < LOCKON_RANGE * myData->diffuculty) {
				if (!charged) {
					weapon->Charge();
					charged = true;
				}

				float randomHorizontalAngle = (float)rand() / (float)RAND_MAX * M_PI * 2.f;
				float randomVerticalAngle = (float)rand() / (float)RAND_MAX * M_PI;

				// pick a random point on a sphere for spray
				glm::vec3 randomOffset = (glm::vec3(cos(randomHorizontalAngle) * sin(randomVerticalAngle),
					cos(randomVerticalAngle),
					sin(randomHorizontalAngle) * sin(randomVerticalAngle)) - glm::vec3(.5f)) * (SPRAY / std::max(myData->diffuculty, .1f));

				glm::vec3 hitLocation = targetPosition + randomOffset;
				weapon->Shoot(hitLocation);
			}
			else {
				LostTargetTime();
			}
		}
		if (lineOfSight && distanceToTarget < TARGETING_RANGE * myData->diffuculty) lostTarget = Time(-1);
	}
}

void AiComponent::Update() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return;

	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();
	for (size_t i = 0; i < 2 && !FindTarget(); ++i); // try finding a target 2x
	if (targetEntity) {
		glm::vec3 targetPostion = targetEntity->transform.GetGlobalPosition();
		distanceToTarget = glm::length(targetPostion - localPosition);
		lineOfSight = GetLineOfSight(targetPostion);
		Shoot();
		Drive();
	}
	SetMode();
}

void AiComponent::TakeDamage(WeaponComponent* damager, float damage) {
	if (Game::GetPlayerFromEntity(damager->GetEntity())->teamIndex != Game::GetPlayerFromEntity(GetEntity())->teamIndex) {
		mode = AiMode_Attack;
		UpdateMode(AiMode_Attack);
		SetTargetEntity(damager->GetEntity());
	}
}