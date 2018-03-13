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
    if (!FinishedPath() && StateManager::gameTime - lastPathUpdate < 0.01f) return;
    lastPathUpdate = StateManager::gameTime;

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
	if (startedStuck.GetTimeSeconds() < 0) startedStuck = StateManager::gameTime;
}

Time AiComponent::GetStuckDuration() {
	if (startedStuck.GetTimeSeconds() == -1) return -1;
	return StateManager::gameTime - startedStuck;
}


void AiComponent::LostTargetTime() {
	if (lostTarget.GetTimeSeconds() < 0) lostTarget = StateManager::gameTime;
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
	if (abs(speed) <= 0.5f) {
		StartStuckTime();
	}
	else startedStuck = Time(-1);

	// check if stuck
	if (GetStuckDuration().GetTimeSeconds() > 1.0f) {
		UpdateMode(AiMode_Stuck);
		return;
	}

	int choice = -1;
	if (StateManager::gameTime.GetTimeSeconds() > modeStart.GetTimeSeconds() + 10) { // every ten seconds select a mode TUNEABLE
		choice = rand() % 2;

		if (rand() % 1/*doesNot have powerup (random for testing)*/ ) {
			if (choice == 0) {
				UpdateMode(AiMode_GetPowerup);
				return;
			}
		} else choice++;

		if (choice == 1) {
			UpdateMode(AiMode_Attack);
		} else {
			UpdateMode(mode);
		}
	}
	return;
}

void AiComponent::Update() {
	if (!enabled) return;

	PxScene* scene = &Physics::Instance().GetScene();
	PxRaycastBuffer hit;
	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();

	if (vehicle) {
		float distanceToEnemy = INFINITY;
		glm::vec3 localPosition = GetEntity()->transform.GetGlobalPosition();

		if (mode == AiMode_Attack) {
			if (previousMode != mode || targetEntity == nullptr) { // target entity is still alive??
				std::vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				float bestRating = INFINITY;
				for (Component *component : vehicleComponents) {
					VehicleComponent *enemy = static_cast<VehicleComponent*>(component);
					if (enemy->GetEntity()->GetId() != GetEntity()->GetId()) {
						//Setup raycasting for the vehicle
						glm::vec3 enemyPosition = enemy->GetEntity()->transform.GetGlobalPosition();
						glm::vec3 direction = enemyPosition - localPosition;
						float distance = glm::length(direction);

						if (distance < 50) { // search radius for the vehicle
							direction = glm::normalize(direction);
							PxQueryFilterData filterData;
							//filterData.data.word0 = -1 ^ (targetEntity->GetComponent<VehicleComponent>()->GetRaycastGroup() | RaycastGroups::GetDefaultGroup()); //filter all cars but enemy
							//Raycast
							//if (scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(direction), distance, hit, PxHitFlag::eDEFAULT, filterData) &&
							//	EntityManager::FindEntity(hit.block.actor)->GetId() == enemy->GetEntity()->GetId()) { // if you can see the enemy
								float rating = distance * enemy->GetHealth(); // this is used to select who to attack
								if (rating <= bestRating) {
									if (rating < bestRating || distance < glm::length(enemyPosition - targetEntity->transform.GetGlobalPosition())) { // if two are same rating. attack the closer one
										bestRating = rating;
										targetEntity = enemy->GetEntity();
									}
								}
							//}
						}
					}
				}
				if (bestRating == INFINITY) { // couldn't find a target 
					std::cout << "no target found" << std::endl;
					UpdateMode(AiMode_GetPowerup);
					return;
				}
			}

			distanceToEnemy = glm::length(targetEntity->transform.GetGlobalPosition() - targetEntity->transform.GetGlobalPosition());
			//Setup raycasting for the vehicle
			glm::vec3 enemyPosition = targetEntity->transform.GetGlobalPosition();
			glm::vec3 direction = enemyPosition - localPosition;
			float distance = glm::length(direction);
			direction = glm::normalize(direction);
			PxQueryFilterData filterData;
			filterData.data.word0 =  -1 ^ (targetEntity->GetComponent<VehicleComponent>()->GetRaycastGroup() | RaycastGroups::GetDefaultGroup());

			//Raycast
			if (1 || scene->raycast(Transform::ToPx(localPosition), Transform::ToPx(direction), distance, hit, PxHitFlag::eDEFAULT, filterData) &&
				EntityManager::FindEntity(hit.block.actor)->GetId() == targetEntity->GetId()) { // if you can see the enemy
				lostTarget = Time(-1); // target is not lost 
			}
			else {
				LostTargetTime();
				if (LostTargetDuration().GetTimeSeconds() > 0.5f) { //if out of sight for so long stop shooting. Time is TUNEABLE
					charged = false;
					UpdateMode(AiMode_GetPowerup);
					return;
				}
			}
			WeaponComponent* weapon = GetEntity()->GetComponent<WeaponComponent>();
			if (weapon) {
				if (distanceToEnemy < 50) { // if close enough to enemy shoot TUNEABLE
					// aim weapon TUNEABLE
					if (!charged) {
						weapon->Charge();
						charged = true;
					}
					glm::vec3 hitLocation = enemyPosition; // + random amounts for targeting
						//+ glm::vec3(((float)(rand() % 200) / 100.f) - 1, ((float)(rand() % 200) / 100.f) - 1, ((float)(rand() % 200) / 100.f) - 1); // randomize for tageting
					weapon->Shoot(hitLocation);
				}
				else {
					LostTargetTime();
				}
			}
		}

		if (mode == AiMode_GetPowerup) { // target entiy exists??
			std::vector<Component*> powerupComponents = EntityManager::GetComponents(ComponentType_Vehicle); // find powerup spawns???
			float bestDistance = INFINITY;
			for (Component *component : powerupComponents) {
				if (component->GetEntity()->GetId() != GetEntity()->GetId()) {
					if (1) { /*is something there || no visibility */
						float distance = glm::length(component->GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
						if (distance < bestDistance) {
							bestDistance = distance;
							targetEntity = component->GetEntity();
						}
					}
				}
				if (bestDistance == INFINITY) {
					UpdateMode(AiMode_Attack);
					return;
				}
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

		/*Do Driving Stuff here*/
		const float steer = glm::dot(directionToNode, right);
		const PxReal speed = vehicle->pxVehicle->computeForwardSpeed();

		const bool reverse = 0 && (((int)GetStuckDuration().GetTimeSeconds() + 1) % 5) > 2; // 5 second loop 3 in reverse 2 in forward
		
		const float accel = 0.8f;

		if (!reverse && vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eREVERSE) {
			vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		}
		else if (reverse && vehicle->pxVehicle->mDriveDynData.getCurrentGear() != PxVehicleGearsData::eREVERSE) {
			vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
		}
		vehicle->pxVehicleInputData.setAnalogSteer(reverse ? -steer : steer);
		vehicle->pxVehicleInputData.setAnalogAccel(accel);

		if (FinishedPath()) {
			UpdatePath();
		}
		SetMode();
	}
}