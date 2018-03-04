#include "CameraComponent.h"
#include "../Entities/EntityManager.h"
#include <glm/gtc/matrix_transform.inl>
#include "../Entities/Entity.h"
#include "../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

const float CameraComponent::NEAR_CLIPPING_PLANE = 0.1f;
const float CameraComponent::FAR_CLIPPING_PLANE = 1000.f;
const float CameraComponent::DEFAULT_FIELD_OF_VIEW = 60.f;		// In degrees

ComponentType CameraComponent::GetType() {
	return ComponentType_Camera;
}

void CameraComponent::HandleEvent(Event* event) {}

CameraComponent::~CameraComponent() {
    EntityManager::DestroyStaticEntity(guiRoot);
}

CameraComponent::CameraComponent() : CameraComponent(glm::vec3(0, 0, -5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)) {}

CameraComponent::CameraComponent(nlohmann::json data) : guiRoot(nullptr) {
	fieldOfView = ContentManager::GetFromJson(data["FOV"], DEFAULT_FIELD_OF_VIEW);
	position = ContentManager::JsonToVec3(data["Position"], glm::vec3(0.f, 0.f, 1.f));
	target = ContentManager::JsonToVec3(data["Target"], glm::vec3(0.f, 0.f, 0.f));
	upVector = ContentManager::JsonToVec3(data["UpVector"], glm::vec3(0.f, 1.f, 0.f));
    distanceFromCenter = ContentManager::GetFromJson<float>(data["CenterDistance"], 20.f);
    targetInLocalSpace = ContentManager::GetFromJson<bool>(data["TargetInLocalSpace"], false);

	UpdateViewMatrix();
}

CameraComponent::CameraComponent(glm::vec3 _position, glm::vec3 _target, glm::vec3 _upVector) : targetInLocalSpace(false),
	position(_position), target(_target), upVector(_upVector), fieldOfView(DEFAULT_FIELD_OF_VIEW), distanceFromCenter(15.f), guiRoot(nullptr) {
	
	UpdateViewMatrix();
}

glm::vec3 CameraComponent::GetPosition() const {
	if (!entity) return position;
	return position + entity->transform.GetGlobalPosition();
}

glm::vec3 CameraComponent::GetTarget() const {
    if (!entity || !targetInLocalSpace) return target;
	return target + entity->transform.GetGlobalPosition();
}

float CameraComponent::GetFieldOfView() const {
	return fieldOfView;
}

void CameraComponent::SetPosition(const glm::vec3 _position) {
	position = _position;
	UpdateViewMatrix();
}

void CameraComponent::SetTarget(const glm::vec3 _target) {
	target = _target;
	UpdateViewMatrix();
}

void CameraComponent::SetFieldOfView(const float _fieldOfView) {
	fieldOfView = _fieldOfView;
	UpdateProjectionMatrix();
}

void CameraComponent::SetAspectRatio(const float _aspectRatio) {
	aspectRatio = _aspectRatio;
	UpdateProjectionMatrix();
}

glm::mat4 CameraComponent::GetViewMatrix() {
	UpdateViewMatrix();
	return viewMatrix;
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
	return projectionMatrix;
}

float CameraComponent::GetCameraHorizontalAngle() {
	return cameraAngle;
}

void CameraComponent::SetCameraHorizontalAngle(float _cameraAngle) {
	cameraAngle = _cameraAngle;
    UpdatePositionFromAngles();
}

float CameraComponent::GetCameraVerticalAngle() {
	return cameraLift;
}

void CameraComponent::SetCameraVerticalAngle(float _cameraLift) {
	cameraLift = _cameraLift;
    UpdatePositionFromAngles();
}

float CameraComponent::GetCameraSpeed() {
	return cameraSpeed;
}

void CameraComponent::RenderDebugGui() {
    Component::RenderDebugGui();
    if (ImGui::DragFloat("FOV", &fieldOfView, 0, 180)) UpdateProjectionMatrix();
    if (ImGui::SliderAngle("Horizontal Angle", &cameraAngle)) UpdatePositionFromAngles();
    if (ImGui::SliderAngle("Vertical Angle", &cameraLift, 5, 175)) UpdatePositionFromAngles();
    if (ImGui::DragFloat("Distance", &distanceFromCenter, 1, 0, FAR_CLIPPING_PLANE)) UpdatePositionFromAngles();
    if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) UpdateViewMatrix();
    if (ImGui::DragFloat3("Target", glm::value_ptr(target), 0.1f)) UpdateViewMatrix();
}

void CameraComponent::UpdatePositionFromAngles() {
    SetPosition(distanceFromCenter * glm::vec3(
        cos(GetCameraHorizontalAngle()) * sin(GetCameraVerticalAngle()),
        cos(GetCameraVerticalAngle()),
        sin(GetCameraHorizontalAngle()) * sin(GetCameraVerticalAngle())));
}

void CameraComponent::UpdateViewMatrix() {
	viewMatrix = glm::lookAt(GetPosition(), GetTarget(), upVector);
}

void CameraComponent::UpdateProjectionMatrix() {
	projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspectRatio, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
}

Entity* CameraComponent::GetGuiRoot() {
	if (guiRoot == nullptr) {
		guiRoot = EntityManager::CreateStaticEntity();
		EntityManager::SetTag(guiRoot, "GuiRoot");
	}
	return guiRoot;
}

/*std::vector<Entity*>& CameraComponent::GetGuiEntities() {
	return guiEntities;
}*/