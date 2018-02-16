#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/glm.hpp>
#include "../Systems/Content/PointLight.h"

class PointLightComponent : public Component {
public:
	PointLightComponent(nlohmann::json data);
	PointLightComponent(glm::vec3 _color, float _power);

	glm::vec3 GetColor() const;
	float GetPower() const;
	PointLight GetData() const;

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;
private:
	glm::vec3 color;
	float power;
};

