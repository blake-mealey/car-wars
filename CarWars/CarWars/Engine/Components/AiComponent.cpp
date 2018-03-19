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
const float AiComponent::ACCELERATION = .4f;	// bottom end of vehicle acceleration
const float AiComponent::STUCK_TIME = 2.f;		// how long until the AI realizes they are stuck
const float AiComponent::UPDATE_TIME = 1.f;		// how frequently the AI updates its mode in seconds

// all these values are modified by the max because they are changed based on their diffuculty later (easier to tune the hard AI this way)
const float AiComponent::TARGETING_RANGE = 400.f	/ MAX_DIFFUCULTY;	// the range that AI searches for targeting
const float AiComponent::LOCKON_RANGE = 300.f		/ MAX_DIFFUCULTY;	// the range that the target will shoot
const float AiComponent::LOST_TIME = .5f			/ MAX_DIFFUCULTY;	// how long until the AI no longer looks for the target, in seconds
const float AiComponent::SPRAY = 1.f				* MAX_DIFFUCULTY;	// how accurate the AI is (lower means more accurate)
const float AiComponent::STOPING_DISTANCE = 100.f	/ MAX_DIFFUCULTY;	// how close to the target the AI will get (better AI is more accurate doesn't need to be as close)
const float AiComponent::STUCK_CONTROL = 1.f		* MAX_DIFFUCULTY;	// controls the cycles of reverse and accelerate for the AI (better AI has smaller cycles)



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

	if (GetTargetEntity() &&  Game::GetDataFromEntity(GetTargetEntity())->alive) {
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

	int choice = -1;
	// TODO: select mode based on stats
	if (StateManager::gameTime.GetSeconds() > modeStart.GetSeconds() + UPDATE_TIME) { // every 2 seconds select a mode TUNEABLE
		choice = rand() % 2;

		if (0 /*doesNot have powerup (random for testing)*/ ) {
			if (choice == 0) {
				UpdateMode(AiMode_GetPowerup);
				return;
			}
		} else choice++;

		if (1 ) {
			UpdateMode(AiMode_Attack);
			return;
		} else {
			UpdateMode(mode);
		}
	}
	return;
}

void AiComponent::Update() {
	AiData* myVehicleData = static_cast<AiData*>(Game::GetDataFromEntity(GetEntity()));
	if (!enabled || !myVehicleData->alive) return;

	GameData gameData = Game::Instance().gameData;
	const int playerCount = gameData.playerCount + gameData.aiCount;
	std::vector<VehicleData*> players;

	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();
	glm::vec3 localPosition = myVehicleData->vehicleEntity->transform.GetGlobalPosition();

	PxScene* scene = &Physics::Instance().GetScene();
	PxRaycastBuffer hit;

	for (size_t i = 0; i < gameData.playerCount; ++i) { // store all vehicle data
		players.push_back(&Game::players[i]);
		
	}
	for (size_t i = 0; i < gameData.aiCount; ++i) {
		players.push_back(&Game::ais[i]);
	}

	if (vehicle) {
		float distanceToEnemy = INFINITY;
		if (mode == AiMode_Attack) {
			if (previousMode != mode || (GetTargetEntity() && !Game::GetDataFromEntity(targetEntity)->alive)) {
				float bestRating = INFINITY;
				for (VehicleData* enemyPlayer : players) {
					if ((enemyPlayer->teamIndex != myVehicleData->teamIndex) // if they are not on my team
						&& enemyPlayer->vehicleEntity						 // have a vehicle
						&& enemyPlayer->alive) {							 // are alive
						//Setup raycasting for the vehicle
						glm::vec3 enemyPosition = enemyPlayer->vehicleEntity->transform.GetGlobalPosition();
						glm::vec3 direction = enemyPosition - localPosition;
						float distanceToEnemy = glm::length(direction);
						
						if (distanceToEnemy < myVehicleData->diffuculty * TARGETING_RANGE) { //see if they are close
							direction = glm::normalize(direction);
							PxQueryFilterData filterData;
							filterData.data.word0 = (enemyPlayer->vehicleEntity->GetComponent<VehicleComponent>()->GetRaycastGroup() | RaycastGroups::GetDefaultGroup()); // only collide with enemy and terrain
							//Raycast
							if (scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(direction), distanceToEnemy, hit, PxHitFlag::eDEFAULT, filterData) &&
								EntityManager::FindEntity(hit.block.actor)->GetId() == enemyPlayer->vehicleEntity->GetId()) { // if you can see the enemy
								float rating = distanceToEnemy; // this is used to select who to attack
								
								if (rating <= bestRating) {
									if (GetTargetEntity()) {
										if (rating < bestRating || distanceToEnemy < glm::length(localPosition - targetEntity->transform.GetGlobalPosition())) { // if two are same rating. attack the closer one
											bestRating = rating;
											targetEntity = enemyPlayer->vehicleEntity;
										}
									}
								}
							}
						}
					}
				}
				if (bestRating == INFINITY) { // couldn't find a target 
					std::cout << "no target found" << std::endl;
					UpdateMode(AiMode_GetPowerup);
					return;
				}
			}
		}

		if (mode == AiMode_GetPowerup) {
			std::vector<Component*> powerupComponents = EntityManager::GetComponents(ComponentType_Vehicle); // find powerup spawns???
			float bestDistance = INFINITY;
			for (Component *component : powerupComponents) {
				if (component->GetEntity()->GetId() != GetEntity()->GetId()) {
					if (1) { /* is something there || no visibility */
						float distance = glm::length(component->GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
						if (distance < bestDistance) {
							bestDistance = distance;
							targetEntity = component->GetEntity();
						}
					}
				}
			}
			if (bestDistance == INFINITY) {
				UpdateMode(AiMode_Attack);
				return;
			}
		}

		Transform &myTransform = GetEntity()->transform;
		const glm::vec3 position = myTransform.GetGlobalPosition();
		const glm::vec3 forward = myTransform.GetForward();
		const glm::vec3 right = myTransform.GetRight();

		UpdatePath(); // Will only update every x seconds

		const glm::vec3 nodePosition = NodeInPath();

		glm::vec3 directionToNode = nodePosition - position;
		const float distanceToNode = glm::length(directionToNode);
		directionToNode = glm::normalize(directionToNode);

		NavigationMesh* navigationMesh = Game::Instance().GetNavigationMesh();

		if (distanceToNode <= navigationMesh->GetSpacing() * 2.f) {
			NextNodeInPath();
		}

		bool lineOfSight = false;
		if (mode == AiMode_Attack) {
			//Setup raycasting for the vehicle
			glm::vec3 enemyPosition = targetEntity->transform.GetGlobalPosition();
			glm::vec3 direction = enemyPosition - localPosition;
			float distanceToEnemy = glm::length(direction);
			direction = glm::normalize(direction);
			PxQueryFilterData filterData;
			filterData.data.word0 =	 (targetEntity->GetComponent<VehicleComponent>()->GetRaycastGroup() | RaycastGroups::GetDefaultGroup()); // only collide with enemy and terrain

			//Raycast
			if (scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(direction), distanceToEnemy + 1, hit, PxHitFlag::eDEFAULT, filterData) &&
				EntityManager::FindEntity(hit.block.actor)->GetId() == targetEntity->GetId()) { // if you can see the enemy
				lineOfSight = true; 
			}
			else {
				LostTargetTime();
				if (LostTargetDuration().GetSeconds() > LOST_TIME * myVehicleData->diffuculty) {
					charged = false;
					UpdateMode(AiMode_GetPowerup);
					return;
				}
			}

			WeaponComponent* weapon = GetEntity()->GetComponent<WeaponComponent>();
			if (weapon) {
				if (distanceToEnemy < LOCKON_RANGE * myVehicleData->diffuculty) {
					if (!charged) {
						weapon->Charge();
						charged = true;
					}

					glm::vec3 randomOffset = (glm::vec3(
						(float)rand() / RAND_MAX,
						(float)rand() / RAND_MAX,
						(float)rand() / RAND_MAX
					) - glm::vec3(.5f)) * (SPRAY / std::max(myVehicleData->diffuculty,.1f));

					glm::vec3 hitLocation = enemyPosition + randomOffset;
					weapon->Shoot(hitLocation);
					if (lineOfSight) lostTarget = Time(-1);
				}
				else{
					LostTargetTime();
				}
			}
		}

		/*Do Driving Stuff here*/
		const float steer = glm::dot(directionToNode, right);
		const PxReal speed = vehicle->pxVehicle->computeForwardSpeed();

		const float maxAcceleration = ACCELERATION + myVehicleData->diffuculty / MAX_DIFFUCULTY * (1 - ACCELERATION);
		float forwardPower = maxAcceleration;
		float backwardPower = 0.0f;
		if (mode == AiMode_Attack) {
			float stoppingDistance = STOPING_DISTANCE * myVehicleData->diffuculty;
			if( distanceToNode < stoppingDistance && lineOfSight) {
				backwardPower = distanceToEnemy / stoppingDistance * maxAcceleration;
				forwardPower = backwardPower;
				startedStuck = Time(-1); // dont become stuck
			}
		}
		
		if (stuck) {
			
			backwardPower = abs(std::cos(GetStuckDuration().GetSeconds() / M_PI * myVehicleData->diffuculty / STUCK_CONTROL)) * maxAcceleration;
			forwardPower = (maxAcceleration - backwardPower);
		}

		bool reverse = backwardPower > forwardPower;

		vehicle->Steer(reverse ? -steer : steer);
		vehicle->HandleAcceleration(forwardPower, backwardPower);

		if (FinishedPath()) {
			UpdatePath();
		}
		SetMode();
	}
}

void AiComponent::TakeDamage(WeaponComponent* damager) {
	if (Game::GetDataFromEntity(damager->GetEntity())->teamIndex != Game::GetDataFromEntity(GetEntity())->teamIndex) {
		mode = AiMode_Attack;
		UpdateMode(AiMode_Attack);
		startedStuck = Time(-1);
		SetTargetEntity(damager->GetEntity());
	}
}