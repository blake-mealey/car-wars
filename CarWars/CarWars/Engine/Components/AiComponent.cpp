#include "AiComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Systems/Pathfinder.h"
#include "../Systems/Game.h"
#include "../Systems/StateManager.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Components/WeaponComponents/WeaponComponent.h"
#include "../Components/RigidbodyComponents/PowerUpSpawnerComponent.h"

#include "../Systems/Physics.h"

#include "../Entities/EntityManager.h"

float AiComponent::MAX_DIFFICULTY;
float AiComponent::STUCK_TIME;
float AiComponent::UPDATE_TIME;
float AiComponent::TARGETING_RANGE;
float AiComponent::LOCKON_RANGE;
float AiComponent::LOST_TIME;
float AiComponent::SPRAY;
float AiComponent::STOPING_DISTANCE;
float AiComponent::STUCK_CONTROL;
float AiComponent::ACCELERATION;



AiComponent::~AiComponent() {
    glDeleteBuffers(1, &pathVbo);
    glDeleteVertexArrays(1, &pathVao);
}

AiComponent::AiComponent(nlohmann::json data) : vehicleEntity(nullptr), powerupEntity(nullptr), lastPathUpdate(0) {
	MAX_DIFFICULTY = Game::gameData.MAX_AI_DIFFICULTY;
	ACCELERATION = ContentManager::GetFromJson<float>(data["Acceleration"], .5f);
	STUCK_TIME = ContentManager::GetFromJson<float>(data["StuckTime"], 2.f);
	UPDATE_TIME = ContentManager::GetFromJson<float>(data["UpdateTime"], 2.f);
	STUCK_CONTROL = ContentManager::GetFromJson<float>(data["StuckControl"], 1.5f);
	TARGETING_RANGE = ContentManager::GetFromJson<float>(data["TargetingRange"], 500.f) / MAX_DIFFICULTY;
	LOCKON_RANGE = ContentManager::GetFromJson<float>(data["LockonRange"], 400.f) / MAX_DIFFICULTY;
	LOST_TIME = ContentManager::GetFromJson<float>(data["LostTime"], 5.f) / MAX_DIFFICULTY;
	SPRAY = ContentManager::GetFromJson<float>(data[ "Spray"], 1.5f) * MAX_DIFFICULTY;
	STOPING_DISTANCE = ContentManager::GetFromJson<float>(data[	"StoppingDistance"], 100.f) / MAX_DIFFICULTY;

	mode = AiMode_GetPowerup;
	UpdateMode(AiMode_Attack);

	startedStuckTime = Time(-1);
	lostTargetTime = Time(-1);
	modeStartTime = Time(-UPDATE_TIME);
	lastSearchTime = Time(-UPDATE_TIME);

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

Entity* AiComponent::GetTargetEntity() const {
	if (mode == AiMode_Attack && vehicleEntity) return vehicleEntity;
	if (mode == AiMode_GetPowerup && powerupEntity) return powerupEntity;
	return nullptr;
}

AiMode AiComponent::GetMode() const {
    return mode;
}

void AiComponent::UpdatePath(glm::vec3 _targetPosition) {
    if (!FinishedPath() && (StateManager::gameTime - lastPathUpdate).GetSeconds() < UPDATE_TIME / 5) return;
    lastPathUpdate = StateManager::gameTime;

	const glm::vec3 currentPosition = GetEntity()->transform.GetGlobalPosition();
	const glm::vec3 targetPosition = _targetPosition;
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

void AiComponent::NextNodeInPath() {
    if (FinishedPath()) return;
    path.pop_back();
}

glm::vec3 AiComponent::NodeInPath() const {
	if (FinishedPath()) {
		if(mode == AiMode_Attack && vehicleEntity) return vehicleEntity->transform.GetGlobalPosition();
		if (mode == AiMode_GetPowerup && powerupEntity) return powerupEntity->transform.GetGlobalPosition();
		return glm::vec3();
	}
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
	return StateManager::gameTime - modeStartTime;
}

void AiComponent::StartStuckTime() {
	if (startedStuckTime.GetSeconds() < 0) startedStuckTime = StateManager::gameTime;
}

Time AiComponent::GetStuckDuration() {
	if (startedStuckTime.GetSeconds() < 0) return -1;
	return StateManager::gameTime - startedStuckTime;
}


void AiComponent::LostTargetTime() {
	if (lostTargetTime.GetSeconds() < 0) lostTargetTime = StateManager::gameTime;
}

Time AiComponent::LostTargetDuration() {
	if (lostTargetTime.GetSeconds() < 0) return -1;
	return StateManager::gameTime - lostTargetTime;
}


void AiComponent::UpdateMode(AiMode _mode) {
	previousMode = mode;
	mode = _mode;
	if (mode != previousMode) modeStartTime = StateManager::gameTime;
}


void AiComponent::SetMode() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return;

	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();
	float speed = vehicle->pxVehicle->computeForwardSpeed();

	//detect being stuck
	if (abs(speed) <= 1.f && !vehicle->inAir) {
		StartStuckTime();
	}

	else if (speed > 1.f) {
		startedStuckTime = Time(-1);
	}
	// check if stuck
	if (GetStuckDuration().GetSeconds() > STUCK_TIME) {
		stuck = true;
		return;
	}
	else {
		stuck = false;
	}

	if (GetModeDuration().GetSeconds() > UPDATE_TIME) {
		UpdateMode(AiMode_Attack);
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

	PlayerData* enemyData = Game::GetPlayerFromEntity(vehicleEntity);
	glm::vec3 driveTo;
	if (enemyData && enemyData->alive && mode == AiMode_Attack) driveTo = vehicleEntity->transform.GetGlobalPosition();
	else if (powerupEntity) driveTo = powerupEntity->transform.GetGlobalPosition();
	else driveTo = glm::vec3();

	const float maxAcceleration = ACCELERATION + myData->difficulty / MAX_DIFFICULTY * (1 - ACCELERATION);

	float forwardPower = maxAcceleration;
	float backwardPower = 0.0f;

	if (mode == AiMode_Attack) {
		float stoppingDistance = STOPING_DISTANCE * myData->difficulty;
		if ((distanceToTarget < stoppingDistance) && lineOfSight) {
			if (powerupEntity) {
				driveTo = powerupEntity->transform.GetGlobalPosition();
			} else {
				backwardPower = distanceToTarget / stoppingDistance * maxAcceleration;
				forwardPower = backwardPower;
				startedStuckTime = Time(-1); // dont become stuck
			}
		}
	}

	glm::vec3 boostDir = glm::vec3(0);
	// try to unstick yourself
	if (stuck) {
		backwardPower = abs(std::cos(GetStuckDuration().GetSeconds() / M_PI / STUCK_CONTROL)) * maxAcceleration;
		forwardPower = (maxAcceleration - backwardPower);
		if (GetStuckDuration().GetSeconds() > 5 * STUCK_CONTROL) {
			boostDir = -myTransform.GetUp() * ((float)rand() / RAND_MAX) + myTransform.GetRight() * ((float)rand() / RAND_MAX - .5f);
		}
	}
	UpdatePath(driveTo); // Will only update every x seconds

	NavigationMesh* navigationMesh = Game::Instance().GetNavigationMesh();
	glm::vec3 nodePosition = NodeInPath();
	glm::vec3 directionToNode = nodePosition - position;
	float distanceToNode = glm::length(directionToNode);
	directionToNode = glm::normalize(directionToNode);

	if (distanceToNode <= (navigationMesh->GetSpacing() * 2.f)) {
		NextNodeInPath();
	}

	bool reverse = backwardPower > forwardPower;
	const float steer = glm::dot(directionToNode, right);

	vehicle->Boost(boostDir);
	vehicle->Steer(reverse ? -steer : steer);
	vehicle->HandleAcceleration(forwardPower, backwardPower);

	if (FinishedPath()) {
		UpdatePath(driveTo);
	}
}

bool AiComponent::GetLineOfSight(glm::vec3 _position) {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return false;

	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();
	PxScene* scene = &Physics::Instance().GetScene();
	PxRaycastBuffer hit;

	bool sight = false;
	glm::vec3 directionToTarget = _position - localPosition;
	float distanceToTarget = glm::length(directionToTarget);
	directionToTarget = glm::normalize(directionToTarget);

	PxQueryFilterData filterData;
	filterData.data.word0 = -1 ^ GetEntity()->GetComponent<VehicleComponent>()->GetRaycastGroup();  //TODO: remove powerups as well
	VehicleComponent* enemyVehicleComponent = vehicleEntity->GetComponent<VehicleComponent>();
	if (enemyVehicleComponent) filterData.data.word0 ^= enemyVehicleComponent->GetRaycastGroup();

	//Raycast for line of sight
	scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(directionToTarget), distanceToTarget, hit, PxHitFlag::eDEFAULT, filterData);
	if (!hit.hasAnyHits()){ // if you hit nothing then you have line of sight
		sight = true;
	}

	return sight;
}

Time AiComponent::GetSearchDuration() {
	return StateManager::gameTime - lastSearchTime;
}


void AiComponent::FindTargets() {
	if (GetSearchDuration().GetSeconds() < UPDATE_TIME * 3) return;

	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return;

	PlayerData* enemyData = Game::GetPlayerFromEntity(vehicleEntity);
	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();

	bool foundTarget = false;

	GameData gameData = Game::Instance().gameData;
	std::vector<PlayerData*> players;

	if (!(GetTargetEntity() && enemyData && enemyData->alive)) { // dont always scan to look for enemy stay locked on one
		// get players
		for (size_t i = 0; i < 4; ++i) {
            HumanData& player = Game::humanPlayers[i];
            if (!player.ready) continue;
			players.push_back(&player);
		}
		for (size_t i = 0; i < gameData.aiCount; ++i) {
			players.push_back(&Game::aiPlayers[i]);
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
				if (distanceToEnemy < (myData->difficulty * TARGETING_RANGE)) { //see if they are in targeting range
					//set target to entity and check line of sight
					vehicleEntity = enemyPlayer->vehicleEntity;
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
			vehicleEntity = nullptr;
		}
		else {
			vehicleEntity = bestTarget;
		}
	}


	//find powerup
	//TODO: pick a better one that is on your way to the vehicle target
	std::vector<Component*> powerupComponents = EntityManager::GetComponents(ComponentType_PowerUpSpawner);
	float bestDistance = INFINITY;
	for (Component* component : powerupComponents) {
		PowerUpSpawnerComponent* powerup = static_cast<PowerUpSpawnerComponent*>(component);
		if (powerup->GetEntity()->GetId() != GetEntity()->GetId()) {
			if (powerup->enabled && powerup->HasActivePowerup()) {
				float distance = glm::length(powerup->GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
				if (distance < bestDistance) {
					bestDistance = distance;
					powerupEntity = powerup->GetEntity();
				}
			}
		}
	}
	if (bestDistance == INFINITY) {
		UpdateMode(AiMode_Attack);
		powerupEntity = nullptr;
	}

	lastSearchTime = StateManager::gameTime;
}


void AiComponent::Act() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!enabled || !myData->alive) return;

	PlayerData* enemyData = Game::GetPlayerFromEntity(vehicleEntity);
	glm::vec3 localPosition = myData->vehicleEntity->transform.GetGlobalPosition();

	// Shooting stuff
	if (mode == AiMode_Attack && enemyData && enemyData->alive) {
		glm::vec3 vehicleTargetPosition = vehicleEntity->transform.GetGlobalPosition();
		distanceToTarget = glm::length(vehicleTargetPosition - localPosition);
		lineOfSight = GetLineOfSight(vehicleTargetPosition);
		WeaponComponent* weapon = GetEntity()->GetComponent<WeaponComponent>();

		if (lineOfSight && distanceToTarget < (TARGETING_RANGE * myData->difficulty)) lostTargetTime = Time(-1);
		if (!lineOfSight) LostTargetTime();

		if (weapon) {
			if (distanceToTarget < (LOCKON_RANGE * myData->difficulty)) {
				if (!charged) {
					weapon->Charge();
					charged = true;
				}

				float randomHorizontalAngle = (float)rand() / (float)RAND_MAX * M_PI * 2.f;
				float randomVerticalAngle = (float)rand() / (float)RAND_MAX * M_PI;

				// pick a random point on a sphere for spray
				glm::vec3 randomOffset = (glm::vec3(cos(randomHorizontalAngle) * sin(randomVerticalAngle),
					cos(randomVerticalAngle),
					sin(randomHorizontalAngle) * sin(randomVerticalAngle)) - glm::vec3(.5f)) * (SPRAY / std::max(myData->difficulty, .1f));

				glm::vec3 hitLocation = vehicleTargetPosition + randomOffset + (myData->weaponType == WeaponType::RocketLauncher ? -vehicleEntity->transform.GetForward() + glm::vec3(0.f, -1.f, 0.f) : glm::vec3(0.f));
				weapon->Shoot(hitLocation);
			}
			else {
				LostTargetTime();
			}
		}
		if (LostTargetDuration().GetSeconds() > (LOST_TIME * myData->difficulty)) {
			UpdateMode(AiMode_GetPowerup);
		}
	}
}

void AiComponent::Update() {
	AiData* myData = static_cast<AiData*>(Game::GetPlayerFromEntity(GetEntity()));
	if (!(enabled &&  myData && myData->alive)) return;

	//std::cout << "target: " << vehicleEntity << "\npowerup: " << powerupEntity << "\nmode: " << mode << "\nline of sight: " << lineOfSight << "\nmode time: " << GetModeDuration().GetSeconds() << "\nstuck time: " << GetStuckDuration().GetSeconds() << "\ntime since search: " << GetSearchDuration().GetSeconds() << std::endl;

	FindTargets(); //finds targets every update time
	Act();
	Drive();

	SetMode();
}

void AiComponent::TakeDamage(WeaponComponent* damager, float damage) {
	if (!damager->GetEntity()) return;
	if (Game::GetPlayerFromEntity(damager->GetEntity())->teamIndex != Game::GetPlayerFromEntity(GetEntity())->teamIndex) {
		mode = AiMode_Attack;
		UpdateMode(AiMode_Attack);
		vehicleEntity = damager->GetEntity();
	}
}