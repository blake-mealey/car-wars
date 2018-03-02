#include "AiComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Systems/Pathfinder.h"
#include "../Systems/Game.h"
#include <iostream>
#include "../Systems/StateManager.h"

AiComponent::AiComponent(nlohmann::json data) : targetEntity(nullptr), waypointIndex(0), lastPathUpdate(0) {
    std::string modeName = ContentManager::GetFromJson<std::string>(data["Mode"], "Waypoints");
    if (modeName == "Waypoints") {
        mode = AiMode_Waypoints;
    } else if (modeName == "Chase") {
        mode = AiMode_Chase;
    }
    marker = ContentManager::LoadEntity("Marker.json");

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

size_t AiComponent::GetWaypoint() const {
    return waypointIndex;
}

void AiComponent::UpdatePath() {
    if (StateManager::gameTime - lastPathUpdate < 0.1f) return;
    lastPathUpdate = StateManager::gameTime;

    auto newPath = Pathfinder::FindPath(
        Game::Instance().GetNavigationMesh(),
        GetEntity()->transform.GetGlobalPosition() - GetEntity()->transform.GetForward() * Game::Instance().GetNavigationMesh()->GetSpacing() * 2.f,
        GetTargetEntity()->transform.GetGlobalPosition());
    if (!newPath.empty() || FinishedPath()) {
        path = newPath;
    }
    UpdateRenderBuffers();
    marker->transform.SetPosition(NodeInPath());
}

void AiComponent::NextNodeInPath() {
    if (FinishedPath()) return;
    path.pop_back();
    marker->transform.SetPosition(NodeInPath());
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

size_t AiComponent::NextWaypoint(size_t waypointCount) {
    waypointIndex = (waypointIndex + 1) % waypointCount;
    return GetWaypoint();
}
