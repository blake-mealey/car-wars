#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/glm.hpp>
#include "../Systems/Content/SpotLight.h"

class SpotLightComponent : public Component {
public:
	SpotLightComponent(nlohmann::json data);
	SpotLightComponent(glm::vec3 _color, float _power, float _angle, glm::vec3 _direction);

	glm::vec3 GetColor() const;
	float GetPower() const;
	float GetAngle() const;
	glm::vec3 GetDirection() const;
	SpotLight GetData() const;

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;
private:
	glm::vec3 color;
	float power;
	float angle;
	glm::vec3 direction;
};