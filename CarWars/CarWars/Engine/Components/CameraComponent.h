#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <json/json.hpp>
#include "../Systems/Physics.h"

class Graphics;

class CameraComponent : public Component {
public:
	static const float NEAR_CLIPPING_PLANE;
	static const float FAR_CLIPPING_PLANE;
	static const float DEFAULT_FIELD_OF_VIEW;

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;

    ~CameraComponent() override;
	CameraComponent();
	CameraComponent(nlohmann::json data);
	CameraComponent(glm::vec3 _position, glm::vec3 _target, glm::vec3 _upVector);

	glm::vec3 GetPosition() const;
	glm::vec3 GetTarget() const;
	float GetFieldOfView() const;

	void SetPosition(glm::vec3 _position);
	void SetTarget(glm::vec3 _target);
	void SetFieldOfView(float _fieldOfView);
	void SetAspectRatio(float _aspectRatio);
	void SetUpVector(glm::vec3 _up);
	void SetTargetOffset(glm::vec3 offset);

	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix() const;

	float GetCameraHorizontalAngle();
	void SetCameraHorizontalAngle(float _cameraAngle);

	float GetCameraVerticalAngle();
	void SetCameraVerticalAngle(float _cameraLift);

	void UpdateCameraPosition(Entity* _vehicle, float _cameraHor, float _cameraVer);

	float GetCameraSpeed();

    void RenderDebugGui() override;

	glm::vec3 CastRay(float rayLength, PxQueryFilterData filterData);

	Entity* GetGuiRoot();

	//std::vector<Entity*>& GetGuiEntities();

private:
	//std::vector<Entity*> guiEntities;
	Entity *guiRoot;

    bool targetInLocalSpace;

	float fieldOfView;		// In degrees
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 targetOffset;
	glm::vec3 upVector;

	float aspectRatio;

	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
    void UpdatePositionFromAngles();

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	float cameraAngle = -3.14 / 2;
	float cameraLift = 3.14 / 4;
    float distanceFromCenter;
	float cameraSpeed = 5.f;
};
