#include "CameraComponent.h"
#include "../Entities/EntityManager.h"
#include <glm/gtc/matrix_transform.inl>
#include "../Entities/Entity.h"
#include "../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

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
    if (!entity || !targetInLocalSpace) return target + targetOffset;
	return target + targetOffset + entity->transform.GetGlobalPosition();
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

void CameraComponent::SetTargetOffset(const glm::vec3 _target) {
	targetOffset = _target;
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

void CameraComponent::SetUpVector(glm::vec3 _up) {
	upVector = _up;
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
}

float CameraComponent::GetCameraVerticalAngle() {
	return cameraLift;
}

void CameraComponent::SetCameraVerticalAngle(float _cameraLift) {
	cameraLift = _cameraLift;
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

void CameraComponent::UpdateCameraPosition(Entity* _vehicle, float _cameraHor, float _cameraVer) {
	/*
	SetCameraHorizontalAngle(_cameraHor);
	SetCameraVerticalAngle(_cameraVer);

	glm::vec3 vehicleForward = _vehicle->transform.GetForward();
	glm::vec3 vehicleUp = _vehicle->transform.GetUp();
	glm::vec3 vehicleRight = _vehicle->transform.GetRight();

	float dotFR = glm::dot(vehicleForward, Transform::RIGHT);		//Detect Direction im Facing
	float dotFU = glm::dot(vehicleForward, Transform::UP);			//Tilted Up or Down
	float dotUR = glm::dot(vehicleRight, Transform::UP);			//Rolled Right or Left - Angle of Roll

	float dotUU = glm::dot(vehicleUp, Transform::UP);				//Angle of Tilt
	float dotFF = glm::dot(vehicleForward, Transform::FORWARD);		//Angle of Rotation

	bool correctForward = dotFR > 0;
	bool correctUp = dotFU < 0;
	bool correctRight = dotUR > 0; 
	*/

	/*
	x = r cos T sin O
	y = r sin T sin O
	z = r cos O
	*/

	cameraAngle = _cameraHor; 
	cameraLift = _cameraVer;
	float xPos = distanceFromCenter * cos(cameraAngle) * sin(-cameraLift);
	float zPos = distanceFromCenter * sin(cameraAngle) * sin(-cameraLift);
	float yPos = distanceFromCenter * cos(-cameraLift);

	glm::vec3 position = glm::vec3(xPos, yPos, zPos);

	SetPosition(position);


	//TODO Figure out Vehicle Roll
	/*SetPosition(distanceFromCenter * (
		(-_vehicle->transform.GetForward() * cos(GetCameraHorizontalAngle() + (correctForward ? -acos(dotFF) : acos(dotFF))) * sin(GetCameraVerticalAngle() + (correctUp ? -acos(dotUU) : acos(dotUU)))) +
		(_vehicle->transform.GetUp() * cos(GetCameraVerticalAngle() + (correctUp ? -acos(dotUU) : acos(dotUU)))) +
		(-_vehicle->transform.GetRight() * sin(GetCameraHorizontalAngle() + (correctForward ? -acos(dotFF) : acos(dotFF))) * sin(GetCameraVerticalAngle() + (correctUp ? -acos(dotUU) : acos(dotUU)))))
	);*/
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

glm::vec3 CameraComponent::CastRay(float rayLength, PxQueryFilterData filterData) {
	PxScene* scene = &Physics::Instance().GetScene();
	glm::vec3 cameraDirection = glm::normalize(GetTarget() - GetPosition());
	//Cast Camera Ray
	PxRaycastBuffer cameraHit;
	glm::vec3 cameraHitPosition;
	if (scene->raycast(Transform::ToPx(GetTarget()), Transform::ToPx(cameraDirection), rayLength, cameraHit, PxHitFlag::eDEFAULT, filterData)) {
		//cameraHit has hit something
		cameraHitPosition = Transform::FromPx(cameraHit.block.position);
		EntityManager::FindEntity(cameraHit.block.actor);
	} else {
		//cameraHit has not hit anything
		cameraHitPosition = GetPosition() + (cameraDirection * rayLength);
	}
	return cameraHitPosition;
}