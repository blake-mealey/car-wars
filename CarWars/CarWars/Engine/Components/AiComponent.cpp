#include "AiComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Systems/Pathfinder.h"
#include "../Systems/Game.h"
#include <iostream>

AiComponent::AiComponent(nlohmann::json data) : targetEntity(nullptr), waypointIndex(0) {
    std::string modeName = ContentManager::GetFromJson<std::string>(data["Mode"], "Waypoints");
    if (modeName == "Waypoints") {
        mode = AiMode_Waypoints;
    } else if (modeName == "Chase") {
        mode = AiMode_Chase;
    }
    marker = ContentManager::LoadEntity("Marker.json");
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

size_t AiComponent::GetWaypoint() const {
    return waypointIndex;
}

void AiComponent::UpdatePath() {
    std::deque<glm::vec3> newPath = Pathfinder::FindPath(
        Game::Instance().GetNavigationMesh(),
        GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetForward() * Game::Instance().GetNavigationMesh()->GetSpacing() * 2.f,
        GetTargetEntity()->transform.GetGlobalPosition());
    if (!newPath.empty() || FinishedPath()) {
        path = newPath;
    }
    marker->transform.SetPosition(path[0]);
}

void AiComponent::NextNodeInPath() {
    path.pop_front();
    if (!FinishedPath()) {
        marker->transform.SetPosition(path[0]);
    }
}

glm::vec3 AiComponent::NodeInPath() const {
    return path[0];
}

bool AiComponent::FinishedPath() const {
    return path.size() == 0;
}

size_t AiComponent::NextWaypoint(size_t waypointCount) {
    waypointIndex = (waypointIndex + 1) % waypointCount;
    return GetWaypoint();
}
