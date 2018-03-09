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

AiComponent::~AiComponent() {
    glDeleteBuffers(1, &pathVbo);
    glDeleteVertexArrays(1, &pathVao);
}

AiComponent::AiComponent(nlohmann::json data) : targetEntity(nullptr), waypointIndex(0), lastPathUpdate(0) {
    std::string modeName = ContentManager::GetFromJson<std::string>(data["Mode"], "Waypoints");
    if (modeName == "Waypoints") {
        mode = AiMode_Waypoints;
    } else if (modeName == "Chase") {
        mode = AiMode_Chase;
    }

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
	if (startedStuck.GetTimeSeconds() == -1)	startedStuck = StateManager::gameTime;
}

Time AiComponent::GetStuckDuration() {
	return StateManager::gameTime - startedStuck;
}

void AiComponent::SetMode() {
	previousMode = mode;

	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();

	// check if stuck
	float speed = vehicle->pxVehicle->computeForwardSpeed();
	if (GetStuckDuration().GetTimeSeconds() > 1.0f) {
		mode = AiMode_Stuck;
		return;
	}
	if (abs(speed) <= 0.5f) {
		if (startedStuck.GetTimeSeconds() < 0.f) {
			StartStuckTime();
		}
	}
	else startedStuck = Time(-1);

	if (0/*doesNot have powerup*/){
		//set target powerup
		
		mode = AiMode_DriveTo;
	}
}

void AiComponent::Update() {
	if (!enabled) return;
	VehicleComponent* vehicle = GetEntity()->GetComponent<VehicleComponent>();

	Transform &myTransform = GetEntity()->transform;
	const glm::vec3 position = myTransform.GetGlobalPosition();
	const glm::vec3 forward = myTransform.GetForward();
	const glm::vec3 right = myTransform.GetRight();

	UpdatePath();       // Will only update every x seconds
	const glm::vec3 targetPosition = GetTargetEntity()->transform.GetGlobalPosition();
	const glm::vec3 nodePosition = NodeInPath();

	glm::vec3 direction = nodePosition - position;
	const float distance = glm::length(direction);
	direction = glm::normalize(direction);

	NavigationMesh* navigationMesh = Game::Instance().GetNavigationMesh();

	if (distance <= navigationMesh->GetSpacing() * 2.f) {
		NextNodeInPath();
	}

	//update mode
	SetMode();
	if (mode == AiMode_Attack){
		if (previousMode != mode) {
			std::vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
			float bestDistance = INFINITY;
			for (Component *component : vehicleComponents) {
				VehicleComponent *enemy = static_cast<VehicleComponent*>(component);
				if (enemy->GetEntity()->GetId() != GetEntity()->GetId()) {
					float distance = glm::length(enemy->GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
					if (distance < bestDistance) {
						bestDistance = distance;
						targetEntity = enemy->GetEntity();
					}
				}
			}
		}

		float distanceToEnemy = glm::length(targetEntity->transform.GetGlobalPosition() - GetEntity()->transform.GetGlobalPosition());
		if (distanceToEnemy < 30) { // if close enough to enemy shoot
			WeaponComponent* weapon = GetEntity()->GetComponent<WeaponComponent>();
			
			if (!charged) {
				weapon->Charge();
				charged = true;
			}
			//weapon->Shoot(targetPosition);
		}
		else {
			charged = false;
		}
	}/*
	case AiMode_Chase:
	case AiMode_DriveTo:
	case AiMode_Evade:
		
	case AiMode_Reverse:
	case AiMode_Stuck:

	case AiMode_Waypoints:
	case AiMode_Chase:
		const float steer = glm::dot(direction, right);
		const PxReal speed = vehicle->pxVehicle->computeForwardSpeed();
		break;
	}
	*/
	if (FinishedPath()) {
		UpdatePath();
	}
}